#include "efatooltests.h"

#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <libefa/efa.h>
#include <libefa/memory.h>

#if HAVE_MODULE_FADOT
# include <libefa/fadot.h>
#endif

#if HAVE_MODULE_FAXML
# include <libefa/faxml.h>
#endif

#if HAVE_MODULE_HASH
# include <libefa/hash.h>
#endif

#if HAVE_MODULE_RBTREE
# include <libefa/rbtree.h>
#endif

/* Global variables with default option */

// Comma separated list of algorithm names
static const char *opt_algorithm = NULL;

// Type of automaton
static const char *opt_at = NULL;

// Class of automaton
static const char *opt_class = NULL;

// Data structure of the automaton
static const char *opt_ds = NULL;

#if HAVE_MODULE_HASH
// Hash function to use
static const char *opt_hash = NULL;
#endif

// Comma separated list of verbosity levels
static const char *opt_verbose = NULL;

// Regular expression to turn into an automaton
static const char *regexp = NULL;

/* Local functions */
int parse_opt_algorithm(algorithm_fun_t **);
int parse_opt_at(Automaton *);
int parse_opt_class(Automaton *);
int parse_opt_ds(Automaton *);
int parse_opt_verbose(Automaton *);
void parse_opts(int argc, char *argv[]);

#if HAVE_MODULE_HASH
int parse_opt_hash(Automaton *);
#endif

/*
 * parse_opt_algorithm:
 * Parse the parameters given to the algorithm option.
 */
int parse_opt_algorithm(algorithm_fun_t **algorithms) {
  char *token, *subtoken, *str1, *str2, *saveptr1 = NULL, *saveptr2 = NULL;
  const char *delim1 = ",";
  const char *delim2 = "=";
  int i, j, k, key = 1, nalgorithms = 0, ret;
  size_t l = 0;

  /* Nasty casting of character pointer to character array. This is needed as
     strtok_r() at line 115 would raise a segmentation fault (SIGSEGV) if a
     character pointer was used.                                              */
  l = strlen(opt_algorithm);
  char str[++l];
  memcpy((void *)str, (const void *)opt_algorithm, l);
  str[l] = '\0';

  // count the number of algorithms
  str1 = (char *)opt_algorithm;
  do {
    nalgorithms++;
    str1++;
    str1 = strchr((const char *)str1, (int) delim1);
  } while(str1);

  // allocate an array of function pointers that is terminated by a zero entry
  ret = ALLOC_N(*algorithms, ++nalgorithms);
  if (ret == -1) {
    fprintf(stderr, " [EFATOOL] Not enough memory to allocate a new algorithm \
list.\n");
    return 1;
  }

  for (j = 0, str1 = str; ; j++, str1 = NULL) {
    token = strtok_r(str1, delim1, &saveptr1);
    if (token == NULL)
      break;
#if DEBUG
    printf("%d: %s\n", j, token);
#endif
  
    for (str2 = token; ; str2 = NULL) {
      subtoken = strtok_r(str2, delim2, &saveptr2);
      if ((subtoken == NULL) && (key == 0)) {
	fprintf(stderr, " [EFATOOL] No value was given to key '%s'.\n", token);
	return 1;
      } else if (subtoken == NULL)
	break;
#if DEBUG
      printf(" --> %s\n", subtoken);
#endif

      /* 
       * FIXME: what follows here is a poor implementation of a parameter 
       * parser for options. I does its job but a better solution should be
       * implemented.
       */
      if (key) // looking for a key token
	for (i = 0; i < ALGO_T_NB; i++) {
	  ret = strncmp(subtoken, algorithm_type_names[i], 2);
	  if (ret == 0) { // key found
	    key = 0;
	    break;
	  }
	}
      else // looking for a value token
	for (k = 0; k < ALGO_NB; k++) {
	  ret = strncmp(subtoken, algorithm_names[k], 2);
	  if (ret == 0) { // value found
	    *algorithms[j] = algorithm_functions[k];
	    key = 1;
	    break;
	  }
	}
    }
  }
  
  return 0;
}

/*
 * parse_opt_at:
 * Parse the parameter given to the automaton type option.
 */
int parse_opt_at(Automaton *automaton) {
  int i, ret;

  for (i = 0; i < AUTOMATON_TYPE_NB; i++) {
    ret = strncmp(opt_at, automaton_type_names[i], 6);
    if (ret == 0) { // automaton type names match
      break;
    }
  }
  if (i == AUTOMATON_TYPE_NB) {
    fprintf(stderr, " [EFATOOL] Wrong or incorrect automaton type '%s'.\n",
	    opt_at);
    return 1;
  }

  if (automaton->type) {
    if (automaton->type != i) {
      fprintf(stderr, " [EFATOOL] Automaton types do not match (automaton: \
	      %d, option: %d).\n", automaton->type, i);
      return 1;
    }
  } else
    automaton->type = i;

  return 0;
}

/*
 * parse_opt_class:
 * Parse the parameter given to the class option.
 * FIXME: poor parser implementation.
 */
int parse_opt_class(Automaton *automaton) {
  int i, ret;
  
  for (i = 0; i < AUTOMATON_CLASS_NB; i++) {
    ret = strncmp(opt_class, automaton_class_names[i], 25);
    if (ret == 0) { // class names match
      break;
    }
  }
  if (i == AUTOMATON_CLASS_NB) {
    fprintf(stderr, " [EFATOOL] Wrong or incorrect automaton class '%s'.\n",
	    opt_class);
    return 1;
  }

  if (automaton->class) {
    if (automaton->class != i) {
      fprintf(stderr, " [EFATOOL] Automaton class and class option do not \
match.\n");
      return 1;
    }
  } else
    automaton->class = i;

  return 0;
}

/*
 * parse_opt_ds:
 * Parse the parameter given to the data structure option.
 */
int parse_opt_ds(Automaton *automaton) {
  int i, ret;

  for (i = 0; i < DATA_TYPE_NB; i++) {
    ret = strncmp(opt_ds, data_type_names[i], 14);
    if (ret == 0) { // data type names match
      break;
    }
  }

  switch (i) {
#if HAVE_MODULE_HASH
  case HASH_TABLE:
    automaton->data_type = HASH_TABLE;

    automaton->data_structure = calloc(1, sizeof(hash_t));
    if (automaton->data_structure == NULL) {
      fprintf(stderr, " [EFATOOL] Not enough memory to allocate a hash \
table.\n");
      return 1;
    }
    break;
#endif

#if HAVE_MODULE_RBTREE
  case RB_TREE:
    automaton->data_type = RB_TREE;

    automaton->data_structure = calloc(1, sizeof(rbtree_t));
    if (automaton->data_structure == NULL) {
      fprintf(stderr, " [EFATOOL] Not enough memory to allocate a \
red-black-tree.\n");
      return 1;
    }
    break;
#endif

  default:
    fprintf(stderr, " [EFATOOL] Error while parsing data structure option \
%d.\n", i);
    return 1;
    break;
  }

  return 0;
}

#if HAVE_MODULE_HASH

/*
 * parse_opt_hash:
 * Parse the parameter given to the hash option.
 */
int parse_opt_hash(Automaton *automaton) {
  int ret = 0;

  ret = hash_set_function((hash_t *)automaton->data_structure, opt_hash);

  return ret;
}

#endif

/*
 * parse_opt_verbose:
 * Parse the parameter given to the verbose option.
 */
int parse_opt_verbose(Automaton *automaton) {
  char *token, *str1, *saveptr;
  const char *delim = ",";
  int i, j, l = 0, ret;

  /* Nasty casting of character pointer to character array. This is needed as
     strtok_r() at line 115 would raise a segmentation fault (SIGSEGV) if a
     character pointer was used.                                              */
  l = strlen(opt_verbose);
  char str[++l];
  memcpy((void *)str, (const void *)opt_verbose, l);
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
      ret = strncmp(token, verbosity_level_names[j], 6);
      if (ret == 0) {
	break;
      }
    }
    // missmatch in verbosity values
    if (j == VERBOSE_NB) {
      fprintf(stderr, " [EFATOOL] Value missmatch for '%s'.\n", token);
      return 1;
    }

    switch (j) {
    case VERBOSE_MEMORY:
      automaton->verbosity.memory = 1;
      break;
    case VERBOSE_TIME:
      automaton->verbosity.time = 1;
      break;
    default:
      fprintf(stderr, " [EFATOOL] Error while parsing verbose option %d.\n", j);
      break;
    } 
  }

  return 0;
}

CuSuite *efatoolGetSuite(void) {
  CuSuite *suite = NULL;

  suite = CuSuiteNew();
  // parse_opt_algorithm() tests
  SUITE_ADD_TEST(suite, Test_1_parse_opt_algorithm);
  SUITE_ADD_TEST(suite, Test_2_parse_opt_algorithm);

  // parse_opt_at() tests
  SUITE_ADD_TEST(suite, Test_1_parse_opt_at);
  SUITE_ADD_TEST(suite, Test_2_parse_opt_at);
  SUITE_ADD_TEST(suite, Test_3_parse_opt_at);
#if HAVE_MODULE_BUECHI
  SUITE_ADD_TEST(suite, Test_4_parse_opt_at);
#endif

  // parse_opt_class() tests
  SUITE_ADD_TEST(suite, Test_1_parse_opt_class);
  SUITE_ADD_TEST(suite, Test_2_parse_opt_class);
  SUITE_ADD_TEST(suite, Test_3_parse_opt_class);
  SUITE_ADD_TEST(suite, Test_4_parse_opt_class);

  // parse_opt_ds() tests
  SUITE_ADD_TEST(suite, Test_1_parse_opt_ds);
  SUITE_ADD_TEST(suite, Test_2_parse_opt_ds);

  // parse_opt_hash() tests
  SUITE_ADD_TEST(suite, Test_1_parse_opt_hash);
  SUITE_ADD_TEST(suite, Test_2_parse_opt_hash);

  // parse_opt_verbose() tests
  SUITE_ADD_TEST(suite, Test_1_parse_opt_verbose);
  SUITE_ADD_TEST(suite, Test_2_parse_opt_verbose);

  return suite;
}

/*
 * parse_opt_algorithm() Test 1:
 * Test the function for normal input.
 */
void Test_1_parse_opt_algorithm(CuTest *tc) {
  algorithm_fun_t *input_fun = NULL;
  const char *input_opt = "complementation=unifr";
  int ac_int = 0, ex_int = 0;
  void *ex_ptr = NULL;

  puts("===== PARSE_OPT_ALGORITHM() TEST 1 ==============================================\n");

  opt_algorithm = input_opt;
  ex_ptr = (void *)algorithm_functions[ALGO_COMP_UNIFR];
  ac_int = parse_opt_algorithm(&input_fun);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertPtrNotNull(tc, input_fun);
  CuAssertPtrEquals(tc, ex_ptr, input_fun[0]);
  
  FREE(input_fun);
}

/*
 * parse_opt_algorithm() Test 2:
 * Test the function for corrupted input.
 */
void Test_2_parse_opt_algorithm(CuTest *tc) {
  algorithm_fun_t *input_fun = NULL;
  const char *input_opt = "complementation";
  int ac_int = 0, ex_int = 1;
  void *ex_ptr = NULL;
  
  puts("===== PARSE_OPT_ALGORITHM() TEST 2 ==============================================\n");
  
  opt_algorithm = input_opt;
  ac_int = parse_opt_algorithm(&input_fun);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertPtrNotNull(tc, input_fun);
  CuAssertPtrEquals(tc, ex_ptr, input_fun[0]);
  
  FREE(input_fun);
}

/*
 * parse_opt_at() Test 1:
 * Test the function for normal input.
 */
void Test_1_parse_opt_at(CuTest *tc) {
  Automaton *automaton = NULL;
  int ac_int = 0, ex_int = 0, ret = 0;

#if HAVE_MODULE_BUECHI
  const char *input_opt = "buechi";
#else
  const char *input_opt = "normal";
#endif

  opt_at = input_opt;
  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  ac_int = parse_opt_at(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);

#if HAVE_MODULE_BUECHI
  CuAssertIntEquals(tc, AUTOMATON_BUECHI, automaton->type);
#else
  CuAssertIntEquals(tc, AUTOMATON_NORMAL, automaton->type);
#endif

  FREE(automaton);
}

/*
 * parse_opt_at() Test 2:
 * Test the function for corrupted input.
 */
void Test_2_parse_opt_at(CuTest *tc) {
  Automaton *automaton = NULL;
  int ac_int = 0, ex_int = 1, ret = 0;

#if HAVE_MODULE_BUECHI
  const char *input_opt = "buech";
#else
  const char *input_opt = "norma";
#endif

  opt_at = input_opt;
  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  ac_int = parse_opt_at(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);

  FREE(automaton);
}

/*
 * parse_opt_at() Test 3:
 * Test the function for matching values, if the XML file states for example
 * a 'normal' automaton and the command line a 'normal' automaton.
 */
void Test_3_parse_opt_at(CuTest *tc) {
  Automaton *automaton = NULL;
  int ac_int = 0, ex_int = 0, ret = 0;
  const char *input_opt = "normal";

  opt_at = input_opt;
  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  automaton->type = AUTOMATON_NORMAL;
  ac_int = parse_opt_at(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertIntEquals(tc, AUTOMATON_NORMAL, automaton->type);

  FREE(automaton);
}

#if HAVE_MODULE_BUECHI

/*
 * parse_opt_at() Test 4:
 * Test the function for missmatching values, if the XML file states for example
 * a 'buechi' automaton and the command line a 'normal' automaton.
 */
void Test_4_parse_opt_at(CuTest *tc) {
  Automaton *automaton = NULL;
  int ac_int = 0, ex_int = 1, ret = 0;
  const char *input_opt = "normal";

  opt_at = input_opt;
  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  automaton->type = AUTOMATON_BUECHI;
  ac_int = parse_opt_at(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertIntEquals(tc, AUTOMATON_BUECHI, automaton->type);

  FREE(automaton);
}

/*
 * parse_opt_class() Test 1:
 * Test the function for normal intput.
 */
void Test_1_parse_opt_class(CuTest *tc) {
  Automaton *automaton = NULL;
  const char *input_opt = "deterministic";
  int ac_int = 0, ex_int = 0, ret = 0;

  opt_class = input_opt;
  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  ac_int = parse_opt_class(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertIntEquals(tc, AUTOMATON_DETERMINISTIC, automaton->class);
  
  FREE(automaton);
}

/*
 * parse_opt_class() Test 2:
 * Test the function for corrupted intput.
 */
void Test_2_parse_opt_class(CuTest *tc) {
  Automaton *automaton = NULL;
  const char *input_opt = "non-determinstic";
  int ac_int = 0, ex_int = 1, ret = 0;

  opt_class = input_opt;
  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  ac_int = parse_opt_class(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertIntEquals(tc, 0, automaton->class);
  
  FREE(automaton);
}

/*
 * parse_opt_class() Test 3:
 * Test the function for matching values, if the XML file states for example a
 * 'deterministic' automaton and the command line too.
 */
void Test_3_parse_opt_class(CuTest *tc) {
  Automaton *automaton = NULL;
  const char *input_opt = "deterministic";
  int ac_int = 0, ex_int = 0, ret = 0;

  opt_class = input_opt;
  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  automaton->class = AUTOMATON_DETERMINISTIC;
  ac_int = parse_opt_class(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertIntEquals(tc, AUTOMATON_DETERMINISTIC, automaton->class);
  
  FREE(automaton);
}

/*
 * parse_opt_class() Test 4:
 * Test the function for matching values, if the XML file states for example
 * a 'non-deterministic' automaton and the command line a 'deterministic'
 * automaton.
 */
void Test_4_parse_opt_class(CuTest *tc) {
  Automaton *automaton = NULL;
  const char *input_opt = "deterministic";
  int ac_int = 0, ex_int = 1, ret = 0;

  opt_class = input_opt;
  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  automaton->class = AUTOMATON_NON_DETERMINISTIC;
  ac_int = parse_opt_class(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertIntEquals(tc, AUTOMATON_NON_DETERMINISTIC, automaton->class);
  
  FREE(automaton);
}

#endif /* HAVE_MODULE_BUECHI */

/*
 * parse_opt_ds() Test 1:
 * Test the function for normal input. 
 */
void Test_1_parse_opt_ds(CuTest *tc) {
  Automaton *automaton = NULL;
  int ac_int = 0, ex_int = 0, ret = 0;

#if HAVE_MODULE_HASH
  const char *input_opt = "hashtable";
#else
  const char *input_opt = "red-black-tree";
#endif

  opt_ds = input_opt;
  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  ac_int = parse_opt_ds(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);

#if HAVE_MODULE_HASH
  CuAssertIntEquals(tc, HASH_TABLE, automaton->data_type);
#else
  CuAssertIntEquals(tc, RB_TREE, automaton->data_type);
#endif

  CuAssertPtrNotNull(tc, automaton->data_structure);

  FREE(automaton->data_structure);
  FREE(automaton);
}

/*
 * parse_opt_ds() Test 2:
 * Test the function for corrupted input.
 */
void Test_2_parse_opt_ds(CuTest *tc) {
  Automaton *automaton = NULL;
  int ac_int = 0, ex_int = 1, ret = 0;

#if HAVE_MODULE_HASH
  const char *input_opt = "hash";
#else
  const char *input_opt = "tree";
#endif

  opt_ds = input_opt;
  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  ac_int = parse_opt_ds(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);

#if HAVE_MODULE_HASH
  CuAssertIntEquals(tc, 0, automaton->data_type);
#else
  CuAssertIntEquals(tc, 0, automaton->data_type);
#endif

  FREE(automaton);
}

#if HAVE_MODULE_HASH

/*
 * parse_opt_hash() Test 1:
 * Test the function for normal input.
 */
void Test_1_parse_opt_hash(CuTest *tc) {
  Automaton *automaton = NULL;
  const char *input_opt = "kazlib";
  int ac_int = 0, ex_int = 0, ret = 0;
  hash_fun_t ex_ptr = hash_functions[HASH_FUN_KAZLIB];

  opt_hash = input_opt;

  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  automaton->data_type = HASH_TABLE;
  automaton->data_structure = calloc(1, sizeof(hash_t));
  CuAssertPtrNotNull(tc, automaton->data_structure);

  ac_int = parse_opt_hash(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertPtrEquals(tc, ex_ptr,
		    ((hash_t *)automaton->data_structure)->function);

  FREE(automaton->data_structure);
  FREE(automaton);
}

/*
 * parse_opt_hash() Test 2:
 * Test the function for corrupted input.
 */
void Test_2_parse_opt_hash(CuTest *tc) {
  Automaton *automaton = NULL;
  const char *input_opt = "kazli";
  int ac_int = 0, ex_int = 1, ret = 0;

  opt_hash = input_opt;

  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  automaton->data_type = HASH_TABLE;
  automaton->data_structure = calloc(1, sizeof(hash_t));
  CuAssertPtrNotNull(tc, automaton->data_structure);

  ac_int = parse_opt_hash(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);

  FREE(automaton->data_structure);
  FREE(automaton);
}

#endif /* HAVE_MODULE_HASH */

/*
 * parse_opt_verbose() Test 1:
 * Test the function for normal input.
 */
void Test_1_parse_opt_verbose(CuTest *tc) {
  Automaton *automaton = NULL;
  const char *input_opt = "time";
  int ac_int = 0, ex_int = 0, ret = 0;

  opt_verbose = input_opt;

  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  ac_int = parse_opt_verbose(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertIntEquals(tc, 1, automaton->verbosity.time);

  FREE(automaton);
}

/*
 * parse_opt_verbose() Test 2:
 * Test the function for corrupted input.
 */
void Test_2_parse_opt_verbose(CuTest *tc) {
  Automaton *automaton = NULL;
  const char *input_opt = "timer";
  int ac_int = 0, ex_int = 1, ret = 0;

  opt_verbose = input_opt;

  ret = ALLOC(automaton);
  CuAssertIntEquals(tc, 0, ret);
  CuAssertPtrNotNull(tc, automaton);

  ac_int = parse_opt_verbose(automaton);
  CuAssertIntEquals(tc, ex_int, ac_int);
  CuAssertIntEquals(tc, 0, automaton->verbosity.time);

  FREE(automaton);
}
