/*
 * Dynamic Hash Table Data Type
 * Copyright (C) 1997 Kaz Kylheku <kaz@ashi.footprints.net>
 * Copyright (C) 2013 Christian Göttel <christian.goettel@unifr.ch>
 *
 * Free Software License:
 *
 * All rights are reserved by the author, with the following exceptions:
 * Permission is granted to freely reproduce and distribute this software,
 * possibly in exchange for a fee, provided that this copyright notice appears
 * intact. Permission is also granted to adapt this software to produce
 * derivative works, as long as the modified versions carry this copyright
 * notice and additional notices stating that the work has been modified.
 * This source code may be translated into executable form and incorporated
 * into proprietary software; there is no requirement for such software to
 * contain a copyright notice related to this source.
 *
 * $Id: hash.h,v 1.22.2.7 2000/11/13 01:36:45 kaz Exp $
 * $Name: kazlib_1_20 $
 * 
 * This file is originally from kzalib v1.20 used in augeas. This file has been
 * modified to fit the need of the efalibrary.
 */

#ifndef HASH_H
#define HASH_H

#include "libefa/efa.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KEY_MAX_LENGTH 100

/*
 * Hash chain node structure (similar to automaton state).
 * Notes:
 * 1. This is a pointer to the next node in the chain. In the last node of a
 *    chain, this pointer is null.
 * 2. The key is a pointer to the automaton state label, which is a string. When
 *    creating or initializing a hash table, the user must supply a pointer to a
 *    function for comparing two keys, and a pointer to a function for hashing a
 *    key into a numeric value.
 * 3. The hashed key is stored in each node so that we don't have to rehash
 *    each key when the table must grow or shrink.
 */
typedef struct hash_node {
  struct hash_node *next;	/* 1 */
  const char *key;		/* 2 */
#if __WORDSIZE == 32
  uint32_t hkey;		/* 3 */
#elif __WORDSIZE == 64
  uint64_t hkey;
#endif
  void *data;
} hnode_t;

/*
 * The comparison function pointer type. A comparison function takes two keys
 * and produces a value of -1 if the left key is less than the right key, a
 * value of 0 if the keys are equal, and a value of 1 if the left key is
 * greater than the right key.
 */
typedef int (*hash_comp_t)(const char *, const char *);

/*
 * The hashing function performs some computation on a key and produces an
 * integral value of type hash_val_t based on that key. For best results, the
 * function should have a good randomness properties in *all* significant bits
 * over the set of keys that are being inserted into a given hash table. In
 * particular, the most significant bits of hash_val_t are most significant to
 * the hash module. Only as the hash table expands are less significant bits
 * examined. Thus a function that has good distribution in its upper bits but
 * not lower is preferrable to one that has poor distribution in the upper bits
 * but not the lower ones.
 */
#if __WORDSIZE == 32
typedef uint32_t (*hash_fun_t)(const char *, unsigned int, void *);
#elif __WORDSIZE == 64
typedef uint64_t (*hash_fun_t)(const char *, unsigned int, void *);
#endif

/*
 * This is the hash table control structure. It keeps track of information
 * about a hash table, as well as the hash table itself.
 * Notes:
 * 1.  Pointer to the hash table proper. The table is an array of pointers to
 *     hash nodes (of type hnode_t). If the table is empty, every element of
 *     this table is a null pointer. A non-null entry points to the first
 *     element of a chain of nodes.
 * 2.  This member keeps track of the size of the hash table---that is, the
 *     number of chain pointers.
 * 3.  The count member maintains the number of elements that are presently
 *     in the hash table.
 * 4.  The maximum count is the greatest number of nodes that can populate this
 *     table. If the table contains this many nodes, no more can be inserted,
 *     and the hash_isfull() function returns true.
 * 5.  The high mark is a population threshold, measured as a number of nodes,
 *     which, if exceeded, will trigger a table expansion. Only dynamic hash
 *     tables are subject to this expansion.
 * 6.  The low mark is a minimum population threshold, measured as a number of
 *     nodes. If the table population drops below this value, a table shrinkage
 *     will occur. Only dynamic tables are subject to this reduction.  No table
 *     will shrink beneath a certain absolute minimum number of nodes.
 * 7.  This is the a pointer to the hash table's comparison function. The
 *     function is set once at initialization or creation time.
 * 8.  Pointer to the table's hashing function, set once at creation or
 *     initialization time.
 * 9.  The current hash table mask. If the size of the hash table is 2^N,
 *     this value has its low N bits set to 1, and the others clear. It is used
 *     to select bits from the result of the hashing function to compute an
 *     index into the table.
 */
typedef struct hash_t {
  hnode_t **table;		/* 1 */
  hnode_t *initial;
#if __WORDSIZE == 32
  uint32_t nchains;			/* 2 */
  uint32_t nodecount;			/* 3 */
  uint32_t maxcount;			/* 4 */
  uint32_t highmark;			/* 5 */
  uint32_t lowmark;			/* 6 */
  uint32_t mask;			/* 9 */
#elif __WORDSIZE == 64
  uint64_t nchains;   /* 2 */
  uint64_t nodecount;  /* 3 */
  uint64_t maxcount;   /* 4 */
  uint64_t highmark;   /* 5 */
  uint64_t lowmark;    /* 6 */
  uint64_t mask;       /* 9 */
#endif
  hash_comp_t compare;			/* 7 */
  hash_fun_t function;			/* 8 */
  unsigned int dynamic : 1;
} hash_t;

/*
 * Hash node list:
 * Structure for hash node lists.
 */
typedef struct hash_node_list {
  struct hash_node_list *next;
  struct hash_node *node;
} hnlst_t;

/*
 * Hash scanner structure, used for traversals of the data structure.
 * Notes:
 * 1. Pointer to the hash table that is being traversed.
 * 2. Reference to the current chain in the table being traversed (the chain
 *    that contains the next node that shall be retrieved).
 * 3. Pointer to the node that will be retrieved by the subsequent call to
 *    hash_scan_next().
 */
typedef struct hscan_t {
  hash_t *table;	/* 1 */
#if __WORDSIZE == 32
  uint32_t chain;	/* 2 */
#elif __WORDSIZE == 64
  uint64_t chain;       /* 2 */
#endif
    hnode_t *next;	/* 3 */
} hscan_t;

/*
 * hash_function_name:
 * Types of hash functions supported by module HASH of libefa.
 */
#if __WORDSIZE == 32
enum hash_function_name {
  HASH_FUN_KAZLIB,
  HASH_FUN_ADDITIVE,
  HASH_FUN_BERNSTEIN,
#if HAVE_XMMINTRIN_H
  HASH_FUN_CESSU,
#endif
  HASH_FUN_CRC,
  HASH_FUN_GENERAL_CRC,
  HASH_FUN_MURMUR1,
  HASH_FUN_MURMUR1A,
  HASH_FUN_MURMUR2,
  HASH_FUN_MURMUR2A,
  HASH_FUN_MURMURN2,
  HASH_FUN_MURMURA2,
  HASH_FUN_MURMUR3,
  HASH_FUN_ONE_AT_A_TIME,
  HASH_FUN_PAUL_HSIEH,
  HASH_FUN_ROTATINGV1,
  HASH_FUN_ROTATINGV2,
  HASH_FUN_NB
};
#elif __WORDSIZE == 64
enum hash_function_name {
  HASH_FUN_KAZLIB,
  HASH_FUN_ADDITIVE,
  HASH_FUN_BERNSTEIN,
  HASH_FUN_CRC,
  HASH_FUN_GENERAL_CRC,
  HASH_FUN_MURMUR1,
  HASH_FUN_MURMUR1A,
  HASH_FUN_MURMUR2,
  HASH_FUN_ROTATINGV1,
  HASH_FUN_ROTATINGV2,
  HASH_FUN_NB
};
#endif

#if __WORDSIZE == 32
# define HASH_FUN_LEN 13
#elif __WORDSIZE == 64
# define HASH_FUN_LEN 11
#endif

/*
 * hash_function_names:
 * Array with hash function names.
 */
extern const char *const hash_function_names[];

/*
 * hash_functions:
 * Array with pointers to hash functions.
 */
extern const hash_fun_t hash_functions[];

hnode_t *hash_alloc_insert(hash_t *, const char *);
int hash_comp_default(const char *, const char *);
hnode_t *hash_delete(hash_t *, hnode_t *);
void hash_delete_free_sos(hash_t *, hnode_t *, unsigned int, unsigned int);
void hash_delete_free_state(hash_t *, hnode_t *, unsigned int, unsigned int);
void hash_free_sos(hash_t *, unsigned int, unsigned int);
void hash_free_state(hash_t *, unsigned int, unsigned int);
void hash_free_nodes_sos(hash_t *, unsigned int, unsigned int);
void hash_free_nodes_state(hash_t *, unsigned int, unsigned int);
hash_t *hash_init(hash_t *, unsigned int);
void hash_insert(hash_t *, hnode_t *, const char *);
hnode_t *hash_lookup(hash_t *, const char *);
void hash_scan_begin(hscan_t *, hash_t *);
hnode_t *hash_scan_delete(hash_t *, hnode_t *);
void hash_scan_delfree_sos(hash_t *, hnode_t *, unsigned int, unsigned int);
void hash_scan_delfree_state(hash_t *, hnode_t *, unsigned int, unsigned int);
hnode_t *hash_scan_next(hscan_t *);
int hash_set_function(hash_t *, const char *);
int hash_verify(hash_t *);
void hnode_destroy(hnode_t *);
void hnode_rehash(hash_t *, hnode_t *, const char *);

#if __WORDSIZE == 32
uint32_t hash_fun_default(const char *, unsigned int, void *);
#elif __WORDSIZE == 64
uint64_t hash_fun_default(const char *, unsigned int, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HASH_H */
