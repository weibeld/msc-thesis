/*
 * efa.h: private headers for efa
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Author: Christian Göttel <christian.goettel@unifr.ch>
 */

#ifndef EFA_H_
#define EFA_H_

#include "config.h"

#if HAVE_MODULE_HASH
# include "libefa/hash.h"
#endif

#include <limits.h>  
#include <stdlib.h>
#include <time.h>

#if __linux__
# if __GNUG__
#  include <features.h>
# endif /* __GNUG__ */
#elif _WIN64
# define __WORDSIZE 64
#elif _WIN32
# define __WORDSIZE 32
#endif /* __linux__ */

#ifndef DEBUG
# define NDEBUG
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * algorithm_name:
 * Enumeration of complementation algorithms.
 */
enum algorithm_comp_name {
#if HAVE_MODULE_FAMOD
  ALGO_COMP_ORDINARY, // complementation for ordinary finite automata
#endif
#if HAVE_MODULE_BUECHI && HAVE_MODULE_FAMOD
  ALGO_COMP_UNIFR,  /* complementation algorithm for Büchi automata developed
                      developed at the University of Fribourg, Switzerland.  */
  ALGO_COMP_UNIFR2, // same as ALGO_COMP_UNIFR with an addition optimization
  ALGO_COMP_UNIFR3, // same as ALGO_COMP_UNIFR2 without optimization
#endif
  ALGO_COMP_NB
};

/*
 * algorithm_equiv_name:
 * Enumeration of equivalence algorithms.
 */
enum algorithm_equiv_name {
#if HAVE_MODULE_FAMOD
  ALGO_EQUIV_TABLE_FILLING,
#endif
  ALGO_EQUIV_NB
};

/*
 * algorithm_gen_name:
 * Enumeration of generator algorithms.
 */
enum alogrithm_gen_name {
#if HAVE_MODULE_FAMOD
  ALGO_GEN_BOLTZMANN_SAMPLERS,
  ALGO_GEN_RECURSIVE_METHOD,
#endif
  ALGO_GEN_NB
};

/*
 * algorithm_min_name:
 * Enumeration of minimization algorithms.
 */
enum algorithm_min_name {
  // minimization algorithms
#if HAVE_MODULE_FAMOD
  ALGO_MIN_BRZOZOWSKI,
  ALGO_MIN_HOPCROFT,
#endif
  ALGO_MIN_NB
};

/*
 * algorithm_run_name:
 * Enumeration of run algorithms.
 */
enum algorithm_run_name {
#if HAVE_MODULE_FAMOD
  ALGO_RUN_ORDINARY,
#endif
  ALGO_RUN_NB
};

/*
 * algorithm_trans_name:
 * Enumeration of transformation algorithms.
 */
enum algorithm_trans_name {
#if HAVE_MODULE_FAMOD
  ALGO_TRANS_COLLECT,
  ALGO_TRANS_COMPLETE,
  ALGO_TRANS_SUBSET_CONSTRUCTION,
#endif
  ALGO_TRANS_NB
};

/*
 * algorithm_names:
 * Arrays with algorithm names.
 */
extern const char *const algorithm_comp_names[];
extern const char *const algorithm_equiv_names[];
extern const char *const algorithm_gen_names[];
extern const char *const algorithm_min_names[];
extern const char *const algorithm_run_names[];
extern const char *const algorithm_trans_names[];

#define ALGO_COMP_LEN 8
#define ALGO_EQUIV_LEN 13
#define ALGO_GEN_LEN 18
#define ALGO_MIN_LEN 10
#define ALGO_RUN_LEN 8
#define ALGO_TRANS_LEN 19
#define ALGO_TYPE_LEN 15

/*
 * algorithm_type_names:
 * Array with algorithm type names.
 */
extern const char *const algorithm_type_names[];

/*
 * algorithm_type_name:
 * Types of algorithms supported by libefa.
 */
enum algorithm_type_name {
  ALGO_T_COMPLEMENTATION,	// complementation algorithm
  ALGO_T_EQUIVALENCE,		// equivalence algorithm
  ALGO_T_GENERATOR,		// generator algorithm
  ALGO_T_MINIMIZATION,		// minimization algorithm
  ALGO_T_RUN,			// run algorithm
  ALGO_T_TRANSFORMATION,	// transformation algorithm
  ALGO_T_NB			// number of algorithm types
};

/*
 * data_type_names:
 * Array with data type names.
 */
extern const char *const data_type_names[];

#define DT_LEN 14

/*
 * data_type_name:
 * Enumeration of supported data types.
 */
enum data_type_name {
#if HAVE_MODULE_HASH
  DT_HASH_TABLE,
#endif
#if HAVE_MODULE_RBTREE
  DT_RB_TREE,
#endif
  DT_NB,
  DT_NO
};

/*
 * efa_fa_class_names:
 * Array with class names.
 */
extern const char *const efa_fa_class_names[];

#define FA_CLASS_LEN 25

/*
 * efa_fa_class_name:
 * Enumeration of supported class names by libefa.
 */
enum efa_fa_class_name {
  FA_CLASS_DETERMINISTIC,		// deterministic
  FA_CLASS_NON_DETERMINISTIC,		// non-deterministic
  FA_CLASS_EPSILON_NON_DETERMINISTIC,	// epsilon-non-deterministic
  FA_CLASS_NB				// number of classes
};

/*
 * efa_module_names:
 * Array with module names.
 */
extern const char *const efa_module_names[];

/*
 * efa_module_name:
 * Enumeration of supported modules.
 */
typedef enum efa_module_name {
#if HAVE_MODULE_BUECHI
  EFA_MOD_BUECHI,
#endif
  EFA_MOD_CORE,
#if HAVE_MODULE_FADOT
  EFA_MOD_FADOT,
#endif
#if HAVE_MODULE_FAMOD
  EFA_MOD_FAMOD,
#endif
#if HAVE_MODULE_FAXML
  EFA_MOD_FAXML,
#endif
#if HAVE_MODULE_HASH
  EFA_MOD_HASH,
#endif
#if HAVE_MODULE_RBTREE
  EFA_MOD_RBTREE,
#endif
  EFA_MOD_TOOL,
  EFA_MOD_NB
} efa_mod_t;

/*
 * efa_fa_type_names:
 * Array with automaton type names.
 */
extern const char *const efa_fa_type_names[];

#define FA_TYPE_LEN 8

/*
 * efa_fa_type_name:
 * Enumeration of supported automaton types.
 */
enum efa_fa_type_name {
  FA_TYPE_ORDINARY,
#if HAVE_MODULE_BUECHI
  FA_TYPE_BUECHI,
#endif
  FA_TYPE_NB
};

/*
 * file_extension_names:
 * Array with file extension names.
 */
extern const char *const file_extension_names[];

/*
 * file_extension_name:
 * Enumeration of supported file extensions.
 */
enum file_extension_name {
#if HAVE_MODULE_FADOT
  FILE_EXTENSION_DOT,
#endif
  FILE_EXTENSION_REGEXP,
#if HAVE_MODULE_FAXML
  FILE_EXTENSION_XML,
#endif
  FILE_EXTENSION_NB
};

/*
 * verbosity_level_names:
 * Array with verbosity level names.
 */
extern const char *const verbosity_level_names[];

#define VLVL_LEN 6
#define VLVL_MEMORY 1u
#define VLVL_TIME 2u

/*
 * verbosity_level_name:
 * Enumeration of supported verbosity levels.
 */
enum verbosity_level_name {
  VERBOSE_MEMORY,
  VERBOSE_TIME,
  VERBOSE_NB
};

/*
 * algorithm_node:
 * A structure to chain algorithms.
 * 
 * TYPE: needs to be an enumeration of 'algorithm_type_name' in order to 
 * identify the the of algorithm function.
 * 
 * FUNCTION: a pointer to an algorithm function.
 */
typedef struct algorithm_node {
  int type;
  void *function;
} anode_t;

/*
 * EFA options:
 * A structure holding flags and string values after having parsed the argument 
 * options by efa_parse_opt().
 * 
 * ALGORITHMS: an array of algorithm nodes that terminates with a algorithm 
 * node having a point to NULL in the function field.
 * 
 * INPUT_FILE_PATH: a string with the PATH and FILENAME of the input file. The 
 * FILENAME needs to have a file extension corresponding to a string value in
 * 'efa_file_extension_names'.
 * 
 * OUTPUT_FILE_PATH: a string with the PATH and FILENAME of the output file. The
 * FILENAME needs to have a file extension corresponding to a string value in
 * 'efa_file_extension_names' and must be non existent.
 * 
 * OPT_VERBOSE: a string with a comma separated list of verbosity levels. Must 
 * be a string value of 'efa_verbose_names'.
 * 
 * REGEXP: a string with a regular expression that will be turned into an 
 * automaton.
 * 
 * NB_AUTOMATA: a counter variable that indicates the number of automata the 
 * library will generate. With this implementation you cannot generate more 
 * than 2^32 - 1 = 4294967295 automata.
 * 
 * SCHEMA_FILE_PATH: a string with the PATH and FILENAME of the XML Schema. The 
 * FILENAME must exist and needs to have a '.xsd' file extension. A default 
 * value called FAXML_SCHEMA is defined in 'faxml.h'.
 */
typedef struct efa_options {
  anode_t *algorithms;
  const char *program_name;
  const char *input_file_path;
  const char *output_file_path;
  unsigned char in_ext : 4;
  unsigned char out_ext : 4;
  unsigned char opt_verbose;
  const char *regexp;
  unsigned int nb_automata;
#if HAVE_MODULE_FAXML
  const char *schema_file_path;
#endif
#if HAVE_MODULE_HASH
  const char *function;
#endif
} efa_opts_t;

/*
 * Finite automaton:
 * A descriptor of a general finite automaton.
 * 
 * OPT_DATA_STRUCTURE: a string defining the type of data structure to use. 
 * Must be a string value of 'efa_data_structure_names'.
 * 
 * OPT_FA_TYPE: a string defining the type of the finite automaton. Must be a 
 * string value of 'efa_fa_type_names'.
 * 
 * OPT_FA_CLASS: a string defining the class of the finite automaton. Must be 
 * a string value of 'efa_fa_class_names'.
 * 
 * OPT_HASH: a string defining the hash function to use with the hash data 
 * type. Must be a string value of 'efa_hash_fun_names'.
 */
typedef struct finite_automaton {
  void *data_structure; /* pointer to the data structure used to store the 
			   automaton.                                        */
  const char *alphabet; /* pointer to a string holding each symbol of the
			   alphabet                                          */
  const char *name; // name of the automaton
  unsigned int type : 3;   // type of automaton
  unsigned int class : 2; /* 0 = deterministic, 1 = non-deterministic,
			     2 = epsilon-non-deterministic                   */
  unsigned int minimal : 1; // is the automaton already a minimal automaton?
  unsigned int trans_re : 1; // are transitions regular expressions?
  unsigned int data_type : 3; /* defines the type of data structure used to
				 store the automaton.                        */
  unsigned int nb_symbols : 16; /* number of symbols in the alphabet. Is 
				   equivalent to the length of the string.   */
} fa_t;

/*
 * State:
 * A descriptor of a finite automaton state.
 * 4. transition_row can be eighter of two things: if it is a non-deterministic
 *    automaton, it is a pointer to an array of pointer to an array of pointers
 *    to hash nodes. If it is a deterministic automaton, it is a pointer to an
 *    array of pointers to a hash node.
 */
typedef struct state {
  const char *label;
  unsigned int accept : 1; // is the state an accepting state
  unsigned int live : 1;  // is the state used during a run
  unsigned int reachable : 1; /* is the state reachable by any transition of
				 another state                              */
  unsigned int visited : 1; // used by multiple algorithms
  void **transition_row;		/* 4 */
} state_t;

/*
 * Set of states:
 * A descriptor for a set of states.
 */
typedef struct set_of_states {
  const char *label;
  void **states;
  unsigned int accept : 1;
  unsigned int live : 1;
  unsigned int reachable : 1;
  unsigned int visited : 1;
#if HAVE_MODULE_BUECHI
  unsigned int buechi_info : 2;
  unsigned int mixed : 1;
#endif
  void **transition_row;
} sos_t;

typedef int (*algorithm_comp_t)(fa_t *, unsigned char);
typedef int (*algorithm_equiv_t)(fa_t *, fa_t *);
typedef int (*algorithm_gen_t)(fa_t *);
typedef int (*algorithm_min_t)(fa_t *);
typedef int (*algorithm_run_t)(fa_t *, const char *);
typedef int (*algorithm_trans_t)(fa_t *);

/*
 * algorithm_handler:
 * Structure holding arrays with pointers to all types of algorithm functions.
 */
typedef struct algorithm_handler {
  algorithm_comp_t algo_comp_fun[ALGO_COMP_NB];
  algorithm_equiv_t algo_equiv_fun[ALGO_EQUIV_NB];
  algorithm_gen_t algo_gen_fun[ALGO_GEN_NB];
  algorithm_min_t algo_min_fun[ALGO_MIN_NB];
  algorithm_run_t algo_run_fun[ALGO_RUN_NB];
  algorithm_trans_t algo_trans_fun[ALGO_TRANS_NB];
} algo_handler_t;

typedef int (*automaton_read_t)(fa_t *, const char *, unsigned char, void *);
typedef int (*automaton_write_t)(fa_t *, const char *, unsigned char, void *);

/* 
 * I/O handler:
 * Structure holding arrays with pointers to I/O functions.
 */
typedef struct efa_io_handler {
  automaton_read_t fa_read[FILE_EXTENSION_NB];
  automaton_write_t fa_write[FILE_EXTENSION_NB];  
} efa_ioh_t;

#if HAVE_MODULE_HASH
extern algo_handler_t hash_algorithms;
extern efa_ioh_t hash_io;
#endif
#if HAVE_MODULE_RBTREE
extern algo_handler_t rbtree_algorithms;
extern efa_ioh_t rbtree_io;
#endif

extern const efa_ioh_t *const efa_dt_io[];

/*
 * efa_parse_opts:
 * Parse the standard options of libefa.
 * 
 * ARGC: number of arguments passed to the program
 * 
 * ARGV: argument vector
 * 
 * OPTS: options structure
 * 
 * FA: finite automaton
 */
void efa_parse_opts(int, char *[], fa_t *, efa_opts_t *);

/*
 * efa_end_timer:
 * Records the current clock value and returns the difference of the start and 
 * end time as timespec structure or NULL if an error occured.
 */
struct timespec *efa_end_timer(void);

/*
 * efa_start_timer:
 * Records the current clock value and returns 0 on success. If an error 
 * occured -1 is returned.
 */
int efa_start_timer(void);

/*
 * regexp_read:
 * Reads a regular expression and produces an epsilon-non-deterministic 
 * automaton.
 */
#if HAVE_MODULE_HASH
int regexp_hash_read(fa_t *, const char *, unsigned char, void *);
#endif
#if HAVE_MDOULE_RBTREE
int regexp_rbtree_read(fa_t *, const char *, unsigned char, void *);
#endif

/*
 * regexp_write:
 * Writes a finite automaton to a regular expression.
 */
#if HAVE_MODULE_HASH
int regexp_hash_write(fa_t *, const char *, unsigned char, void *);
#endif
#if HAVE_MDOULE_RBTREE
int regexp_rbtree_write(fa_t *, const char *, unsigned char, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EFA_H_ */
