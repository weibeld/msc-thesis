/*
 * efa.c: extended finite automata library
 *
 * Copyright (C) 2013 Christian Göttel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 *
 * Author: Christian Göttel <christian.goettel@unifr.ch>
 */

#include "libefa/efa.h"
#include "libefa/errno.h"
#include "libefa/error.h"
#include "libefa/memory.h"

#if HAVE_MODULE_BUECHI
# include "libefa/buechi.h"
#endif

#if HAVE_MODULE_FADOT
# include "libefa/fadot.h"
#endif

#if HAVE_MODULE_FAMOD
# include "libefa/famod.h"
#endif

#if HAVE_MODULE_FAXML
# include "libefa/faxml.h"
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef _GNU_SOURCE
# include <getopt.h>
# include <unistd.h>
# include <sys/types.h>
#elif _WIN32 || _WIN64
# include <io.h>
#endif

/*
 * algorithm_comp_names:
 * Array with complementation algorithm names.
 */
const char *const algorithm_comp_names[] = {
#if HAVE_MODULE_FAMOD
  "ordinary",
#endif
#if HAVE_MODULE_BUECHI && HAVE_MODULE_FAMOD
  "unifr",
  "unifr2",
  "unifr3",
#endif
  NULL
};

/*
 * algorithm_equiv_names:
 * Array with equivalence algorithm names.
 */
const char *const algorithm_equiv_names[] = {
#if HAVE_MODULE_FAMOD
  "table-filling",
#endif
  NULL
};

/*
 * algorithm_gen_names:
 * Array with generator algorithm names.
 */
const char *const algorithm_gen_names[] = {
#if HAVE_MODULE_FAMOD
  "boltzmann_samplers",
  "recursive_method",
#endif
  NULL
};

/*
 * algorithm_min_names:
 * Array with minimization algorithm names.
 */
const char *const algorithm_min_names[] = {
#if HAVE_MODULE_FAMOD
  "brzozowski",
  "hopcroft",
#endif
  NULL
};

/*
 * algorithm_run_names:
 * Array with run algorithm names.
 */
const char *const algorithm_run_names[] = {
  "ordinary",
  NULL
};

/*
 * algorithm_trans_names:
 * Array with transformation algorithm names.
 */
const char *const algorithm_trans_names[] = {
#if HAVE_MODULE_FAMOD
  "collect",
  "complete",
  "subset_construction",
#endif
  NULL
};

const char *const algorithm_type_names[] = {
  "complementation",
  "equivalence",
  "generator",
  "minimization",
  "run",
  "transformation",
  NULL
};

const char *const data_type_names[] = {
#if HAVE_MODULE_HASH
  "hashtable",
#endif
#if HAVE_MODULE_RBTREE
  "red-black-tree",
#endif
  NULL,
  "none"
};

const char *const efa_fa_class_names[] = {
  "deterministic",
  "non-deterministic",
  "epsilon-non-deterministic",
  NULL
};

const char *const efa_fa_type_names[] = {
  "ordinary",
#if HAVE_MODULE_BUECHI
  "buechi",
#endif
  NULL
};

#if HAVE_MODULE_HASH
efa_ioh_t hash_io = {
  {
#if HAVE_MODULE_FADOT
    NULL,
#endif
    &regexp_hash_read,
#if HAVE_MODULE_FAXML
    &faxml_validate_parse
#endif
  },
  {
#if HAVE_MODULE_FADOT
    &fadot_hash_write,
#endif
    &regexp_hash_write,
#if HAVE_MODULE_FAXML
    &faxml_hash_write
#endif
  }
};
#endif

#if HAVE_MODULE_RBTREE
efa_ioh_t rbtree_io = {
  {
#if HAVE_MODULE_FADOT
    NULL,
#endif
    &regexp_rbtree_read,
#if HAVE_MODULE_FAXML
    NULL
#endif
  },
  {
#if HAVE_MODULE_FADOT
    &fadot_rbtree_write,
#endif
    &regexp_rbtree_write,
#if HAVE_MODULE_FAXML
    &faxml_rbtree_write
#endif
  }
};
#endif

const efa_ioh_t *const efa_dt_io[] = {
#if HAVE_MODULE_HASH
  &hash_io,
#endif
#if HAVE_MODULE_RBTREE
  &rbtree_io,
#endif
  NULL
};

const char *const file_extension_names[] = {
#if HAVE_MODULE_FADOT
  ".dot",
#endif
  ".txt",
#if HAVE_MODULE_FAXML
  ".xml",
#endif
  NULL
};

const char *const efa_module_names[] = {
#if HAVE_MODULE_BUECHI
  "BUECHI",
#endif
  "EFA",
#if HAVE_MODULE_FADOT
  "FADOT",
#endif
#if HAVE_MODULE_FAMOD
  "FAMOD",
#endif
#if HAVE_MODULE_FAXML
  "FAXML",
#endif
#if HAVE_MODULE_HASH
  "HASH",
#endif
#if HAVE_MODULE_RBTREE
  "RBTREE",
#endif
  "EFATOOL",
  NULL
};

const char *const verbosity_level_names[] = {
  "memory",
  "time",
  NULL
};

#if HAVE_MODULE_HASH
algo_handler_t hash_algorithms = {
  {
#if HAVE_MODULE_FAMOD
    fa_hash_complement,
#endif
#if HAVE_MODULE_BUECHI && HAVE_MODULE_FAMOD
    buechi_hash_unifr,
    buechi_hash_unifr2,
    buechi_hash_unifr3,
#endif
  },
  {
#if HAVE_MODULE_FAMOD
    fa_hash_equivalence,
#endif
  },
  {
#if HAVE_MODULE_FAMOD
    fa_hash_boltzmann_samplers,
    fa_hash_recursive_method,
#endif
  },
  {
#if HAVE_MODULE_FAMOD
    fa_hash_brzozowski,
    fa_hash_hopcroft,
#endif
  },
  {
#if HAVE_MODULE_FAMOD
    fa_hash_run,
#endif
  },
  {
#if HAVE_MODULE_FAMOD
    fa_hash_collect,
    fa_hash_complete,
    fa_hash_subset_construction,
#endif
  },
};
#endif /* HAVE_MODULE_HASH */

#if HAVE_MODULE_RBTREE
algo_handler_t rbtree_algorithms = {
  {
#if HAVE_MODULE_FAMOD
    fa_rbtree_complement,
#endif
#if HAVE_MODULE_BUECHI && HAVE_MODULE_FAMOD
    buechi_rbtree_unifr,
    buechi_rbtree_unifr2,
    buechi_rbtree_unifr3,
#endif
  },
  {
#if HAVE_MODULE_FAMOD
    fa_rbtree_equivalence,
#endif
  },
  {
#if HAVE_MODULE_FAMOD
    fa_rbtree_boltzmann_samplers,
    fa_rbtree_recursive_method,
#endif
  },
  {
#if HAVE_MODULE_FAMOD
    fa_rbtree_brzozowski,
    fa_rbtree_hopcroft,
#endif
  },
  {
#if HAVE_MODULE_FAMOD
    fa_rbtree_run,
#endif
  },
  {
#if HAVE_MODULE_FAMOD
    fa_rbtree_collect,
    fa_rbtree_complete,
    fa_rbtree_subset_construction,
#endif
  },
};
#endif /* HAVE_MODULE_RBTREE */

/* Private global variables */
static struct timespec *ts_start = NULL, *ts_end = NULL;

/* Private functions */
__attribute__((noreturn)) static void efa_display_algorithm(void);
__attribute__((noreturn)) static void efa_display_class(void);
__attribute__((noreturn)) static void efa_display_file(void);
__attribute__((noreturn)) static void efa_display_level(void);
static void efa_display_preamble(void);
__attribute__((noreturn)) static void efa_display_structure(void);
__attribute__((noreturn)) static void efa_display_type(void);
__attribute__((noreturn)) static void efa_display_usage(const char *);
__attribute__((noreturn)) static void efa_display_version(void);
static int efa_parse_opt_algorithm(anode_t **, fa_t *, const char *);
static int efa_parse_opt_at(fa_t *, const char *);
static int efa_parse_opt_class(fa_t *, const char *);
static int efa_parse_opt_ds(fa_t *, const char *);
static int efa_parse_opt_usage(const char *, const char *);
static int efa_parse_opt_verbose(const char *, unsigned char *);

#if HAVE_MODULE_HASH
static void efa_display_hash(void);
#endif

/*
 * efa_display_algorithm:
 * Display information on algorithm options.
 */
static void efa_display_algorithm(void) {
  int ret;
  
  efa_display_preamble();
  
  ret = puts("  ALGORITHM:\n \
  An algorithm is defined by a CATEGORY.KEY=VALUE tuple. The CATEGORY\n \
  describes the type of algorithm, the KEY stands for the algorithm itself,\n \
  and the VALUE are colon separated additional options for the actual\n \
  algorithm. The following CATEGORY.KEY=VALUE tuples are available:\n \
    CATEGORY           KEY                                       VALUE\n \
   complement   ordinary, unifr\n \
   equivalence  table-filling                                     FILE\n \
   generate     boltzmann_samplers, recursive_method\n \
   minimize     brzozowski, hopcroft\n \
   run          ordinary\n \
   transform    collect, complete, subset_construction\n\n \
   - complement   complements the automaton\n \
   - equivalence  test two automata on their equivalence\n \
   - generate     generates a random automaton. Should be used with the\n \
                options '-n' and '-o' to work properly\n \
   - minimize     minimize the automaton\n \
   - transform    transforms a CLASS of automaton into another CLASS.");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }
  
  exit(EXIT_SUCCESS);
}

/*
 * efa_display_class:
 * Display information on the class option.
 */
static void efa_display_class(void) {
  int ret;
  
  efa_display_preamble();
  
  ret = puts("  CLASS:\n \
  Three classes of automata are defined:\n \
   - deterministic\n \
   - non-deterministic (default)\n \
   - epsilon-non-deterministic");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }
  
  exit(EXIT_SUCCESS);
}

/*
 * efa_display_file:
 * Display information on the file option.
 */
static void efa_display_file(void) {
  int ret;
  
  efa_display_preamble();
  
  ret = puts("  FILE:\n \
  FILE must be a valid path and the name of an existing non-empty file on\n \
  the local file system or network. The automaton can also be passed by a\n \
  pipe by writing a hyphen '-'. The following file types are supported:");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
  
#if HAVE_MODULE_FADOT
  ret = puts("   - DOT files (can only be written to not read from)");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
#endif
#if HAVE_MODULE_FAXML
  ret = puts("   - XML files in FAXML format (can be compressed, are \
  validated when read)\n \
   - XML Schemas describing the FAXML file format");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
#endif

  exit(EXIT_SUCCESS);
  
error:
  exit(EXIT_FAILURE);
}

#if HAVE_MODULE_HASH
/*
 * efa_display_hash:
 * Display information on the hash option.
 */
static void efa_display_hash(void) {
  int ret;
  
  efa_display_preamble();
  
#if __WORDSIZE == 32
  ret = puts("  HASH:\n \
  The following hash function can be used for the hash data type:\n \
   - kazlib          Default hash function of the 'kazlib' (default)\n \
   - additive        Adds up character values. WARNING: high collision\n \
                     rate for big tables and table size must be prime!\n \
   - bernstein       Produces hashes with fewer collisions! WARNING:\n \
                     using it for automaton state labels produces easily\n \
                     detectable flaws!");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    return;
  }
#if HAVE_XMMINTRIN_H
  ret = puts("   - cessu           WARNING: needs SSE2 instruction set!");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    return;
  }
#endif /* HAVE_XMMINTRIN_H */
  ret = puts("   - crc             Shifts out lower order bits and xors with\n \
                    a polynomial.\n \
   - general-crc     Similar to 'crc'.\n \
   - murmur1\n \
   - murmur1a\n \
   - murmur2\n \
   - murmur2a\n \
   - murmurn2\n \
   - murmura2\n \
   - murmur3\n \
   - one-at-a-time   Similar to 'rotating' but mixes internal state.\n \
   - paul-hsieh\n \
   - rotatingv1      Does a circular shift of the hash by 4. WARNING:\n \
                     table size must be prime!\n \
   - ratatingv2      Does a circular shift of the hash by 4.\n \
  If not otherwise indicated, all hashes have a table size of power of 2.");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    return;
  }
#elif __WORDSIZE == 64
  ret = puts("  HASH:\n \
  The following hash function can be used for the hash data type:\n \
   - kazlib          Default hash function of the 'kazlib' (default)\n \
   - additive        Adds up character values. WARNING: high collision\n \
                     rate for big tables and table size must be prime!\n \
   - bernstein       Produces hashes with fewer collisions! WARNING:\n \
                     using it for automaton state labels produces easily\n \
                     detectable flaws!\n \
   - crc             Shifts out lower order bits and xors with a\n \
                     polynomial.\n \
   - general-crc     Similar to 'crc'.\n \
   - murmur1\n \
   - murmur1a\n \
   - murmur2\n \
   - rotatingv1      Does a circular shift of the hash by 4. WARNING:\n \
                     table size must be prime!\n \
   - ratatingv2      Does a circular shift of the hash by 4.\n \
  If not otherwise indicated, all hashes have a table size of power of 2.");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    return;
  }
#endif /* __WORDSIZE = XX */

  exit(EXIT_SUCCESS);
}
#endif /* HAVE_MODULE_HASH */

/*
 * efa_display_level:
 * Display information on the verbosity level option.
 */
static void efa_display_level(void) {
  int ret;
  
  efa_display_preamble();
  
  ret = puts("  LEVEL:\n \
  LEVEL is a comma separated list of the following verbosity levels:\n \
   - memory  measures the amount of memory the automaton needs\n \
   - time    measures the time an algorithm needs to process the automaton");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }
  
  exit(EXIT_SUCCESS);
}

/*
 * efa_display_preamble:
 * Display the preamble with basic information on libefa.
 */
static void efa_display_preamble(void) {
  int ret;
  
  ret = printf("Standard Extended Finite Automata Library Tool\n\
Version %s compiled on %s at %s\n\n", VERSION, __DATE__, __TIME__);
 if (ret < 0) {
   efa_errno = EFA_EPRINTF;
   efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
   return;
 }
}

/*
 * efa_display_regexp:
 * Display information on the regexp option.
 */
static void efa_display_regexp(void) {
  int ret;
  
  efa_display_preamble();
  
  ret = puts("  REGEXP:\n \
  Any symbol can be used to formulate a regular expression except '*' and\n \
  '°' which are used as closure characters for regular respectively\n \
  omega-languages.");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    return;
  }
  
  exit(EXIT_SUCCESS);
}

/*
 * efa_display_structure:
 * Display information on the data structure option.
 */
static void efa_display_structure(void) {
  int ret;
  
  efa_display_preamble();
  
  ret = puts("  STRUCTURE:\n \
  The following types of data structure can be used for an automaton:");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
#if HAVE_MODULE_HASH
  ret = puts("   - hashtable");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
#endif
#if HAVE_MODULE_RBTREE
  ret = puts("   - red-black-tree");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
#endif

  exit(EXIT_SUCCESS);
  
error:
  exit(EXIT_FAILURE);
}

/*
 * efa_display_type:
 * Display information on the automaton type option.
 */
static void efa_display_type(void) {
  int ret;
  
  efa_display_preamble();
  
  ret = puts("  TYPE:");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
#if HAVE_MODULE_FAMOD
  ret = puts("   - normal   Normal finite automaton / state machine. This is \
the default.");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
#endif
#if HAVE_MODULE_BUECHI
  ret = puts("   - buechi   Büchi type automaton");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
#endif

  exit(EXIT_SUCCESS);
  
error:
  exit(EXIT_FAILURE);
}

/*
 * efa_display_usage:
 * Display some helpful text about the program and exit.
 */
static void efa_display_usage(const char *pn) {
  int ret;

  efa_display_preamble();
  
  ret = printf("Usage: %s [OPTIONS]\n \
 Load automata from a file or a regular expression and transform them\n \
 accordingly.\n\n \
 Options:\n\n \
  -a, --algorithm ALGORITHM a comma separated ordered KEY=VALUE list of\n \
                            algorithms to apply on the automaton.\n \
  -c, --class CLASS defines the class of the automaton. The value of this\n \
                    option has to match any class defined in a file used as\n \
                    input, otherwise the program exits with an error.\n \
  -d, --data-structure STRUCTURE the way the states of the automaton are\n \
                                 stored.\n \
   -h, --help print this text.\n", pn);
  if (ret < 0) {
    efa_errno = EFA_EPRINTF;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
#if HAVE_MODULE_HASH
  ret = puts("   -H, --hash HASH uses the defined hash function for the hash \
data type.");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
#endif
  ret = puts("   -i, --input FILE read automaton from FILE. FILE can be a \
file stored in\n \
                                  file system, on network or passed via a\n \
                                  pipe '-'. See '-h file' for a full list of\n \
                                  supported file types.\n \
  -n, --number NUMBER number of automata to generate. If no option is given,\n \
                      only one automaton will be generated.\n \
  -o, --output PATH path to directory where the generated automata will be\n \
                    stored. If no option is given, the automata are stored\n \
                    in the working directory with the default file name\n \
                    'out.x'.\n \
  -s, --schema FILE read XML Schema from FILE. FILE can be a file stored in\n \
                    file system, on network or passed via a pipe '-'.See\n \
                    '-h file' for a full list of supported file types.\n \
  -r, --regexp REGEXP generates an automaton from the given regular\n \
                      expression.\n \
  -t, --type TYPE defines the type of the automaton that is loaded from a\n \
                  file or a regular expression into the program.\n \
  -v, --verbose LEVEL advises the program to trigger different verbosity\n \
                      levels.\n \
  --version print version information and exit.");
  if (ret == EOF) {
    efa_errno = EFA_EPUTS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    goto error;
  }
  
  exit(EXIT_SUCCESS);
  
error:
  exit(EXIT_FAILURE);
}

/*
 * efa_display_version:
 * Display the version information of the EFA library and exit.
 */
static void efa_display_version(void) {
  printf("efa %s \n \
          Copyright (C) 2013 Christian Göttel\n	     \
          License LGPLv2+: GNU LGPL version 2.1 or later\n	\
          <http://www.gnu.org/licenses/lgpl-2.1.html>\n \
          This is free software: you are free to change and redistribute it.\n \
          There is NO WARRANTY, to the extent permitted by law.\n\n \
          Written by Christian Göttel\n", VERSION);
  
  exit(EXIT_SUCCESS);
}

/*
 * efa_parse_opt_algorithm:
 * Parse the parameters given to the algorithm option.
 */
static int efa_parse_opt_algorithm(anode_t **algos, fa_t *fa, const char *opt) 
{
  algo_handler_t *algorithm_functions;
  char *tok, *stok, *sstok, *ssstok, *str1, *str2, *str3, *str4, *saveptr1;
  char *saveptr2, *saveptr3, *saveptr4;
  const char *delim1 = ",";
  const char *delim2 = ".";
  const char *delim3 = "=";
  const char *delim4 = ":";
  int i, j, k, category = 0, nalgorithms = 0, ret;
  unsigned int category_cont, key_cont;
  size_t l;

  switch(fa->data_type) {
#if HAVE_MODULE_HASH
    case DT_HASH_TABLE:
      algorithm_functions = &hash_algorithms;
      break;
#endif
#if HAVE_MODULE_RBTREE
    case DT_RB_TREE:
      algorithm_functions = &rbtree_algorithms;
      break;
#endif
    case DT_NO:
    default:
      errno = EINVAL;
      efa_errno = EFA_EPARSE_OPT_ALGORITHM;
      efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
      exit(EXIT_FAILURE);
      break;
  }
  
  /* Nasty casting of character pointer to character array. This is needed as
     strtok_r() at line 727 would raise a segmentation fault (SIGSEGV) if a
     character pointer was used.                                              */
  l = strlen(opt);
  char str[++l];
  memcpy((void *)str, (const void *)opt, l);
  str[l] = '\0';

  // count the number of algorithms
  str1 = (char *)opt;
  do {
    nalgorithms++;
    str1++;
    str1 = strchr((const char *)str1, (int) *delim1);
  } while(str1);

  /* allocate an array of function pointers that is terminated by a zero entry
     Note: algos is the address of the options->algorithms field!             */
  ret = efa_alloc(algos, sizeof(anode_t), ++nalgorithms);
  if (ret == -1) {
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }

  /* Inception at its best:
   * tok = token
   * stok = subtoken
   * sstok = subsubtoken
   * ...
   * lets hope we don't get stuck in limbo!
   */
  for (j = 0, str1 = str; ; j++, str1 = NULL) {
    tok = strtok_r(str1, delim1, &saveptr1);
    if (tok == NULL) // string match
      break;
#if DEBUG
    ret = printf("%d: %s\n", j, tok);
    if (ret < 0) {
      efa_errno = EFA_EPRINTF;
      efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    }
#endif
  
    for (str2 = tok; ; str2 = NULL) {
      stok = strtok_r(str2, delim2, &saveptr2);
      if (stok == NULL) // string match
        break;
#if DEBUG
      ret = printf(" -> %s\n", stok);
      if (ret < 0) {
	efa_errno = EFA_EPRINTF;
	efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
      }
#endif

      for (i = 0; i < ALGO_T_NB; i++) {
	ret = strncmp(stok, algorithm_type_names[i], ALGO_TYPE_LEN);
	if (ret == 0) { // string match
	  category = i;
	  category_cont = 1;
	  break;
	}
      }
      
      switch(i) {
	  case ALGO_T_COMPLEMENTATION:
	    (*algos)[j].type = ALGO_T_COMPLEMENTATION;
	    break;
	  case ALGO_T_EQUIVALENCE:
	    (*algos)[j].type = ALGO_T_EQUIVALENCE;
	    break;
	  case ALGO_T_GENERATOR:
	    (*algos)[j].type = ALGO_T_GENERATOR;
	    break;
	  case ALGO_T_MINIMIZATION:
	    (*algos)[j].type = ALGO_T_MINIMIZATION;
	    break;
	  case ALGO_T_RUN:
	    (*algos)[j].type = ALGO_T_RUN;
	    break;
	  case ALGO_T_TRANSFORMATION:
	    (*algos)[j].type = ALGO_T_TRANSFORMATION;
	    break;
	  default:
	    break;
      }
      
      if (category_cont) {
	category_cont = 0;
	continue;
      }

      for (str3 = stok; ; str3 = NULL) {
	sstok = strtok_r(str3, delim3, &saveptr3);
	if (sstok == NULL) // string match
	  break;
#if DEBUG
	ret = printf(" --> %s\n", sstok);
	if (ret < 0) {
	  efa_errno = EFA_EPRINTF;
	  efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
	}
#endif
	
	switch(category) {
	  case ALGO_T_COMPLEMENTATION:
	    for (k = 0; k < ALGO_COMP_NB; k++) {
	      ret = strncmp(sstok, algorithm_comp_names[k], ALGO_COMP_LEN);
	      if (ret == 0) // key found
		(*algos)[j].function = (void 
*)algorithm_functions->algo_comp_fun[k];
	    }
	    key_cont = 1;
	    break;
	  case ALGO_T_EQUIVALENCE:
	    for (k = 0; k < ALGO_EQUIV_NB; k++) {
	      ret = strncmp(sstok, algorithm_equiv_names[k], ALGO_EQUIV_LEN);
	      if (ret == 0) // key found
		(*algos)[j].function = (void 
*)algorithm_functions->algo_equiv_fun[k];
	    }
	    key_cont = 1;
	    break;
	  case ALGO_T_GENERATOR:
	    for (k = 0; k < ALGO_GEN_NB; k++) {
	      ret = strncmp(sstok, algorithm_gen_names[k], ALGO_GEN_LEN);
	      if (ret == 0) // key found
		(*algos)[j].function = (void 
*)algorithm_functions->algo_gen_fun[k];
	    }
	    key_cont = 1;
	    break;
	  case ALGO_T_MINIMIZATION:
	    for (k = 0; k < ALGO_MIN_NB; k++) {
	      ret = strncmp(sstok, algorithm_min_names[k], ALGO_MIN_LEN);
	      if (ret == 0) // key found
		(*algos)[j].function = (void 
*)algorithm_functions->algo_min_fun[k];
	    }
	    key_cont = 1;
	    break;
	  case ALGO_T_RUN:
	    for (k = 0; k < ALGO_RUN_NB; k++) {
	      ret = strncmp(sstok, algorithm_run_names[k], ALGO_MIN_LEN);
	      if (ret == 0) // key found
		(*algos)[j].function = (void 
*)algorithm_functions->algo_run_fun[k];
	    }
	    key_cont = 1;
	    break;
	  case ALGO_T_TRANSFORMATION:
	    for (k = 0; k < ALGO_TRANS_NB; k++) {
	      ret = strncmp(sstok, algorithm_trans_names[k], ALGO_TRANS_LEN);
	      if (ret == 0) // key found
		(*algos)[j].function = (void 
*)algorithm_functions->algo_trans_fun[k];
	    }
	    key_cont = 1;
	    break;
	  default:
	    break;
	}
	
	if (key_cont) {
	  key_cont = 0;
	  continue;
	}
	
	for (str4 = sstok; ; str4 = NULL) {
	  ssstok = strtok_r(str4, delim4, &saveptr4);
	  if (ssstok == NULL) // value found
	    break;
#if DEBUG
	  ret = printf(" ---> %s\n", ssstok);
	  if (ret < 0) {
	    efa_errno = EFA_EPRINTF;
	    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
	  }
#endif
	}
      }
    }
  }
  
  return 0;
}

/*
 * efa_parse_opt_at:
 * Parse the parameter given to the automaton type option.
 */
static int efa_parse_opt_at(fa_t *automaton, const char *opt) {
  int i, ret;

  for (i = 0; i < FA_TYPE_NB; i++) {
    ret = strncmp(opt, efa_fa_type_names[i], FA_TYPE_LEN);
    if (ret == 0) // automaton type names match
      break;
  }

  if (i == FA_TYPE_NB) {
    errno = EINVAL;
    efa_errno = EFA_EPARSE_OPT_AT;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }

  if (automaton->type) {
    if (automaton->type != i) {
      errno = EDOM;
      efa_errno = EFA_EPARSE_OPT_AT;
      efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
      exit(EXIT_FAILURE);
    }
  } else
    automaton->type = i;

  return 0;
}

/*
 * efa_parse_opt_class:
 * Parse the parameter given to the class option.
 */
static int efa_parse_opt_class(fa_t *automaton, const char *opt) {
  int i, ret;
  
  for (i = 0; i < FA_CLASS_NB; i++) {
    ret = strncmp(opt, efa_fa_class_names[i], FA_CLASS_LEN);
    if (ret == 0) // class names match
      break;
  }
  if (i == FA_CLASS_NB) {
    errno = EINVAL;
    efa_errno = EFA_EPARSE_OPT_CLASS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }

  if (automaton->class) {
    if (automaton->class != i) {
      errno = EDOM;
      efa_errno = EFA_EPARSE_OPT_CLASS;
      efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
      exit(EXIT_FAILURE);
    }
  } else
    automaton->class = i;

  return 0;
}

/*
 * efa_parse_opt_ds:
 * Parse the parameter given to the data structure option.
 */
static int efa_parse_opt_ds(fa_t *automaton, const char *opt) {
  int i, ret;

  for (i = 0; i < DT_NB; i++) {
    ret = strncmp(opt, data_type_names[i], DT_LEN);
    if (ret == 0) { // data type names match
      break;
    }
  }

  switch (i) {
#if HAVE_MODULE_HASH
  case DT_HASH_TABLE:
    automaton->data_type = DT_HASH_TABLE;

    ret = efa_alloc(&automaton->data_structure, sizeof(hash_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    break;
#endif

#if HAVE_MODULE_RBTREE
  case DT_RB_TREE:
    automaton->data_type = DT_RB_TREE;

    ret = efa_alloc(&automaton->data_structure, sizeof(rbtree_t), 1);
    if (ret == -1)
      goto alloc_error;

    break;
#endif

  default:
    errno = EINVAL;
    efa_errno = EFA_EPARSE_OPT_DS;
    efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }

  return 0;
  
alloc_error:
  efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}

/*
 * efa_parse_opt_usage:
 * Parse the parameter given to the usage option.
 */
static int efa_parse_opt_usage(const char *opt, const char *pn) {
  int i, ret;
  
  static const char *const help_names[] = {
    "algorithm",
    "class",
    "file",
#if HAVE_MODULE_HASH
    "hash",
#endif
    "level",
    "regexp",
    "structure",
    "type",
    NULL
  };
  
  enum help_name {
    HELP_ALGORITHM,
    HELP_CLASS,
    HELP_FILE,
#if HAVE_MODULE_HASH
    HELP_HASH,
#endif
    HELP_LEVEL,
    HELP_REGEXP,
    HELP_STRUCTURE,
    HELP_TYPE,
    HELP_NB
  };
  
  if (opt) {
    for (i = 0; i < HELP_NB; i++) {
      ret = strncmp(opt, help_names[i], 9);
      if (ret == 0) // help options match
	break;
    }
  
    switch (i) {
      case HELP_ALGORITHM:
	efa_display_algorithm();
	break;
      case HELP_CLASS:
	efa_display_class();
	break;
      case HELP_FILE:
	efa_display_file();
	break;
#if HAVE_MODULE_HASH
      case HELP_HASH:
	efa_display_hash();
	break;
#endif
      case HELP_LEVEL:
	efa_display_level();
	break;
      case HELP_REGEXP:
	efa_display_regexp();
	break;
      case HELP_STRUCTURE:
	efa_display_structure();
	break;
      case HELP_TYPE:
	efa_display_type();
	break;
      default:
	efa_display_usage(pn);
	break;
    }
  } else
    efa_display_usage(pn);
 
  return 0;
}

/*
 * efa_parse_opt_verbose:
 * Parse the parameter given to the verbose option.
 */
static int efa_parse_opt_verbose(const char *opt, unsigned char *verbose) {
  char *token, *str1, *saveptr;
  const char *delim = ",";
  int i, j, l = 0, ret;

  /* Nasty casting of character pointer to character array. This is needed as
     strtok_r() at line 115 would raise a segmentation fault (SIGSEGV) if a
     character pointer was used.                                              */
  l = strlen(opt);
  char str[++l];
  memcpy((void *)str, (const void *)opt, l);
  str[l] = '\0';

  for (i = 0, str1 = str; ; i++, str1 = NULL) {
    token = strtok_r(str1, delim, &saveptr);
    if (token == NULL)
      break;
    
    /* 
     * FIXME: what follows here is a poor implementation of a parameter 
     * parser for options. I does its job but a better solution should be
     * implemented.
     */
    for (j = 0; j < VERBOSE_NB; j++) {
      ret = strncmp(token, verbosity_level_names[j], VLVL_LEN);
      if (ret == 0)
	break;
    }
    // missmatch in verbosity values
    if (j == VERBOSE_NB) {
      errno = EDOM;
      efa_errno = EFA_EPARSE_OPT_VERBOSE;
      efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
    }

    switch (j) {
    case VERBOSE_MEMORY:
      *verbose |= VLVL_MEMORY;
      break;
    case VERBOSE_TIME:
      *verbose |= VLVL_TIME;
      break;
    default:
      *verbose = 0;
      break;
    } 
  }

  return 0;
}

/*
 * efa_parse_opts:
 * Parse the standard options of libefa.
 */
void efa_parse_opts(int argc, char *argv[], fa_t *fa, efa_opts_t *opts) {
  char *opt_algorithm = NULL;
  const char *optstr;
  int c, opt_idx, ret;
  static int verbose_flag;
  unsigned int callback_algo, ds_parsed;
#ifdef _GNU_SOURCE
  uid_t real_uid, eff_uid;
#endif

#if HAVE_GETOPT_H
  static struct option long_options[] = {
    { "algorithm", required_argument, 0, 'a' },
    { "class", required_argument, 0, 'c' },
    { "data-structure", required_argument, 0, 'd' },
    { "help", optional_argument, 0, 'h' },
#if HAVE_MODULE_HASH
    { "hash", required_argument, 0, 'H' },
#endif
    { "input", required_argument, 0, 'i' },
    { "number", required_argument, 0, 'n' },
    { "output", required_argument, 0, 'o' },
    { "regexp", required_argument, 0, 'r' },
    { "schema", required_argument, 0, 's' },
    { "type", required_argument, 0, 't' },
    { "verbose", required_argument, 0, 'v' },
    { "version", no_argument, &verbose_flag, 1 },
    { 0, 0, 0, 0}
  };
#endif /* HAVE_GETOPT_H */

#if HAVE_MODULE_HASH
  optstr = "a:c:d:h::H:i:n:o:r:s:t:v:0";
#else
  optstr = "a:c:d:h::i:n:o:r:s:t:v:0";
#endif
  
  do {
    opt_idx = 0;
#if HAVE_GETOPT_H    
    c = getopt_long(argc, argv, optstr, long_options, &opt_idx);
#else
    c = getopt(argc, argv, optstr);
#endif
    
    if (verbose_flag)
      efa_display_version();
    
    switch(c) {
    case 'a':
      if (!ds_parsed) {
	opt_algorithm = optarg;
	callback_algo = 1;
	break;
      }
      efa_parse_opt_algorithm(&opts->algorithms, fa, optarg);
      break;
    case 'c':
      efa_parse_opt_class(fa, optarg);
      break;
    case 'd':
      efa_parse_opt_ds(fa, optarg);
      ds_parsed = 1;
      if (callback_algo)
	efa_parse_opt_algorithm(&opts->algorithms, fa, opt_algorithm);
      break;
    case 'h':
      efa_parse_opt_usage(optarg, opts->program_name);
      break;
#if HAVE_MODULE_HASH
    case 'H':
      opts->function = optarg;
      break;
#endif
    case 'i':
      opts->input_file_path = optarg;
      break;
    case 'n':
      opts->nb_automata = atoi(optarg);
      break;
    case 'o':
      opts->output_file_path = optarg;
      // check if the file already exists in a secure way
#ifdef _GNU_SOURCE      
      real_uid = getuid();
      eff_uid = geteuid();
      ret = setuid(real_uid);
      if (ret == -1) {
	efa_errno = EFA_ESETUID;
	efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
	exit(EXIT_FAILURE);
      }
      ret = access(opts->output_file_path, F_OK);
      if (ret == 0) {
	errno = EEXIST;
	efa_errno = EFA_EPARSE_OPTS;
	efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
	exit(EXIT_FAILURE);
      } else if ((ret == -1) && (errno != ENOENT)) {
	efa_errno = EFA_EPARSE_OPTS;
	efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
      }
      ret = setuid(eff_uid);
      if (ret == -1) {
	efa_errno = EFA_ESETUID;
	efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
	exit(EXIT_FAILURE);
      }
#elif _WIN32 || _WIN64
      _access_s(opts->output_file_path, 00);
#endif
      break;
    case 'r':
      opts->regexp = optarg;
      break;
    case 's':
      opts->schema_file_path = optarg;
      break;
    case 't':
      efa_parse_opt_at(fa, optarg);
      break;
    case 'v':
      efa_parse_opt_verbose(optarg, &opts->opt_verbose);
      break;
    case -1:
      break;
    default:
      efa_display_usage(opts->program_name);
      break;
    }
  } while (c != -1);
}

/*
 * efa_end_timer:
 * Stops the timer and returns a pointer to a time structure holding the 
 * difference. The user has to free the memory of the returned time structure.
 * If an error occurs NULL is returned.
 */
struct timespec *efa_end_timer(void) {
  int ret;
  struct timespec *ts_diff = NULL;
  
  // check if timer has been started
  if (ts_start == NULL || ts_end == NULL) {
    efa_errno = EFA_EEND_TIMER;
    return NULL;
  }
  
  ret = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, ts_end);
  if (ret == -1) {
    efa_errno = EFA_ETIMER;
    return NULL;
  }
  
  ret = efa_alloc(&ts_diff, sizeof(struct timespec), 1);
  if (ret == -1)
    return NULL;
 
  ts_diff->tv_sec = ts_end->tv_sec - ts_start->tv_sec;
  ts_diff->tv_nsec = ts_end->tv_nsec - ts_start->tv_nsec;
  
  if (ts_diff->tv_nsec < 0)
    ts_diff->tv_nsec = ((signed long int) 1000000000) + ts_diff->tv_nsec;
  
  efa_free(ts_start);
  efa_free(ts_end);
  
  assert(ts_start == NULL);
  assert(ts_end == NULL);
  
  return ts_diff;
}

/*
 * efa_start_timer:
 * Starts the timer. On success 0 is returned, if an error occurs -1 is 
 * returned.
 */
int efa_start_timer(void) {
  int ret;
  
  // check if timer is in use
  if (ts_start != NULL) {
    errno = EBUSY;
    efa_errno = EFA_ESTART_TIMER;
    return -1;
  }
  
  ret = efa_alloc(&ts_start, sizeof(struct timespec), 1);
  if (ret == -1)
    return -1;
    
  ret = efa_alloc(&ts_end, sizeof(struct timespec), 1);
  if (ret == -1)
    return -1;
  
  ret = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, ts_start);
  if (ret == -1) {
    efa_errno = EFA_ETIMER;
    return -1;
  }
  
  return 0;
}

/*
 * regexp_read:
 * Reads a regular expression and produces an epsilon-non-deterministic 
 * automaton.
 */
#if HAVE_MODULE_HASH
int regexp_hash_read(fa_t *fa __attribute__((unused)),
		     const char *regexp __attribute__((unused)), 
		     unsigned char verbose __attribute__((unused)),
		     void *data __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EREGEXP_READ;
  efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int regexp_rbtree_read(fa_t *fa __attribute__((unused)),
		       const char *regexp __attribute__((unused)), 
		       unsigned char verbose __attribute__((unused)),
		       void *data __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EREGEXP_READ;
  efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

/*
 * regexp_write:
 * Writes a finite automaton to a regular expression.
 */
#if HAVE_MODULE_HASH
int regexp_hash_write(fa_t *fa __attribute__((unused)),
		      const char *regexp __attribute__((unused)), 
		      unsigned char verbose __attribute__((unused)),
		      void *data __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EREGEXP_READ;
  efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int regexp_rbtree_write(fa_t *fa __attribute__((unused)),
			const char *regexp __attribute__((unused)), 
			unsigned char verbose __attribute__((unused)),
			void *data __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EREGEXP_READ;
  efa_error(EFA_MOD_CORE, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif
