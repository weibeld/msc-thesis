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
 * $Id: hash.c,v 1.36.2.11 2000/11/13 01:36:45 kaz Exp $
 * $Name: kazlib_1_20 $
 *
 * 2008-04-17 Small modifications to build with stricter warnings
 *            David Lutterkort <dlutter@redhat.com>
 * 
 * This file is originally from kzalib v1.20 used in augeas. This file has been
 * modified to fit the need of the efalibrary.
 */

#include "libefa/error.h"
#include "libefa/hash.h"
#include "libefa/memory.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "hash/additive.h"
#include "hash/bernstein.h"
#include "hash/crc.h"
#include "hash/gencrc.h"
#include "hash/MurmurHash1.h"
#include "hash/MurmurHash2.h"
#include "hash/one-at-a-time.h"
#include "hash/paul_hsieh.h"
#include "hash/rotating.h"

#if (__WORDSIZE == 32) && HAVE_XMMINTRIN_H
#include "hash/cessu.h"
#include "hash/MurmurHash3.h"
#elif __WORDSIZE == 32
#include "hash/MurmurHash3.h"
#endif

#if __WORDSIZE == 32

const char *const hash_function_names[] = {
  "kazlib",
  "additive",
  "bernstein",
#if HAVE_XMMINTRIN_H
  "cessu",
#endif
  "crc",
  "general-crc",
  "murmur1",
  "murmur1a",
  "murmur2",
  "murmur2a",
  "murmurn2",
  "murmura2",
  "murmur3",
  "one-at-a-time",
  "paul-hsieh",
  "rotatingv1",
  "rotatingv2",
  NULL
};

const hash_fun_t const hash_functions[] = {
  hash_fun_default,
  additive,
  bernstein,
#if HAVE_XMMINTRIN_H
  hasshe2,
#endif
  crc,
  gencrc,
  MurmurHash1,
  MurmurHash1Aligned,
  MurmurHash2,
  MurmurHash2A,
  MurmurHashNeutral2,
  MurmurHashAligned2,
  MurmurHash3_x86_32,
  one_at_a_time,
  SuperFastHash,
  rotating_v1,
  rotating_v2,
  NULL
};

#elif __WORDSIZE == 64

const char *const hash_function_names[] = {
  "kazlib",
  "additive",
  "bernstein",
  "crc",
  "general-crc",
  "murmur1",
  "murmur1a",
  "murmur2",
  "rotatingv1",
  "rotatingv2",
  NULL
};

const hash_fun_t const hash_functions[] = {
  hash_fun_default,
  additive,
  bernstein,
  crc,
  gencrc,
  MurmurHash1,
  MurmurHash1Aligned,
  MurmurHash64A,
  rotating_v1,
  rotating_v2,
  NULL
};

#endif

#if __WORDSIZE == 32
static uint32_t init_size;
#elif __WORDSIZE == 64
static uint64_t init_size;
#endif

static int hash_val_t_bit;

static void compute_bits(void);
static void grow_table(hash_t *);
static void hnode_free_sos(hnode_t *, unsigned int, unsigned int);
static void hnode_free_state(hnode_t *, unsigned int, unsigned int);
static void shrink_table(hash_t *);

#if __WORDSIZE == 32
static int is_power_of_two(uint32_t);
#elif __WORDSIZE == 64
static int is_power_of_two(uint64_t);
#endif

/*
 * Compute the number of bits in the hash_val_t type.  We know that hash_val_t
 * is an unsigned integral type. Thus the highest value it can hold is a
 * Mersenne number (power of two, less one). We initialize a hash_val_t
 * object with this value and then shift bits out one by one while counting.
 * Notes:
 * 1. HASH_VAL_T_MAX is a Mersenne number---one that is one less than a power
 *    of two. This means that its binary representation consists of all one
 *    bits, and hence ``val'' is initialized to all one bits.
 * 2. While bits remain in val, we increment the bit count and shift it to the
 *    right, replacing the topmost bit by zero.
 */
static void compute_bits(void) {
#if __WORDSIZE == 32
  uint32_t val = UINT32_MAX;	/* 1 */
#elif __WORDSIZE == 64
  uint64_t val = UINT64_MAX;
#endif
  int bits = 0;

  while (val) {	/* 2 */
    bits++;
    val >>= 1;
  }

  hash_val_t_bit = bits;
}

/*
 * Double the size of a dynamic table. This works as follows. Each chain splits
 * into two adjacent chains.  The shift amount increases by one, exposing an
 * additional bit of each hashed key. For each node in the original chain, the
 * value of this newly exposed bit will decide which of the two new chains will
 * receive the node: if the bit is 1, the chain with the higher index will have
 * the node, otherwise the lower chain will receive the node. In this manner,
 * the hash table will continue to function exactly as before without having to
 * rehash any of the keys.
 * Notes:
 * 1.  Overflow check.
 * 2.  The new number of chains is twice the old number of chains.
 * 3.  The new mask is one bit wider than the previous, revealing a
 *     new bit in all hashed keys.
 * 4.  Allocate a new table of chain pointers that is twice as large as the
 *     previous one.
 * 5.  If the reallocation was successful, we perform the rest of the growth
 *     algorithm, otherwise we do nothing.
 * 6.  The exposed_bit variable holds a mask with which each hashed key can be
 *     AND-ed to test the value of its newly exposed bit.
 * 7.  Now loop over each chain in the table and sort its nodes into two
 *     chains based on the value of each node's newly exposed hash bit.
 * 8.  The low chain replaces the current chain.  The high chain goes
 *     into the corresponding sister chain in the upper half of the table.
 * 9.  We have finished dealing with the chains and nodes. We now update
 *     the various bookeeping fields of the hash structure.
 */
static void grow_table(hash_t *hash) {
#if __WORDSIZE == 32
  uint32_t chain, exposed_bit, mask;
#elif __WORDSIZE == 64
  uint64_t chain, exposed_bit, mask;
#endif
  hnode_t **newtable = NULL, *low_chain, *high_chain, *hptr, *next;

  newtable = realloc(hash->table,
		     sizeof *newtable * hash->nchains * 2);	/* 4 */

  if (newtable) {	/* 5 */
    mask = (hash->mask << 1) | 1;	/* 3 */
    exposed_bit = mask ^ hash->mask;	/* 6 */

    for (chain = 0; chain < hash->nchains; chain++) { /* 7 */
      low_chain = NULL;
      high_chain = NULL;

      for (hptr = newtable[chain]; hptr != 0; hptr = next) {
	next = hptr->next;

	if (hptr->hkey & exposed_bit) {
	  hptr->next = high_chain;
	  high_chain = hptr;
	} else {
	  hptr->next = low_chain;
	  low_chain = hptr;
	}
      }

      newtable[chain] = low_chain; 	/* 8 */
      newtable[chain + hash->nchains] = high_chain;
    }

    hash->table = newtable;			/* 9 */
    hash->mask = mask;
    hash->nchains *= 2;
    hash->lowmark *= 2;
    hash->highmark *= 2;
  }
}

hnode_t *hash_alloc_insert(hash_t *hash, const char *key) {
  int ret = 0;
  hnode_t *node = NULL;

  ret = efa_alloc(&node, sizeof(hnode_t), 1);
  if (ret == -1) {
    fprintf(stderr, " [HASH] Not enough memory to allocate a new hash node \
structure.\n");
    return NULL;
  }

  hash_insert(hash, node, key);

  return node;
}

int hash_comp_default(const char *key1, const char *key2) {
  return strncmp(key1, key2, KEY_MAX_LENGTH);
}

/*
 * Delete the given node from the hash table.  Since the chains
 * are singly linked, we must locate the start of the node's chain
 * and traverse.
 * Notes:
 * 1. The node must belong to this hash table, and its key must not have
 *    been tampered with.
 * 2. If this deletion will take the node count below the low mark, we
 *    shrink the table now.
 * 3. Determine which chain the node belongs to, and fetch the pointer
 *    to the first node in this chain.
 * 4. If the node being deleted is the first node in the chain, then
 *    simply update the chain head pointer.
 * 5. Otherwise advance to the node's predecessor, and splice out
 *    by updating the predecessor's next pointer.
 * 6. Indicate that the node is no longer in a hash table.
 */
hnode_t *hash_delete(hash_t *hash, hnode_t *node) {
#if __WORDSIZE == 32
  uint32_t chain;
#elif __WORDSIZE == 64
  uint64_t chain;
#endif
  hnode_t *hptr;

  if (hash->dynamic && hash->nodecount <= hash->lowmark
      && hash->nodecount > init_size)
    shrink_table(hash);				/* 2 */

  chain = node->hkey & hash->mask;			/* 3 */
  hptr = hash->table[chain];

  if (hptr == node)					/* 4 */
    hash->table[chain] = node->next;
  else {
    while (hptr->next != node)			/* 5 */
      hptr = hptr->next;

    hptr->next = node->next;
  }

  hash->nodecount--;

  node->next = NULL;					/* 6 */
  return node;
}

void hash_delete_free_sos(hash_t *hash, hnode_t *node, unsigned int nfa,
		      unsigned int nb_symbols) {
  hash_delete(hash, node);
  hnode_free_sos(node, nfa, nb_symbols);
}

void hash_delete_free_state(hash_t *hash, hnode_t *node, unsigned int nfa,
		      unsigned int nb_symbols) {
  hash_delete(hash, node);
  hnode_free_state(node, nfa, nb_symbols);
}

/*
 * hash_free_sos:
 * Obsolescent function for removing all nodes from a table,
 * freeing them and then freeing the table all in one step.
 */
void hash_free_sos(hash_t *hash, unsigned int nfa, unsigned int nb_symbols) {
  hash_free_nodes_sos(hash, nfa, nb_symbols);
  efa_free(hash->table);
  efa_free(hash);
}

/*
 * hash_free_state:
 * Obsolescent function for removing all nodes from a table,
 * freeing them and then freeing the table all in one step.
 */
void hash_free_state(hash_t *hash, unsigned int nfa, unsigned int nb_symbols) {
  hash_free_nodes_state(hash, nfa, nb_symbols);
  if (hash->table != NULL)
    efa_free(hash->table);
  
  efa_free(hash);
}

/*
 * hash_free_nodes_sos:
 * Free every node in the hash using the hash->freenode() function pointer, and
 * cause the hash to become empty.
 */
void hash_free_nodes_sos(hash_t *hash, unsigned int nfa,
			 unsigned int nb_symbols) {
#if __WORDSIZE == 32
  uint32_t chain;
#elif __WORDSIZE == 64
  uint64_t chain;
#endif
  hnode_t *node, *next;

  for (chain = 0; chain < hash->nchains; chain ++) {
    node = hash->table[chain];
    while (node != NULL) {
      next = node->next;
      hnode_free_sos(node, nfa, nb_symbols);
      node = next;
    }
    hash->table[chain] = NULL;
  }
  hash->nodecount = 0;
}

/*
 * hash_free_nodes_states:
 * Free every node in the hash using the hash->freenode() function pointer, and
 * cause the hash to become empty.
 */
void hash_free_nodes_state(hash_t *hash, unsigned int nfa,
			    unsigned int nb_symbols) {
#if __WORDSIZE == 32
  uint32_t chain;
#elif __WORDSIZE == 64
  uint64_t chain;
#endif
  hnode_t *node, *next;

  for (chain = 0; chain < hash->nchains; chain++) {
    node = hash->table[chain];
    while (node != NULL) {
      next = node->next;
      hnode_free_state(node, nfa, nb_symbols);
      node = next;
    }
    hash->table[chain] = NULL;
  }
  hash->nodecount = 0;
}

#if __WORDSIZE == 32
uint32_t hash_fun_default(const char *key, unsigned int len,
			  void *foo __attribute__ ((unused)))
#elif __WORDSIZE == 64
uint64_t hash_fun_default(const char *key, unsigned int len,
			  void *foo __attribute__ ((unused)))
#endif
{
  unsigned int i;

#if __WORDSIZE == 32
  static uint32_t randbox[] = {
    0x49848f1bU, 0xe6255dbaU, 0x36da5bdcU, 0x47bf94e9U,
    0x8cbcce22U, 0x559fc06aU, 0xd268f536U, 0xe10af79aU,
    0xc1af4d69U, 0x1d2917b5U, 0xec4c304dU, 0x9ee5016cU,
    0x69232f74U, 0xfead7bb3U, 0xe9089ab6U, 0xf012f6aeU,
  };
  uint32_t acc = 0;
#elif __WORDSIZE == 64
  static uint64_t randbox[] = {
    0x5602350a6111982fU, 0xd9a2b8733a30766aU, 0x76eddb2a7a6ffe01U, 0x7cde92ce52bf8d2aU,
    0x51c73a6453dd7707U, 0xd43b1ab5c7121f2bU, 0xb1e1987783ea0539U, 0xb4efe4f15ee56358U,
    0x9036719382640f62U, 0x6458b277472cfa1fU, 0xb96ad6a87c5ac8fcU, 0xdde669ea72752da9U,
    0x5f6d161729c5225cU, 0x205c75b6bcdc2adaU, 0x3871debb5494a031U, 0x3557dd2c7853dda8U,
  };
  uint64_t acc = 0;
#endif

  const unsigned char *str = (const unsigned char *)key;

  for (i = 0; i < len; i++) {
    acc ^= randbox[(str[i] + acc) & 0xf];
    acc = (acc << 1) | (acc >> 31);
#if __WORDSIZE == 32
    acc &= 0xffffffffU;
#elif __WORDSIZE == 64
    acc &= 0xffffffffffffffffU;
#endif
    acc ^= randbox[((str[i] >> 4) + acc) & 0xf];
    acc = (acc << 2) | (acc >> 30);
#if __WORDSIZE == 32
    acc &= 0xffffffffU;
#elif __WORDSIZE == 64
    acc &= 0xffffffffffffffffU;
#endif
  }
  return acc;
}

/*
 * Initialize a user supplied hash structure. The user also supplies a table of
 * chains which is assigned to the hash structure. The table is static---it
 * will not grow or shrink.
 * 1. See note 1. in hash_create().
 * 2. The user supplied array of pointers hopefully contains nchains nodes.
 * 3. See note 7. in hash_create().
 * 4. We must dynamically compute the mask from the given power of two table
 *    size.
 * 5. The user supplied table can't be assumed to contain null pointers,
 *    so we reset it here.
 */
hash_t *hash_init(hash_t *hash, unsigned int nb_states) {
  int ret = 0;
  
#if __WORDSIZE == 32
  uint32_t pow2 = 0;
#elif __WORDSIZE == 64
  uint64_t pow2 = 0;
#endif

  if (hash == NULL) {
    fprintf(stderr, " [HASH] Hash descriptor is NULL.\n");
    return NULL;
  }
	 
  // bsr (Bit Scan Reverse) to get next higher power of two
  /* 
   * Reference:
   * Intel 64 and IA-32 Architectures
   * Software Developer's Manual
   * Vol. 2A, p. 3-82
   * http://download.intel.com/products/processor/manual/325383.pdf
   */
#if __WORDSIZE == 32
  asm ("bsr %%eax, %%ecx\n\t"     /* searches for most significant set bit in
				     source and stores its index in EAX       */
       "jnz NZERO\n\t"		  /* bsr sets zero-flag if source is zero, which
				     would lead to an undefined destination   */
       "xor %%ecx, %%ecx\n\t"	  /* set result in EAX to zero */
       "jmp CONT\n\t"
       "NZERO: movl $2, %%eax\n\t"/* store 1+1=2 in the register EAX as register
				     index starts with 0!                     */
       "shl %%cl, %%eax\n\t"      /* shift the bit in EAX CL times left to get:
				     2^n * 2 = 2^(n+1)                        */
       "CONT: movl %%eax, %0\n\t" /* move result in EAX to 'pow2' */
       : "=r" ( pow2 )            /* output variable */
       : "a" ( nb_states )	  /* input variable */
       : "%ecx"			  /* clobbered register */ 
  );
#elif __WORDSIZE == 64
  asm ("bsr %%rax, %%rcx\n\t"
       "jnz NZERO\n\t"
       "xor %%rcx, %%rcx\n\t"
       "jmp CONT\n\t"
       "NZERO: movq $2, %%rax\n\t"
       "shl %%cl, %%rax\n\t"
       "CONT: movq %%rax, %0\n\t"
       : "=r" ( pow2 )
       : "a" ( nb_states )
       : "%rcx"
  );
#endif

  if (pow2 == 0) {
    fprintf(stderr, " [HASH] The number of states is zero.\n");
    return NULL;
  }

  if (pow2 < 4)
	 init_size = 4;
  else
	 init_size = pow2;

  if (hash_val_t_bit == 0)	/* 1 */
    compute_bits();

  /* 4 */
  ret = efa_alloc(&hash->table, sizeof(hnode_t), sizeof(*hash->table) * 
init_size);
  if (ret == -1) {
    efa_error(EFA_MOD_HASH, __LINE__, __FILE__);
    free(hash);
    return NULL;
  }

  hash->nchains = init_size;
  hash->highmark = init_size * 2;
  hash->lowmark = init_size / 2;
  hash->nodecount = 0;
#if __WORDSIZE == 32
  hash->maxcount = UINT32_MAX;
#elif __WORDSIZE == 64
  hash->maxcount = UINT64_MAX;
#endif
  hash->compare = hash_comp_default;
  hash->function = hash_fun_default;
  hash->mask = init_size - 1;	/* 4 */
  hash->dynamic = 0;		/* 3 */

  return hash;
}

/*
 * Insert a node into the hash table.
 * Notes:
 * 1. It's illegal to insert more than the maximum number of nodes. The client
 *    should verify that the hash table is not full before attempting an
 *    insertion.
 * 2. The same key may not be inserted into a table twice.
 * 3. If the table is dynamic and the load factor is already at >= 2,
 *    grow the table.
 * 4. We take the bottom N bits of the hash value to derive the chain index,
 *    where N is the base 2 logarithm of the size of the hash table.
 */
void hash_insert(hash_t *hash, hnode_t *node, const char *key) {
#if __WORDSIZE == 32
  uint32_t hkey, chain;
#elif __WORDSIZE == 64
  uint64_t hkey, chain;
#endif

  if (hash->dynamic && hash->nodecount >= hash->highmark)	/* 3 */
    grow_table(hash);

  hkey = hash->function(key, strlen(key), NULL);
  chain = hkey & hash->mask;	/* 4 */

  node->key = key;
  node->hkey = hkey;
  node->next = hash->table[chain];
  hash->table[chain] = node;
  hash->nodecount++;
}

/*
 * Find a node in the hash table and return a pointer to it.
 * Notes:
 * 1. We hash the key and keep the entire hash value. As an optimization, when
 *    we descend down the chain, we can compare hash values first and only if
 *    hash values match do we perform a full key comparison.
 * 2. To locate the chain from among 2^N chains, we look at the lower N bits of
 *    the hash value by anding them with the current mask.
 * 3. Looping through the chain, we compare the stored hash value inside each
 *    node against our computed hash. If they match, then we do a full
 *    comparison between the unhashed keys. If these match, we have located the
 *    entry.
 */
hnode_t *hash_lookup(hash_t *hash, const char *key) {
#if __WORDSIZE == 32
  uint32_t hkey, chain;
#elif __WORDSIZE == 64
  uint64_t hkey, chain;
#endif
  hnode_t *nptr;

  hkey = hash->function(key, strlen(key), NULL);		/* 1 */
  chain = hkey & hash->mask;		/* 2 */

  for (nptr = hash->table[chain]; nptr; nptr = nptr->next) {	/* 3 */
    if (nptr->hkey == hkey && hash->compare(nptr->key, key) == 0)
      return nptr;
  }

  return NULL;
}

/*
 * Reset the hash scanner so that the next element retrieved by
 * hash_scan_next() shall be the first element on the first non-empty chain.
 * Notes:
 * 1. Locate the first non empty chain.
 * 2. If an empty chain is found, remember which one it is and set the next
 *    pointer to refer to its first element.
 * 3. Otherwise if a chain is not found, set the next pointer to NULL
 *    so that hash_scan_next() shall indicate failure.
 */
void hash_scan_begin(hscan_t *scan, hash_t *hash) {
#if __WORDSIZE == 32
  uint32_t nchains;
  uint32_t chain;
#elif __WORDSIZE == 64
  uint64_t nchains;
  uint64_t chain;
#endif

  nchains = hash->nchains;
  scan->table = hash;

  /* 1 */

  for (chain = 0; chain < nchains && hash->table[chain] == 0; chain++)
    ;

  if (chain < nchains) {	/* 2 */
    scan->chain = chain;
    scan->next = hash->table[chain];
  } else			/* 3 */
    scan->next = NULL;
}

/*
 *  Exactly like hash_delete, except does not trigger table shrinkage. This is to be
 *  used from within a hash table scan operation. See notes for hash_delete.
 */
hnode_t *hash_scan_delete(hash_t *hash, hnode_t *node) {
#if __WORDSIZE == 32
  uint32_t chain;
#elif __WORDSIZE == 64
  uint64_t chain;
#endif
  hnode_t *hptr;

  chain = node->hkey & hash->mask;
  hptr = hash->table[chain];

  if (hptr == node)
	hash->table[chain] = node->next;
  else {
    while (hptr->next != node)
      hptr = hptr->next;
    hptr->next = node->next;
  }

  hash->nodecount--;
  node->next = NULL;

  return node;
}

/*
 * Like hash_delete_free but based on hash_scan_delete.
 */
void hash_scan_delfree_sos(hash_t *hash, hnode_t *node, unsigned int nfa,
		       unsigned int nb_symbols) {
  hash_scan_delete(hash, node);
  hnode_free_sos(node, nfa, nb_symbols);
}

/*
 * Like hash_delete_free but based on hash_scan_delete.
 */
void hash_scan_delfree_state(hash_t *hash, hnode_t *node, unsigned int nfa,
		       unsigned int nb_symbols) {
  hash_scan_delete(hash, node);
  hnode_free_state(node, nfa, nb_symbols);
}

/*
 * Retrieve the next node from the hash table, and update the pointer
 * for the next invocation of hash_scan_next().
 * Notes:
 * 1. Remember the next pointer in a temporary value so that it can be
 *    returned.
 * 2. This assertion essentially checks whether the module has been properly
 *    initialized. The first point of interaction with the module should be
 *    either hash_create() or hash_init(), both of which set hash_val_t_bit to
 *    a non zero value.
 * 3. If the next pointer we are returning is not NULL, then the user is
 *    allowed to call hash_scan_next() again. We prepare the new next pointer
 *    for that call right now. That way the user is allowed to delete the node
 *    we are about to return, since we will no longer be needing it to locate
 *    the next node.
 * 4. If there is a next node in the chain (next->next), then that becomes the
 *    new next node, otherwise ...
 * 5. We have exhausted the current chain, and must locate the next subsequent
 *    non-empty chain in the table.
 * 6. If a non-empty chain is found, the first element of that chain becomes
 *    the new next node. Otherwise there is no new next node and we set the
 *    pointer to NULL so that the next time hash_scan_next() is called, a null
 *    pointer shall be immediately returned.
 */
hnode_t *hash_scan_next(hscan_t *scan) {
  hnode_t *next = scan->next;		/* 1 */
  hash_t *hash = scan->table;
#if __WORDSIZE == 32
  uint32_t chain;
  uint32_t nchains;
#elif __WORDSIZE == 64
  uint64_t chain;
  uint64_t nchains;
#endif

  chain = scan->chain + 1;
  nchains = hash->nchains;

  if (next) {			/* 3 */
    if (next->next)	/* 4 */
      scan->next = next->next;
    else {
      while (chain < nchains && hash->table[chain] == 0)	/* 5 */
	chain++;
      if (chain < nchains) {	/* 6 */
	scan->chain = chain;
	scan->next = hash->table[chain];
      } else
	scan->next = NULL;
    }
  }
  return next;
}

/*
 * hash_set_function:
 * Set the function and modify the hash descriptor acordingly.
 */
int hash_set_function(hash_t *hash, const char *function) {
  int i, ret = 0;

  for (i = 0; i < HASH_FUN_NB; i++) {
    ret = strncmp(function, hash_function_names[i], 13);
    if (ret == 0) // hash function names match                                                                                                                                    
      break;
  }
  if (i == HASH_FUN_NB) {
    fprintf(stderr, " [HASH] Value missmatch for '%s'.\n", function);
    return 1;
  }

  switch (i) {
  case HASH_FUN_ADDITIVE:
    // fallthrough
  case HASH_FUN_ROTATINGV1:
    /* 
     * Make the following changes to the hash descriptor:
     * - init_size equals the next higher prime number
     * - high mark and low mark get set accodingly
     * - hash table gets reallocated
     */
    fprintf(stderr, " [HASH] This hash function is not yet fully \
implemented.\n");
    return 1;
    break;
  default:
    break;
  }

  hash->function = hash_functions[i];

  return 0;
}

/*
 * Verify whether the given object is a valid hash table. This means
 * Notes:
 * 1. If the hash table is dynamic, verify whether the high and
 *    low expansion/shrinkage thresholds are powers of two.
 * 2. Count all nodes in the table, and test each hash value
 *    to see whether it is correct for the node's chain.
 */
int hash_verify(hash_t *hash) {
#if __WORDSIZE == 32
  uint32_t count;
  uint32_t chain;
#elif __WORDSIZE == 64
  uint64_t count;
  uint64_t chain;
#endif
  hnode_t *hptr;

  count = 0;

  if (hash->dynamic) {	/* 1 */
    if (hash->lowmark >= hash->highmark) {
      fprintf(stderr, " [HASH] Hash table has higher lowmark than highmark.\n");
      return 1;
    }
    if (!is_power_of_two(hash->highmark)) {
      fprintf(stderr, " [HASH] Hash table's highmark is not a power of 2.\n");
      return 1;
    }
    if (!is_power_of_two(hash->lowmark)) {
      fprintf(stderr, " [HASH] Hash table's lowmark is not a power of 2.\n");
      return 1;
    }
  }

  for (chain = 0; chain < hash->nchains; chain++) {	/* 2 */
    for (hptr = hash->table[chain]; hptr != 0; hptr = hptr->next) {
      if ((hptr->hkey & hash->mask) != chain) {
	fprintf(stderr, " [HASH] Hash value does not match the node's \
chain.\n");
	return 1;
      }
      count++;
    }
  }

  if (count != hash->nodecount) {
    fprintf(stderr, " [HASH] There are more hash nodes than the hash table \
knows off.\n");
	return 1;
  }

  return 0;
}

/*
 * hnode_free_sos:
 * Destroy a dynamically allocated node holding a set of states data structure.
 */
static void hnode_free_sos(hnode_t *node, unsigned int nfa,
			   unsigned int nb_symbols) {
  int i;

  // free non-deterministic transition row
  if (nfa) {
    for (i = 0; i < nb_symbols; i++) {
      if (((sos_t *)node->data)->transition_row != NULL)
	free(((hnode_t ***)((sos_t *)node->data)->transition_row)[i]);
    }
  }
  
  // free transition row
  if (((sos_t *)node->data)->transition_row != NULL) {
    free(((sos_t *)node->data)->transition_row);
    ((sos_t *)node->data)->transition_row = NULL;
  }

  // free the state
  if (node->data != NULL)
    efa_free(node->data);

  // free the hash node
  if (node->key != NULL) {
    free((void *)node->key);
    node->key = NULL;
  }
  
  efa_free(node);
}

/*
 * hnode_free_state:
 * Destroy a dynamically allocated node holding a state data structure.
 */
static void hnode_free_state(hnode_t *node, unsigned int nfa,
		       unsigned int nb_symbols) {
  int i;

  if (node != NULL) {
  
    // free the state
    if (node->data != NULL) {
      // free non-deterministic transition row
      if (nfa) {
	for (i = 0; i < nb_symbols; i++) {
	  if (((state_t *)node->data)->transition_row != NULL) {
	    free(((hnode_t ***)((state_t *)node->data)->transition_row)[i]);
	    ((hnode_t ***)((state_t *)node->data)->transition_row)[i] = NULL;
	  }
	}
      }
  
      // free transition row
      if (((state_t *)node->data)->transition_row != NULL) {
	free(((state_t *)node->data)->transition_row);
	((state_t *)node->data)->transition_row = NULL;
      }
    
      efa_free(node->data);
    }

    // free the hash node
    if (node->key != NULL) {
      free((void *)node->key);
      node->key = NULL;
    }
  
    efa_free(node);
  }
}

/*
 * Rehash the hash node.
 */
void hnode_rehash(hash_t *hash, hnode_t *node, const char *key) {
  hnode_t *rehashed = NULL;

  if (node->key != NULL) {
    free((void *)node->key);
    node->key = NULL;
  }
  rehashed = hash_delete(hash, node);
  hash_insert(hash, rehashed, key);
}

/*
 * Verify whether the given argument is a power of two.
 */
#if __WORDSIZE == 32
static int is_power_of_two(uint32_t arg)
#elif __WORDSIZE == 64
static int is_power_of_two(uint64_t arg)
#endif
{
  if (arg == 0)
    return 0;
  while ((arg & 1) == 0)
    arg >>= 1;
  return (arg == 1);
}

/*
 * Cut a table size in half. This is done by folding together adjacent chains
 * and populating the lower half of the table with these chains. The chains are
 * simply spliced together. Once this is done, the whole table is reallocated
 * to a smaller object.
 * Notes:
 * 1.  It is illegal to have a hash table with one slot. This would mean that
 *     hash->shift is equal to hash_val_t_bit, an illegal shift value.
 *     Also, other things could go wrong, such as hash->lowmark becoming zero.
 * 2.  Looping over each pair of sister chains, the low_chain is set to
 *     point to the head node of the chain in the lower half of the table,
 *     and high_chain points to the head node of the sister in the upper half.
 * 3.  The intent here is to compute a pointer to the last node of the
 *     lower chain into the low_tail variable. If this chain is empty,
 *     low_tail ends up with a null value.
 * 4.  If the lower chain is not empty, we simply tack the upper chain onto it.
 *     If the upper chain is a null pointer, nothing happens.
 * 5.  Otherwise if the lower chain is empty but the upper one is not,
 *     If the low chain is empty, but the high chain is not, then the
 *     high chain is simply transferred to the lower half of the table.
 * 6.  Otherwise if both chains are empty, there is nothing to do.
 * 7.  All the chain pointers are in the lower half of the table now, so
 *     we reallocate it to a smaller object. This, of course, invalidates
 *     all pointer-to-pointers which reference into the table from the
 *     first node of each chain.
 * 8.  Though it's unlikely, the reallocation may fail. In this case we
 *     pretend that the table _was_ reallocated to a smaller object.
 * 9.  Finally, update the various table parameters to reflect the new size.
 */
static void shrink_table(hash_t *hash) {
#if __WORDSIZE == 32
  uint32_t chain, nchains;
#elif __WORDSIZE == 64
  uint64_t chain, nchains;
#endif
  hnode_t **newtable, *low_tail, *low_chain, *high_chain;
  
  nchains = hash->nchains / 2;

  for (chain = 0; chain < nchains; chain++) {
    low_chain = hash->table[chain];		/* 2 */
    high_chain = hash->table[chain + nchains];
    for (low_tail = low_chain; low_tail && low_tail->next; low_tail = low_tail->next)
      ;	/* 3 */
    if (low_chain != 0)				/* 4 */
      low_tail->next = high_chain;
    else if (high_chain != 0)			/* 5 */
      hash->table[chain] = high_chain;
  }
  newtable = realloc(hash->table,
		     sizeof *newtable * nchains);		/* 7 */
  if (newtable)					/* 8 */
    hash->table = newtable;
  hash->mask >>= 1;			/* 9 */
  hash->nchains = nchains;
  hash->lowmark /= 2;
  hash->highmark /= 2;
}
