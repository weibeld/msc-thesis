/*
 * buechi.h: Büchi automata
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

#ifndef BUECHI_H_
#define BUECHI_H_

#include <libefa/efa.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Buechi information:
 * Enumeration of informations that are assigned to sets of states.
 */
enum buechi_info {
  BUECHI_INFO_FORDINARY,
  BUECHI_INFO_ORDINARY,
  BUECHI_INFO_ON_HOLD,
  BUECHI_INFO_DISCONTINUED,
  BUECHI_INFO_NB
};

/*
 * Buechi part:
 * Enumeration describing the parts of a Büchi automaton.
 */
enum buechi_part {
  BUECHI_PART_INITIAL,
  BUECHI_PART_FINITE,
  BUECHI_PART_INFINITE,
  BUECHI_PART_NB
};

/*
 * Tuple:
 * A descriptor for a tuple holding an ordered list of sets of states.
 */
typedef struct tuple {
  const char *label;
  void **set_of_states;
  unsigned int accept : 1;
  unsigned int live : 1;
  unsigned int reachable : 1;
  unsigned int visited : 1;
  unsigned int part : 2;
  unsigned int has_discontinued : 1;
  void **transition_row;
} tuple_t;

/*
 * buechi_build_set_list:
 * Constructs a tuple from a list of sets of states for unifr Büchi 
 * complementation algorithm.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_build_set_list(hnlst_t **, hnlst_t *, hash_t *, unsigned int);
#endif
#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_build_set_list(rbnlst_t **, rbnlst_t *, rbtree_t *,
				 unsigned int);
#endif

/*
 * buechi_create_insert_tuple:
 * Creates a tuple from a list of sets of states and inserts it into the tuple 
 * hash table.
 */
#if HAVE_MODULE_HASH
int buechi_hash_create_insert_tuple(hnlst_t *, hash_t *, hnode_t **);
#endif
#if HAVE_MODULE_RBTREE
int buechi_rbtree_create_insert_tuple(rblst_t *, rbtree_t *, rbnode_t **);
#endif

/*
 * buechi_create_tuple:
 * Creates a tuple from a list of sets of states.
 */
#if HAVE_MODULE_HASH
hnode_t *buechi_hash_create_tuple(hnlst_t *);
#endif
#if HAVE_MODULE_RBTREE
rbnode_t *buechi_rbtree_create_tuple(rbnlst_t *);
#endif

/*
 * buechi_create_tuple_label
 * Creates a label for a tuple from a list of sets of states.
 */
#if HAVE_MODULE_HASH
const char *buechi_hash_create_tuple_label(hnlst_t *);
#endif
#if HAVE_MODULE_RBTREE
const char *buechi_rbtree_create_tuple_label(rbnlst_t *);
#endif

/*
 * buechi_mod_sub_const:
 * Modified subset construction.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_mod_sub_const(hnlst_t *, hash_t *, unsigned int);
#endif
#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_mod_sub_const(rbnlst_t *, rbtree_t *, unsigned int);
#endif

/*
 * buechi_subset_splitting:
 * Splits a mixed set of states into a non-accepting set and an accepting set.
 */
#if HAVE_MODULE_HASH
int buechi_hash_subset_splitting(sos_t *, hnlst_t **, hash_t *, tuple_t *,
				 unsigned int);
#endif
#if HAVE_MODULE_RBTREE
int buechi_rbtree_subset_splitting(sos_t *, rbnlst_t **, rbtree_t *,
				   tuple_t *, unsigned int);
#endif

/*
 * buechi_tuple_const:
 * Constructs tuples similar to subset construction but for Büchi 
 * complementation.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_tuple_const(hnlst_t *, hash_t *, hash_t *, hash_t *,
			    unsigned int);
#endif
#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_tuple_const(rbnlst_t *, rbtree_t *, rbtree_t *,
			      rbtree_t *, unsigned int);
#endif

/*
 * buechi_tuple_const2:
 * Same as buechi_tuple_const with one optimization.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_tuple_const2(hnlst_t *, hash_t *, hash_t *, hash_t *,
			     unsigned int);
#endif
#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_tuple_const2(rbnlst_t *, rbtree_t *, rbtree_t *,
			       rbtree_t *, unsigned int);
#endif

/*
 * buechi_tuple_hash_to_state_hash:
 * Transforms a tuple hash table into a state hash table.
 */
#if HAVE_MODULE_HASH
hash_t *buechi_tuple_hash_to_state_hash(hash_t *, unsigned int);
#endif

/*
 * buechi_unifr:
 * A Büchi complementation algorithm.
 */
#if HAVE_MODULE_FAMOD && HAVE_MODULE_HASH
int buechi_hash_unifr(fa_t *automaton, unsigned char);
#endif
#if HAVE_MODULE_FAMOD && HAVE_MODULE_RBTREE
int buechi_rbtree_unifr(fa_t *automaton, unsigned char);
#endif

/*
 * buechi_unifr2:
 * A Büchi complementation algorithm with one optimization.
 */
#if HAVE_MODULE_FAMOD && HAVE_MODULE_HASH
int buechi_hash_unifr2(fa_t *automaton, unsigned char);
#endif
#if HAVE_MODULE_FAMOD && HAVE_MODULE_RBTREE
int buechi_rbtree_unifr2(fa_t *automaton, unsigned char);
#endif

/*
 * buechi_unifr3:
 * A Büchi complementation algorithm with one optimization.
 */
#if HAVE_MODULE_FAMOD && HAVE_MODULE_HASH
int buechi_hash_unifr3(fa_t *automaton, unsigned char);
#endif
#if HAVE_MODULE_FAMOD && HAVE_MODULE_RBTREE
int buechi_rbtree_unifr3(fa_t *automaton, unsigned char);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BUECHI_H_ */
