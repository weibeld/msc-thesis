/*
 * buechi.c: Büchi automata
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

#include "libefa/buechi.h"
#include "libefa/errno.h"
#include "libefa/error.h"
#include "libefa/list.h"
#include "libefa/memory.h"

#if HAVE_MODULE_FAMOD
# include "libefa/famod.h"
#endif

#if HAVE_MODULE_HASH
# include "libefa/hash.h"
#endif

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>

static int unifr_part = BUECHI_PART_FINITE;
static size_t initial_tuple_len = 0;

static hnode_t *buechi_hash_set_lookup(hash_t *, const char *, unsigned int);
static hnode_t *buechi_hash_tuple_lookup(hash_t *, const char *);

/*
 * buechi_build_set_list:
 * Constructs a list of sets of states that can then be transformed into a 
 * tuple. A set of state is added to the list TLST by removing all states that 
 * are already in the list.
 * Returns 0 on success and 1 on failure.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_build_set_list(hnlst_t **tlst, hnlst_t *slst, hash_t *hash,
				unsigned int nb_symbols) {
  hnlst_t *elt, *state_lst;
  hnode_t *lookup, *node_set, *node_state, **node_states;
  int i, ret;
  size_t len;
  state_t *state;
  
  // initialize variables
  state_lst = NULL;
  
  /* loop through each set of states in SLST
     NOTE: SLST is a reverse list!                                            */
  list_for_each(iter, slst) {
    node_set = iter->node;
    node_states = (hnode_t **)((sos_t *)node_set->data)->states;

    // loop through states and construct a list of states
    for (i = 0; node_states[i] != NULL; i++) {
      node_state = node_states[i];
      state = (state_t *)node_state->data;
      
      // state is already present - remove it
      if (!state->visited) {
	ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
	if (ret == -1)
	  goto alloc_error;
      
	elt->node = node_state;
	list_append(state_lst, elt);
	state->visited = 1;
      }
    }
    
    // handle empty set
    if (state_lst == NULL)
      continue;
    
    // create the resulting set of states
    node_set = fa_hash_create_sos(state_lst);
    if (node_set == NULL)
      return 1;
    
    list_free(state_lst);
    state_lst = NULL;
    
    if (unifr_part == BUECHI_PART_INFINITE) {
      /* By default fa_hash_create_sos returns a BUCHI_INFO_FORDINARY set of
	 states. We therefore have to restore its original Büchi information. */
      switch (iter->node->key[0]) {
	case '{':
	  ((sos_t *)node_set->data)->buechi_info = BUECHI_INFO_ORDINARY;
	  break;
	case '(':
	  len = strlen(node_set->key);
	  ((char *)node_set->key)[0] = '(';
	  ((char *)node_set->key)[len - 1] = ')';
	  ((sos_t *)node_set->data)->buechi_info = BUECHI_INFO_ON_HOLD;
	  break;
	case '[':
	  len = strlen(node_set->key);
	  ((char *)node_set->key)[0] = '[';
	  ((char *)node_set->key)[len - 1] = ']';
	  ((sos_t *)node_set->data)->buechi_info = BUECHI_INFO_DISCONTINUED;
	  break;
	default:
	  return 1;
	  break;
      }
      
      // make sure it is in hash sets
      lookup = buechi_hash_set_lookup(hash, node_set->key, ((sos_t 
*)node_set->data)->buechi_info);
    } else
      lookup = hash_lookup(hash, node_set->key);
    
    // set of states is not yet in hash table - add it
    if (lookup == NULL) {
      hash_insert(hash, node_set, node_set->key);
      
      // compute successor sets of states
      ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
      if (ret == -1)
	goto alloc_error;
      
      elt->node = node_set;
      ret = buechi_hash_mod_sub_const(elt, hash, nb_symbols);
      if (ret == 1)
	return ret;
      
      lookup = node_set;
    
    // set is already in hash table - free allocated memory
    } else {
      efa_free(((sos_t *)node_set->data)->states);
      free(node_set->data);
      free((void *)node_set->key);
      efa_free(node_set);
    }
    
    // prepend the set of states to the list of sets of states
    ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    elt->node = lookup;
    list_cons(*tlst, elt);
  }
  
  return 0;
  
alloc_error:
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif

#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_build_set_list(rbnlst_t **tlst, rbnlst_t *slst,
				 rbtree_t *tree, unsigned int nb_symbols) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

/*
 * buechi_create_insert_tuple:
 * Creates a tuple from a list of sets of states and inserts it into the tuple 
 * hash table. The user has to free the allocated memory.
 * Returns 0 on success and 1 on failure.
 */
#if HAVE_MODULE_HASH
int buechi_hash_create_insert_tuple(hnlst_t *lst, hash_t *hash,
				    hnode_t **tuple) {
  const char *label;
  unsigned int reset_dyn = 0;
  
  if (!hash->dynamic) {
    hash->dynamic = 1;
    reset_dyn = 1;
  }
  
  // make sure the list of states LST is not NULL
  if (lst == NULL) {
    errno = EINVAL;
    efa_errno = EFA_EBUECHI_CREATE_INSERT_TUPLE;
    return 1;
  }
  
  *tuple = NULL;
  
  // assemble the label of the tuple
  label = buechi_hash_create_tuple_label(lst);
  if (label == NULL)
    return 1;
  
  // check if the new tuple is in HASH table
  if (unifr_part == BUECHI_PART_FINITE)
    *tuple = hash_lookup(hash, label);
  else
    *tuple = buechi_hash_tuple_lookup(hash, label);
  
  // new tuple is not in HASH table - add it
  if (*tuple == NULL) {
    // create a set of states from the list of states
    *tuple = buechi_hash_create_tuple(lst);
    if (*tuple == NULL)
      return 1;
    
    // insert the new tuple into the hash table
    hash_insert(hash, *tuple, label);
  } else {
    // new tuple is already in HASH table
    free((void *)label);
  }
  
  if (reset_dyn)
    hash->dynamic = 0;
  
  assert(*tuple != NULL);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int buechi_rbtree_create_insert_tuple(rbnlst_t *lst, rbtree_t *tree,
				      rbnode_t **tuple) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return NULL;
}
#endif

/*
 * buechi_create_tuple:
 * Creates a tuple from a list of sets of states. The user has to free the 
 * allocated memory of the returned tuple pointer.
 * Returns a pointer to a tuple on success or NULL on failure.
 */
#if HAVE_MODULE_HASH
hnode_t *buechi_hash_create_tuple(hnlst_t *lst) {
  const char *label;
  hnode_t *node;
  int accept, ret;
  size_t set_count;
  sos_t *set;
  tuple_t *tuple;
  
  // make sure the list of sets of states LST is not NULL
  if (lst == NULL) {
    errno = EINVAL;
    efa_errno = EFA_EBUECHI_CREATE_TUPLE;
    return NULL;
  }
  
  // assemble the label of the tuple
  label = buechi_hash_create_tuple_label(lst);
  if (label == NULL)
    return NULL;
  
  // allocate memory for the new tuple
  ret = efa_alloc(&node, sizeof(hnode_t), 1);
  if (ret == -1)
    goto alloc_error;
      
  ret = efa_alloc(&tuple, sizeof(tuple_t), 1);
  if (ret == -1)
    goto alloc_error;
    
  set_count = 0;
  list_for_each(iter, lst) {
    set_count++;
  }
    
  ret = efa_alloc(&tuple->set_of_states, sizeof(hnode_t *), set_count + 1);
  if (ret == -1)
    goto alloc_error;
  
  if (unifr_part == BUECHI_PART_FINITE)
    accept = 0;
  else
    accept = 1;
 
  set_count = 0;
  list_for_each(iter, lst) {
    set = (sos_t *)iter->node->data;
    if (unifr_part == BUECHI_PART_INFINITE &&
	set->buechi_info == BUECHI_INFO_DISCONTINUED) {
      accept = 0;
      tuple->has_discontinued = 1;
    }

    ((hnode_t **)tuple->set_of_states)[set_count++] = iter->node;
    tuple->reachable |= set->reachable;
  }
  
  node->data = (void *)tuple;
  node->key = label;
  
  // set properties for the new tuple
  tuple->accept = accept;
  tuple->part = unifr_part;
  tuple->label = label;
  
  assert((node != NULL) && (node->data != NULL));
  assert(((hnode_t **)tuple->set_of_states)[0] != NULL);
  
  return node;
  
alloc_error:
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif

#if HAVE_MODULE_RBTREE
rbnode_t *buechi_rbtree_create_tuple(rbnlst_t *lst) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return NULL;
}
#endif

/*
 * buechi_create_tuple_label:
 * Creates a label for a tuple from a list of sets of states. User must free 
 * the allocated memory of the label.
 * Returns a pointer to a character array on success or NULL on failure.
 */
#if HAVE_MODULE_HASH
const char *buechi_hash_create_tuple_label(hnlst_t *lst) {
  char *buffer;
  int ret;
  unsigned int i, set_count;
  size_t len;
  
  // make sure the list of sets of states LST is not NULL
  if (lst == NULL) {
    errno = EINVAL;
    efa_errno = EFA_EBUECHI_CREATE_TUPLE_LABEL;
    return NULL;
  }
  
  len = 0;
  set_count = 0;
  list_for_each(iter, lst) {
    len += strlen(iter->node->key);
    set_count++;
  }
  
  // assemble the label of the new tuple
  ret = efa_alloc(&buffer, sizeof(char), len + set_count + 2);
  if (ret == -1) {
    efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }
  
  i = 0;
  buffer[i++] = '(';
  list_for_each(iter, lst) {
    len = strlen(iter->node->key);
    strncat(&buffer[i], iter->node->key, len);
    i += len;
      
    if (iter->next != NULL)
      buffer[i++] = ',';
  }
  buffer[i] = ')';
  
  return (const char *)buffer;
}
#endif

#if HAVE_MODULE_RBTREE
const char *buechi_rbtree_create_tuple_label(rbnlst_t *lst) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return NULL;
}
#endif

/*
 * buechi_hash_set_lookup:
 * Used to lookup sets of states since an addition comparison is needed for
 * finite and infinite part.
 */
static hnode_t *buechi_hash_set_lookup(hash_t *hash, const char *key,
				       unsigned int info) {
#if __WORDSIZE == 32
  uint32_t hkey, chain;
#elif __WORDSIZE == 64
  uint64_t hkey, chain;
#endif
  hnode_t *nptr;
  
  hkey = hash->function(key, strlen(key), NULL);
  chain = hkey & hash->mask;
  
  for (nptr = hash->table[chain]; nptr; nptr = nptr->next) {
    if (nptr->hkey == hkey && hash->compare(nptr->key, key) == 0 &&
        ((sos_t *)nptr->data)->buechi_info == info)
      return nptr;
  }
  
  return NULL;
}

/*
 * buechi_hash_tuple_lookup:
 * Used to lookup tuples since an additional comparison is needed for finite 
 * and infinite part.
 */
static hnode_t *buechi_hash_tuple_lookup(hash_t *hash, const char *key) {
#if __WORDSIZE == 32
  uint32_t hkey, chain;
#elif __WORDSIZE == 64
  uint64_t hkey, chain;
#endif
  hnode_t *nptr;

  hkey = hash->function(key, strlen(key), NULL);
  chain = hkey & hash->mask;

  for (nptr = hash->table[chain]; nptr; nptr = nptr->next) {
    if (nptr->hkey == hkey && hash->compare(nptr->key, key) == 0 &&
	((tuple_t *)nptr->data)->part == unifr_part)
      return nptr;
  }

  return NULL;
}

/*
 * buechi_mod_sub_const:
 * Modified subset construction. The modified subset construction is similar to 
 * the subset construction except that it does not compute the transitions for 
 * mixed sets of states. A mixed set of states is a set, that consists of 
 * accepting and non-accepting states.
 * Each set of states in the list of sets of states must have a transition row 
 * pointing to NULL. Transition rows are allocated by this function and the 
 * user should free them.
 * After having called this function, the user should make sure to set the 
 * first argument of this function to NULL as all element will have been 
 * removed.
 * Returns 0 on success and 1 on failure.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_mod_sub_const(hnlst_t *work_list, hash_t *hash,
			      unsigned int nb_symbols) {
  hnlst_t *elt;
  hnode_t *node, *node_set, *node_suc_set;
  int ret;
  unsigned int i, reset_dyn = 0;
  sos_t *new_set, *set;
  
  // make sure arguments are passed correctly
  if (work_list == NULL || hash == NULL || hash->initial == NULL || 
      hash->nodecount == 0) {
    errno = EINVAL;
    efa_errno = EFA_EBUECHI_MOD_SUB_CONST;
    return 1;
  }
  
  // switch on dynamic growth of hash table
  if (!hash->dynamic) {
    hash->dynamic = 1;
    reset_dyn = 1;
  }
  
  /* The work list holds all set of states in the set hash as all set of states
     are unmarked                                                             */
  while(work_list != NULL) {
    elt = work_list; // let ELT point to the first element of WORK_LIST
    list_remove(elt, work_list); /* remove the first element of the list aka 
				    mark it                                  */
   
    node_set = elt->node;
    set = (sos_t *)node_set->data;
    efa_free(elt); // free the hash node list element

    assert(set->transition_row == NULL);
    
    // allocate memory for the transition row of the set of states
    ret = efa_alloc(&set->transition_row, sizeof(hnode_t *), nb_symbols);
    if (ret == -1)
      goto alloc_error;
    
    // loop through the alphabet
    for (i = 0; i < nb_symbols; i++) {
      // do a transition and find the successor set
      ret = fa_hash_nfa_move(hash, node_set, &node_suc_set, i);
      if (ret) {
	efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
	exit(EXIT_FAILURE);
      }
      
      // handle the empty set
      if (node_suc_set == NULL)
	continue;
      
      new_set = (sos_t *)node_suc_set->data;
      
      assert(new_set->label != NULL);
      
      // check if the successor set of stats is already in set hash
      node = hash_lookup(hash, new_set->label);
      
      /* A new set of states has been generated and needs to be added to the 
         set hash                                                             */
      if (node == NULL) {
	node = node_suc_set;
	hash_insert(hash, node, new_set->label);
	
	// add only non-mixed sets to the WORK_LIST as mixed sets will be split
	if (!new_set->mixed) {
	  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
	  if (ret == -1)
	    goto alloc_error;
	  
	  elt->node = node;
	  list_append(work_list, elt);
	}
	
	/* Set of states exists already in the set hash. Free all memory that 
           was allocated in the mean time.                                    */
      } else {
	efa_free(new_set->states);
	free((void *)new_set->label);
	efa_free(new_set);
      }
      
      assert(node != NULL &&
	     node->key != NULL &&
	     node->data != NULL &&
	     ((sos_t *)node->data)->label != NULL &&
	     ((sos_t *)node->data)->states != NULL);
      
      // add the set of states to the transition row
      set->transition_row[i] = (void *)node;
      
      // reset variables
      new_set = NULL;
      node = NULL;
    }
  }
  
  if (reset_dyn)
    hash->dynamic = 0;
  
  return 0;
  
alloc_error:
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif

#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_mod_sub_const(rbnlst_t *work_list, rbtree_t *tree,
				unsigned int nb_symbols) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

/*
 * buechi_subset_splitting:
 * Splits a mixed set of states by separating accepting and non-accepting 
 * states. The function returns a ordered list of two sets of states: first the 
 * non-accepting set then the accepting set.
 * Return 0 on success and 1 on failure.
 * 
 * SET: set of states that needs to be split
 * LST: split set of states list with accepting and non-accepting sets listed 
 *      in reverse order
 * HASH: hash of sets of states
 * TUPLE: from-tuple of complementary Büchi automaton
 * INFO: from-set information / color
 */
#if HAVE_MODULE_HASH
int buechi_hash_subset_splitting(sos_t *set, hnlst_t **lst, hash_t *hash,
				 tuple_t *tuple, unsigned int info) {
  hnlst_t *accepting, *non_accepting, *elt;
  hnode_t *node, *acc_node, *nac_node, *node_state, **states;
  int i, ret;
  size_t len;
  state_t *state;
  
  accepting = NULL;
  non_accepting = NULL;
  acc_node = NULL;
  nac_node = NULL;
  
  states = (hnode_t **)set->states;
  
  /* Split accepting and non-accepting states from an ordered mixed set and 
     store them temporarily in lists of states                                */
  for (i = 0; states[i] != NULL; i++) {
    ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    node_state = elt->node = states[i];
    state = (state_t *)node_state->data;

    if (state->accept)
      list_append(accepting, elt);
    else
      list_append(non_accepting, elt);

    elt = NULL;
  }

  // transform the list of states into an actual set of states (accepting)
  if (accepting) {
    acc_node = fa_hash_create_sos(accepting);
    if (acc_node == NULL)
      return 1;
  }
  
  // transform the list of states into an actual set of states (non_accepting)
  if (non_accepting) {
    nac_node = fa_hash_create_sos(non_accepting);
    if (nac_node == NULL)
      return 1;
  }

  // handle mixed sets with color
  if (unifr_part == BUECHI_PART_INFINITE) {
    switch(info) {
      case BUECHI_INFO_FORDINARY:
	/* FALLTHROUGH */
      case BUECHI_INFO_ORDINARY:
	if (!tuple->has_discontinued) {
	  len = strlen(acc_node->key);
	  ((char *)acc_node->key)[0] = '[';
	  ((char *)acc_node->key)[len - 1] = ']';
	  ((sos_t *)acc_node->data)->buechi_info = BUECHI_INFO_DISCONTINUED;
	} else {
	  len = strlen(acc_node->key);
	  ((char *)acc_node->key)[0] = '(';
	  ((char *)acc_node->key)[len - 1] = ')';
	  ((sos_t *)acc_node->data)->buechi_info = BUECHI_INFO_ON_HOLD;
	}
	len = strlen(nac_node->key);
	((char *)nac_node->key)[0] = '{';
	((char *)nac_node->key)[len - 1] = '}';
	((sos_t *)nac_node->data)->buechi_info = BUECHI_INFO_ORDINARY;
	break;
      case BUECHI_INFO_ON_HOLD:
	if (tuple->has_discontinued) {
	  len = strlen(acc_node->key);
	  ((char *)acc_node->key)[0] = '(';
	  ((char *)acc_node->key)[len - 1] = ')';
	  ((sos_t *)acc_node->data)->buechi_info = BUECHI_INFO_ON_HOLD;
	  len = strlen(nac_node->key);
	  ((char *)nac_node->key)[0] = '(';
	  ((char *)nac_node->key)[len - 1] = ')';
	  ((sos_t *)nac_node->data)->buechi_info = BUECHI_INFO_ON_HOLD;
	  break;
	}
	
	/* FALLTHROUGH */
      case BUECHI_INFO_DISCONTINUED:
	len = strlen(acc_node->key);
	((char *)acc_node->key)[0] = '[';
	((char *)acc_node->key)[len - 1] = ']';
	((sos_t *)acc_node->data)->buechi_info = BUECHI_INFO_DISCONTINUED;
	len = strlen(nac_node->key);
	((char *)nac_node->key)[0] = '[';
	((char *)nac_node->key)[len - 1] = ']';
	((sos_t *)nac_node->data)->buechi_info = BUECHI_INFO_DISCONTINUED;
	break;
      default:
	return 1;
	break;
    }
    
    node = buechi_hash_set_lookup(hash, acc_node->key, ((sos_t 
*)acc_node->data)->buechi_info);
    if (node == NULL)
    hash_insert(hash, acc_node, acc_node->key);
    else {
      free(((sos_t *)acc_node->data)->states);
      free((void *)((sos_t *)acc_node->data)->label);
      efa_free(acc_node);
      acc_node = node;
    }
    
    node = buechi_hash_set_lookup(hash, nac_node->key, ((sos_t 
*)nac_node->data)->buechi_info);
    if (node == NULL)
      hash_insert(hash, nac_node, nac_node->key);
    else {
      free(((sos_t *)nac_node->data)->states);
      free((void *)((sos_t *)nac_node->data)->label);
      efa_free(nac_node);
      nac_node = node;
    }
  } else {
    node = hash_lookup(hash, acc_node->key);
    if (node == NULL)
      hash_insert(hash, acc_node, acc_node->key);
    else {
      free(((sos_t *)acc_node->data)->states);
      free((void *)((sos_t *)acc_node->data)->label);
      efa_free(acc_node);
      acc_node = node;
    }
    
    node = hash_lookup(hash, nac_node->key);
    if (node == NULL)
      hash_insert(hash, nac_node, nac_node->key);
    else {
      free(((sos_t *)nac_node->data)->states);
      free((void *)((sos_t *)nac_node->data)->label);
      efa_free(nac_node);
      nac_node = node;
    }
  }
    
  
  // Store the split sets as a reverse list of set of states
  if (acc_node) {
    ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    elt->node = acc_node;
    list_append(*lst, elt);
  }
  
  if (nac_node) {
    ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
    if (ret == -1)
      goto alloc_error;

    elt->node = nac_node;
    list_append(*lst, elt);
  }
  
  return 0;

alloc_error:
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif

#if HAVE_MODULE_RBTREE
int buechi_rbtree_subset_splitting(sos_t *set, rbnlst_t **lst, rbtree_t *tree) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

/*
 * buechi_tuple_const:
 * Similar to subset construction except that it constructs tuples for the 
 * Büchi complementation algorithm.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_tuple_const(hnlst_t *work_list, hash_t *sthash, hash_t *shash,
			    hash_t *thash, unsigned int nb_symbols) {
  hnlst_t *elt, *suc_tuple_lst, *set_lst;
  hnode_t *new_node, *node, *node_set, *node_suc_set, **node_states;
  int i, j, reset_dyn, ret;
  size_t len;
  sos_t *set, *suc_set;
  state_t *state;
  tuple_t *tuple;
  
  // make sure work list is not NULL
  if (work_list == NULL) {
    errno = EINVAL;
    efa_errno = EFA_EBUECHI_TUPLE_CONST;
    return 1;
  }
  
  if (!sthash->dynamic || !shash->dynamic || !thash->dynamic) {
    sthash->dynamic = 1;
    shash->dynamic = 1;
    thash->dynamic = 1;
    reset_dyn = 1;
  }
  
  /* Splitting main loop:
     The work list holds unmarked tuples of the tuple hash                    */
  while(work_list != NULL) {
    elt = work_list; // let ELT point to the first element of WORK_LIST
    list_remove(elt, work_list); /* remove the first element of the list aka 
				    mark it                                  */
   
    tuple = (tuple_t *)elt->node->data;
    
    assert(tuple->transition_row == NULL ||
	   (unifr_part == BUECHI_PART_INFINITE &&
	    tuple->part != BUECHI_PART_INFINITE)); // should not happen
    
    efa_free(elt); // free the hash node list element
  
    // allocate memory for the transition row
    if (tuple->transition_row == NULL) {
      ret = efa_alloc(&tuple->transition_row, sizeof(hnode_t **), nb_symbols);
      if (ret == -1)
	goto alloc_error;
    }
    
    // loop through the alphabet
    for (i = 0; i < nb_symbols; i++) {
      node = NULL;
      suc_tuple_lst = NULL;
      
      // count the number of set of states in the tuple
      for (j = 0; tuple->set_of_states[j + 1] != NULL; j++);
	
      // loop through the set of states (reverse)
      for (; j >= 0; j--) {
	set_lst = NULL;
	node_set = ((hnode_t **)tuple->set_of_states)[j];
	set = (sos_t *)node_set->data;
	
	ret = fa_hash_nfa_move(sthash, node_set, &node_suc_set, i);
	if (ret == 1)
	  return ret;
	
	// handle the empty set
	if (node_suc_set == NULL)
	  continue;
	
	suc_set = (sos_t *)node_suc_set->data;
	
	/* Check if we construct the infinite part of the complementary Büchi 
	   automaton.                                                         */
	if (unifr_part == BUECHI_PART_INFINITE) {
	  /* fa_nfa_move assigns by default BUECHI_INFO_FORDINARY to colored 
	     sets. Therefore colored sets must be assigned the correct color
	     and label according to the transition rules. This switch statement 
	     has to deal with the transition rules.                           */
	  switch (set->buechi_info) {
	    case BUECHI_INFO_FORDINARY:
	      if (set->accept || suc_set->mixed) {
		len = strlen(suc_set->label);
		((char *)suc_set->label)[0] = '[';
		((char *)suc_set->label)[len - 1] = ']';
		suc_set->buechi_info = BUECHI_INFO_DISCONTINUED;
		break;
	      }
	      
	      /* FALLTHROUGH */
	    case BUECHI_INFO_ORDINARY:
	      if (suc_set->accept || suc_set->mixed) {
		len = strlen(suc_set->label);
		if (tuple->has_discontinued) {
		  ((char *)suc_set->label)[0] = '(';
		  ((char *)suc_set->label)[len - 1] = ')';
		  suc_set->buechi_info = BUECHI_INFO_ON_HOLD;
		} else {
		  ((char *)suc_set->label)[0] = '[';
		  ((char *)suc_set->label)[len - 1] = ']';
		  suc_set->buechi_info = BUECHI_INFO_DISCONTINUED;
		}
	      } else
		suc_set->buechi_info = BUECHI_INFO_ORDINARY;
	      
	      break;
	    case BUECHI_INFO_ON_HOLD:
	      len = strlen(suc_set->label);
	      if (tuple->has_discontinued) {
		((char *)suc_set->label)[0] = '(';
		((char *)suc_set->label)[len - 1] = ')';
		suc_set->buechi_info = BUECHI_INFO_ON_HOLD;
		break;
	      }
	      
	      /* FALLTHROUGH */
	    case BUECHI_INFO_DISCONTINUED:
	      len = strlen(suc_set->label);
	      ((char *)suc_set->label)[0] = '[';
	      ((char *)suc_set->label)[len - 1] = ']';
	      suc_set->buechi_info = BUECHI_INFO_DISCONTINUED;
	      break;
	    default:
	      errno = EDOM;
	      efa_errno = EFA_EBUECHI_TUPLE_CONST;
	      return 1;
	      break;
	  }
	  
	  // make sure SUC_SET is in set hash
	  node = buechi_hash_set_lookup(shash, suc_set->label, 
					suc_set->buechi_info);
	} else
	  node = hash_lookup(shash, suc_set->label);
	
	// set of states is not yet in set hash - add it
	if (node == NULL) {
	  hash_insert(shash, node_suc_set, suc_set->label);
	  node = node_suc_set;
	  
	  // compute successor sets
	  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
	  if (ret == -1)
	    goto alloc_error;
	  
	  elt->node = node;
	  
	  ret = buechi_hash_mod_sub_const(elt, shash, nb_symbols);
	  if (ret == 1)
	    return 1;
	  
	  // set of states is in set hash - free SUC_SET
	} else {
	  efa_free(suc_set->states);
	  free((void *)suc_set->label);
	  efa_free(suc_set);
	  efa_free(node_suc_set);
	}
	
	suc_set = (sos_t *)node->data;
	
	// split mixed set of states
	if (suc_set->mixed) {
	  ret = buechi_hash_subset_splitting(suc_set, &set_lst, shash, 
					     tuple, set->buechi_info);
	  if (ret == 1)
	    return ret;
	  
	} else {
	  ret = efa_alloc(&set_lst, sizeof(hnlst_t), 1);
	  if (ret == -1)
	    goto alloc_error;
	  
	  set_lst->node = node;
	}
	
	ret = buechi_hash_build_set_list(&suc_tuple_lst, set_lst, shash, 
					 nb_symbols);
	if (ret == 1)
	  return ret;
	
	list_free(set_lst);
      }
      
      // handle the empty set
      if (suc_tuple_lst == NULL)
	continue;
      
      /* Reset all visited flags of the states and optimize if two 
	 non-accepting states switch between each other                       */
      list_for_each(iter, suc_tuple_lst) {
	node = iter->node;
	set = (sos_t *)node->data;
	node_states = (hnode_t **)set->states;
	for (j = 0; node_states[j] != NULL; j++) {
	  node = node_states[j];
	  state = (state_t *)node->data;
	  state->visited = 0;
	}
      }
      
      // create the tuple
      node = buechi_hash_create_tuple(suc_tuple_lst);
      if (node == NULL)
	return 1;
      
      list_free(suc_tuple_lst);
      suc_tuple_lst = NULL;
      
      if (unifr_part == BUECHI_PART_INFINITE)
	new_node = buechi_hash_tuple_lookup(thash, node->key);
      else
	new_node = hash_lookup(thash, node->key);
      
      /* New tuple is not yet in tuple hash table - add it to the table and the 
	 work list.                                                           */
      if (new_node == NULL) {
	// copied initial tuple needs special treatment
	ret = strncmp(node->key, thash->initial->key, initial_tuple_len);
	if (ret == 0)
	  ((tuple_t *)node->data)->visited = 1;
	
	ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
	if (ret == -1)
	  goto alloc_error;
	
	hash_insert(thash, node, node->key);
	
	elt->node = node;
	list_append(work_list, elt);
	
	// new tuples is already in tuple hash table - free allocated memory
      } else {
	free(((tuple_t *)node->data)->set_of_states);
	free((void *)((tuple_t *)node->data)->label);
	efa_free(node->data);
	efa_free(node);
	node = new_node;
      }

      // update the transition row of TUPLE
      if (((hnode_t ***)tuple->transition_row)[i] == NULL) {
	ret = efa_alloc(&((hnode_t ***)tuple->transition_row)[i], 
			sizeof(hnode_t *), 2);
	if (ret == -1)
	  goto alloc_error;
      }
      
      for (j = 0; ((hnode_t ***)tuple->transition_row)[i][j] != NULL; j++);
      ((hnode_t ***)tuple->transition_row)[i][j] = node;
    }
  }
  
  if (reset_dyn) {
    sthash->dynamic = 0;
    shash->dynamic = 0;
    thash->dynamic = 0;
  }
  
  return 0;
  
alloc_error:
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif

#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_tuple_const(rbnlst_t *work_list, rbtree_t *sttree,
			      rbtree_t *stree, rbtree_t *ttree,
			      unsigned int nb_symbols) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

/*
 * buechi_tuple_const2:
 * Same as buechi_tuple_const with one optimization.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_tuple_const2(hnlst_t *work_list, hash_t *sthash, hash_t *shash,
			     hash_t *thash, unsigned int nb_symbols) {
  hnlst_t *elt, *suc_tuple_lst, *set_lst;
  hnode_t *new_node, *node, *node_set, *node_suc_set, **node_states;
  int i, j, nb_sets, reset_dyn, ret;
  size_t len;
  sos_t *set, *suc_set;
  state_t *state;
  tuple_t *tuple;
  
  // make sure work list is not NULL
  if (work_list == NULL) {
    errno = EINVAL;
    efa_errno = EFA_EBUECHI_TUPLE_CONST;
    return 1;
  }
  
  if (!sthash->dynamic || !shash->dynamic || !thash->dynamic) {
    sthash->dynamic = 1;
    shash->dynamic = 1;
    thash->dynamic = 1;
    reset_dyn = 1;
  }
  
  /* Splitting main loop:
     The work list holds unmarked tuples of the tuple hash                    */
  while(work_list != NULL) {
    elt = work_list; // let ELT point to the first element of WORK_LIST
    list_remove(elt, work_list); /* remove the first element of the list aka 
				    mark it                                  */
   
    tuple = (tuple_t *)elt->node->data;
    
    assert(tuple->transition_row == NULL ||
	   (unifr_part == BUECHI_PART_INFINITE &&
	    tuple->part != BUECHI_PART_INFINITE)); // should not happen
    
    efa_free(elt); // free the hash node list element
  
    // allocate memory for the transition row
    if (tuple->transition_row == NULL) {
      ret = efa_alloc(&tuple->transition_row, sizeof(hnode_t **), nb_symbols);
      if (ret == -1)
	goto alloc_error;
    }
    
    // loop through the alphabet
    for (i = 0; i < nb_symbols; i++) {
      node = NULL;
      suc_tuple_lst = NULL;
      
      // count the number of set of states in the tuple
      for (j = 0; tuple->set_of_states[j + 1] != NULL; j++);
      nb_sets = j;
	
      // loop through the set of states (reverse)
      for (; j >= 0; j--) {
	set_lst = NULL;
	node_set = ((hnode_t **)tuple->set_of_states)[j];
	set = (sos_t *)node_set->data;
	
	ret = fa_hash_nfa_move(sthash, node_set, &node_suc_set, i);
	if (ret == 1)
	  return ret;
	
	// handle the empty set
	if (node_suc_set == NULL)
	  continue;
	
	suc_set = (sos_t *)node_suc_set->data;
	
	/* Check if we construct the infinite part of the complementary Büchi 
	   automaton.                                                         */
	if (unifr_part == BUECHI_PART_INFINITE) {
	  /* fa_nfa_move assigns by default BUECHI_INFO_FORDINARY to colored 
	     sets. Therefore colored sets must be assigned the correct color
	     and label according to the transition rules. This switch statement 
	     has to deal with the transition rules.                           */
	  switch (set->buechi_info) {
	    case BUECHI_INFO_FORDINARY:
	      if (set->accept || suc_set->mixed) {
		len = strlen(suc_set->label);
		((char *)suc_set->label)[0] = '[';
		((char *)suc_set->label)[len - 1] = ']';
		suc_set->buechi_info = BUECHI_INFO_DISCONTINUED;
		break;
	      }
	      
	      /* FALLTHROUGH */
	    case BUECHI_INFO_ORDINARY:
	      if (suc_set->accept || suc_set->mixed) {
		len = strlen(suc_set->label);
		if (tuple->has_discontinued) {
		  ((char *)suc_set->label)[0] = '(';
		  ((char *)suc_set->label)[len - 1] = ')';
		  suc_set->buechi_info = BUECHI_INFO_ON_HOLD;
		} else {
		  ((char *)suc_set->label)[0] = '[';
		  ((char *)suc_set->label)[len - 1] = ']';
		  suc_set->buechi_info = BUECHI_INFO_DISCONTINUED;
		}
	      } else
		suc_set->buechi_info = BUECHI_INFO_ORDINARY;
	      
	      break;
	    case BUECHI_INFO_ON_HOLD:
	      len = strlen(suc_set->label);
	      if (tuple->has_discontinued) {
		((char *)suc_set->label)[0] = '(';
		((char *)suc_set->label)[len - 1] = ')';
		suc_set->buechi_info = BUECHI_INFO_ON_HOLD;
		break;
	      }
	      
	      /* FALLTHROUGH */
	    case BUECHI_INFO_DISCONTINUED:
	      len = strlen(suc_set->label);
	      ((char *)suc_set->label)[0] = '[';
	      ((char *)suc_set->label)[len - 1] = ']';
	      suc_set->buechi_info = BUECHI_INFO_DISCONTINUED;
	      break;
	    default:
	      errno = EDOM;
	      efa_errno = EFA_EBUECHI_TUPLE_CONST;
	      return 1;
	      break;
	  }
	  
	  // make sure SUC_SET is in set hash
	  node = buechi_hash_set_lookup(shash, suc_set->label, 
					suc_set->buechi_info);
	} else
	  node = hash_lookup(shash, suc_set->label);
	
	// set of states is not yet in set hash - add it
	if (node == NULL) {
	  hash_insert(shash, node_suc_set, suc_set->label);
	  node = node_suc_set;
	  
	  // compute successor sets
	  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
	  if (ret == -1)
	    goto alloc_error;
	  
	  elt->node = node;
	  
	  ret = buechi_hash_mod_sub_const(elt, shash, nb_symbols);
	  if (ret == 1)
	    return 1;
	  
	  // set of states is in set hash - free SUC_SET
	} else {
	  efa_free(suc_set->states);
	  free((void *)suc_set->label);
	  efa_free(suc_set);
	  efa_free(node_suc_set);
	}
	
	suc_set = (sos_t *)node->data;
	
	// split mixed set of states
	if (suc_set->mixed) {
	  ret = buechi_hash_subset_splitting(suc_set, &set_lst, shash, 
					     tuple, set->buechi_info);
	  if (ret == 1)
	    return ret;
	  
	} else {
	  ret = efa_alloc(&set_lst, sizeof(hnlst_t), 1);
	  if (ret == -1)
	    goto alloc_error;
	  
	  set_lst->node = node;
	}
	
	/* Optimization: whenever the last set of states (e.g. the first set 
           of states in the reversed set of states list SET_LST) in the tuple 
           is a discontinued set of states we ignore it.                      */
	if (j == nb_sets &&
	    ((sos_t *)((hnode_t *)set_lst->node)->data)->buechi_info == 
BUECHI_INFO_DISCONTINUED) {
	  // 1) free the allocated memory of SET_LST (do not delete the sets!)
	  list_free(set_lst);
	  set_lst = NULL;
	  // 2) be sure SUC_TUPLE_LST is NULL
	  assert(suc_tuple_lst == NULL);
	  // 3) break the loop
	  break;
	}
	
	ret = buechi_hash_build_set_list(&suc_tuple_lst, set_lst, shash, 
					 nb_symbols);
	if (ret == 1)
	  return ret;
	
	list_free(set_lst);
      }
      
      // handle the empty set
      if (suc_tuple_lst == NULL)
	continue;
      
      /* Reset all visited flags of the states and optimize if two 
	 non-accepting states switch between each other                       */
      list_for_each(iter, suc_tuple_lst) {
	node = iter->node;
	set = (sos_t *)node->data;
	node_states = (hnode_t **)set->states;
	for (j = 0; node_states[j] != NULL; j++) {
	  node = node_states[j];
	  state = (state_t *)node->data;
	  state->visited = 0;
	}
      }
      
      // create the tuple
      node = buechi_hash_create_tuple(suc_tuple_lst);
      if (node == NULL)
	return 1;
      
      list_free(suc_tuple_lst);
      suc_tuple_lst = NULL;
      
      if (unifr_part == BUECHI_PART_INFINITE)
	new_node = buechi_hash_tuple_lookup(thash, node->key);
      else
	new_node = hash_lookup(thash, node->key);
      
      /* New tuple is not yet in tuple hash table - add it to the table and the 
	 work list.                                                           */
      if (new_node == NULL) {
	// copied initial tuple needs special treatment
	ret = strncmp(node->key, thash->initial->key, initial_tuple_len);
	if (ret == 0)
	  ((tuple_t *)node->data)->visited = 1;
	
	ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
	if (ret == -1)
	  goto alloc_error;
	
	hash_insert(thash, node, node->key);
	
	elt->node = node;
	list_append(work_list, elt);
	
	// new tuples is already in tuple hash table - free allocated memory
      } else {
	free(((tuple_t *)node->data)->set_of_states);
	free((void *)((tuple_t *)node->data)->label);
	efa_free(node->data);
	efa_free(node);
	node = new_node;
      }

      // update the transition row of TUPLE
      if (((hnode_t ***)tuple->transition_row)[i] == NULL) {
	ret = efa_alloc(&((hnode_t ***)tuple->transition_row)[i], 
			sizeof(hnode_t *), 2);
	if (ret == -1)
	  goto alloc_error;
      }
      
      for (j = 0; ((hnode_t ***)tuple->transition_row)[i][j] != NULL; j++);
      ((hnode_t ***)tuple->transition_row)[i][j] = node;
    }
  }
  
  if (reset_dyn) {
    sthash->dynamic = 0;
    shash->dynamic = 0;
    thash->dynamic = 0;
  }
  
  return 0;
  
alloc_error:
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif

#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_tuple_const2(rbnlst_t *work_list, rbtree_t *sttree,
			      rbtree_t *stree, rbtree_t *ttree,
			      unsigned int nb_symbols) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

/*
 * buechi_tuple_hash_to_state_hash:
 * Transforms a tuple hash table into a state hash table
 */
#if HAVE_MODULE_HASH
hash_t *buechi_tuple_hash_to_state_hash(hash_t *hash, unsigned int nb_symbols) {
  char *buffer;
  hash_t *hash_states;
  hnode_t *from_node, *node, *new_node, ****state_tr, *to_node, ***tuple_tr;
  hscan_t *scanner;
  int i, j, ret;
  size_t len;
  state_t *state;
  tuple_t *tuple;
  
  // create state hash table for finite tuple automaton
  ret = efa_alloc(&hash_states, sizeof(hash_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  hash_states = hash_init(hash_states, 1);
  hash_states->dynamic = 1;
  
  // scan the tuple hash
  ret = efa_alloc(&scanner, sizeof(hscan_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  hash_scan_begin(scanner, hash);
  node = hash_scan_next(scanner);
  
  while (node) {
    tuple = (tuple_t *)node->data;
    
    assert(tuple != NULL);
    
    ret = efa_alloc(&new_node, sizeof(hnode_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    ret = efa_alloc(&state, sizeof(state_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    new_node->data = (void *)state;
    
    // copy all relevant fields
    if (tuple->part == BUECHI_PART_INFINITE) {
      len = strlen(tuple->label);
      ret = efa_alloc(&buffer, sizeof(char), len + 1);
      if (ret == -1)
	goto alloc_error;
      
      memcpy((void *)buffer, (const void *)tuple->label, len);
      
      buffer[0] = '[';
      buffer[len - 1] = ']';
      state->label = buffer;
    } else
      state->label = tuple->label;
    
    tuple->label = NULL;
    
    free((hnode_t **)tuple->set_of_states);
    tuple->set_of_states = NULL;
    
    state->accept = tuple->accept;
    state->reachable = tuple->reachable;
   
    // insert the state into the state hash table
    hash_insert(hash_states, new_node, state->label);
    
    assert(new_node->key != NULL);
    
    if (node == hash->initial)
      hash_states->initial = new_node;
    
    node = hash_scan_next(scanner);
  }
  
  efa_free(scanner);
  
  // scan the tuple hash again for the transition row
  ret = efa_alloc(&scanner, sizeof(hscan_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  hash_scan_begin(scanner, hash);
  node = hash_scan_next(scanner);
  
  while (node) {
    tuple = (tuple_t *)node->data;
    
    assert(tuple != NULL);
    
    tuple_tr = (hnode_t ***)tuple->transition_row;
    
    assert(tuple_tr != NULL);
    
    if (tuple->part == BUECHI_PART_INFINITE) {
      len = strlen(node->key);
      ret = efa_alloc(&buffer, sizeof(char), len + 1);
      if (ret == -1)
	goto alloc_error;
      
      memcpy((void *)buffer, (const void *)node->key, len);
      
      buffer[0] = '[';
      buffer[len - 1] = ']';
      from_node = hash_lookup(hash_states, (const char *)buffer);
    } else
      from_node = hash_lookup(hash_states, node->key);
    
    assert(from_node != NULL); // should not happen
    
    state = (state_t *)from_node->data;
    
    assert(state != NULL);
    assert(state->transition_row == NULL);
    
    state_tr = (hnode_t ****)&state->transition_row;
    
    for (i = 0; i < nb_symbols; i++) {
      if (tuple_tr[i] != NULL) {
	if (*state_tr == NULL) {
	  ret = efa_alloc(state_tr, sizeof(hnode_t **), nb_symbols);
	  if (ret == -1)
	    goto alloc_error;
	}
	
	ret = efa_alloc(&((*state_tr)[i]), sizeof(hnode_t *), 3);
	if (ret == -1)
	  goto alloc_error;
      
	j = 0;
	new_node = tuple_tr[i][j];
	while (new_node) {
	  assert(new_node->key != NULL &&
		 new_node->data != NULL);
	  if (((tuple_t *)new_node->data)->part == BUECHI_PART_INFINITE) {
	    len = strlen(new_node->key);
	    ret = efa_alloc(&buffer, sizeof(char), len + 1);
	    if (ret == -1)
	      goto alloc_error;
      
	    memcpy((void *)buffer, (const void *)new_node->key, len);
      
	    buffer[0] = '[';
	    buffer[len - 1] = ']';
	    to_node = hash_lookup(hash_states, (const char *)buffer);
	  } else
	    to_node = hash_lookup(hash_states, new_node->key);
	
	  assert(new_node != NULL); // should not happen
	
	  (*state_tr)[i][j] = to_node;
	  new_node = tuple_tr[i][++j];
	}
      
	efa_free(tuple_tr[i]);
      }
    }
    
    efa_free(tuple_tr);
    
    node = hash_scan_next(scanner);
  }
  
  efa_free(scanner);
  
  ret = efa_alloc(&scanner, sizeof(hscan_t), 1);
  if (ret == -1)
    goto alloc_error;
 
  hash_scan_begin(scanner, hash);
  node = hash_scan_next(scanner);
  
  while (node) {
    tuple = (tuple_t *)node->data;
    
    efa_free(tuple);
    efa_free(node);
    
    node = hash_scan_next(scanner);
  }
  
  efa_free(scanner);
  
  hash_states->dynamic = 0;
  
  return hash_states;
  
alloc_error:
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif

/*
 * buechi_unifr:
 * This is a determinization-based Büchi complementation algorithm. It produces 
 * a complementary Büchi automaton from a non-deterministc Büchi automaton.
 * Returns 0 on success and 1 on failure.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_unifr(fa_t *fa, unsigned char verbose) {
  hash_t *hash_states, *hash_sets, *hash_tuples;
  hnode_t *node_state, *node_set, *node_tuple, *node;
  hnlst_t *work_list, *elt;
  hscan_t *scanner;
  int ret = 0;
  struct timespec *runtime = NULL;
  tuple_t *tuple;
  
#if __WORDSIZE == 32
  uint32_t nb_states, nb_sets, nb_tuples;
#elif __WORDSIZE == 64
  uint64_t nb_states, nb_sets, nb_tuples;
#endif

  /* Test if the finite automaton is a Büchi automaton and if it is 
     deterministic                                                            */
  if (fa->type != FA_TYPE_BUECHI) {
    errno = EINVAL;
    efa_errno = EFA_EBUECHI_UNIFR;
    efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
    return 1;
  }

  if (fa->class == FA_CLASS_DETERMINISTIC)
    return 0;
  
  if (verbose & VLVL_TIME) {
    ret = efa_start_timer();
    if (ret == -1)
      return 1;
  }
  
  /*******************************************************
   * Initialization:                                     *
   * - Allocate memory for the set hash and tuple hash   *
   * - Initialise set hash and tuple hash                *
   *******************************************************/
  hash_states = (hash_t *)fa->data_structure;

  ret = efa_alloc(&hash_sets, sizeof(hash_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  ret = efa_alloc(&hash_tuples, sizeof(hash_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  hash_sets = hash_init(hash_sets, 1);
  hash_tuples = hash_init(hash_tuples, 1);
  
  hash_sets->function = hash_states->function;
  hash_tuples->function = hash_states->function;
  
  unifr_part = BUECHI_PART_FINITE;
  
  /****************************************************************************
   * Tuple construction without coloring                                      *
   * - Apply modified subset construction to each set of the tuple            *
   * - Construct finite part of complementary Büchi automaton                 *
   ****************************************************************************/
  
  // initialization
  elt = NULL;
  node = NULL;
  work_list = NULL;
  
  node_state = hash_states->initial;
  
  // allocate memory for hash node list element
  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
  if (ret == -1)
    goto alloc_error;
    
  elt->node = node_state;
    
  // create a set of states from the state list and add it to set hash table
  ret = fa_hash_create_insert_sos(elt, hash_sets, &node_set);
  if (ret == 1)
    return ret;
  
  hash_sets->initial = node_set;
  
  // compute successor sets
  elt->node = node_set;
  list_append(work_list, elt);
  ret = buechi_hash_mod_sub_const(work_list, hash_sets, fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  work_list = NULL;
  
  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  elt->node = node_set;
  
  // create a tuple from the set of states list and add it to tuple hash table
  ret = buechi_hash_create_insert_tuple(elt, hash_tuples, &node_tuple);
  if (ret == 1)
    return ret;
  
  hash_tuples->initial = node_tuple;
  initial_tuple_len = strlen(hash_tuples->initial->key);
  ((tuple_t *)node_tuple->data)->part = BUECHI_PART_INITIAL;
  elt->node = node_tuple;
     
  list_append(work_list, elt);
  
  /* The state has been transformed into a set of states and then into a tuple.
     We can now begin to construct the finite part of the complementary Büchi 
     automaton.                                                               */
  ret = buechi_hash_tuple_const(work_list, hash_states, hash_sets, hash_tuples,
				fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  /****************************************************************************
   * Tuple construction with coloring:                                        *
   * - Apply modified subset construction to each set of the tuple            *
   * - Construct ininite part of complementary Büchi automaton                *
   ****************************************************************************/
  
  // initialization
  elt = NULL;
  node_set = NULL;
  work_list = NULL;
  
  unifr_part = BUECHI_PART_INFINITE;
  
  // add the initial tuple to the work list again for infinite part construction
  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
  if (ret == -1)
    goto alloc_error;
 
  elt->node = hash_tuples->initial;
  list_append(work_list, elt);
  
  // compute the infinite part of the complementary Büchi automaton
  ret = buechi_hash_tuple_const(work_list, hash_states, hash_sets, hash_tuples, 
				fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  /*****************************************************************************
   * Connect finite and infinite parts:                                        *
   * - Apply tuple construction with coloring to each tuple in the finite part *
   * - Construct complementary Büchi automaton                                 *
   ****************************************************************************/
  
  // initialization
  elt = NULL;
  node = NULL;
  tuple = NULL;
  work_list = NULL;
  
  // allocate memory for a hash table scanner
  ret = efa_alloc(&scanner, sizeof(hscan_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  hash_scan_begin(scanner, hash_tuples);
  node = hash_scan_next(scanner);
  
  while (node) {
    elt = NULL;
    tuple = (tuple_t *)node->data;
    
    /* To connect the finite and infinite part we only need the tuples in 
       the finite part.                                                       */
    if (tuple->part != BUECHI_PART_FINITE) {
      node = hash_scan_next(scanner);
      continue;
    }
    
    // allocate memory for a tuple list
    ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    // append the tuple element to the work list
    elt->node = node;
    list_append(work_list, elt);
    
    node = hash_scan_next(scanner);
  }
  
  efa_free(scanner);
  
  // connect the finite and infinite part
  ret = buechi_hash_tuple_const(work_list, hash_states, hash_sets, 
				hash_tuples, fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  
  /* Transform tuples into states such that they can be written to an output 
     file                                                                     */
  nb_states = hash_states->nodecount;
  nb_sets = hash_sets->nodecount;
  nb_tuples = hash_tuples->nodecount;
  hash_free_state(hash_states, 1, fa->nb_symbols);
  hash_free_sos(hash_sets, 0, fa->nb_symbols);
  
  hash_states = buechi_tuple_hash_to_state_hash(hash_tuples, fa->nb_symbols);
  if (hash_states == NULL)
    return 1;
  
  fa->data_structure = (void *)hash_states;
  
  if (verbose & VLVL_TIME) {
    runtime = efa_end_timer();
    if (runtime == NULL)
      return 1;
    
    printf(" [BUECHI] Complementing Büchi automaton '%s': %ld s %ld ns\n", 
	   fa->name, (signed long int) runtime->tv_sec,
	   (signed long int) runtime->tv_nsec);
    efa_free(runtime);
  }
  
#if __WORDSIZE == 32
  printf(" [BUECHI] Number of states: %"PRIu32"\n \
[BUECHI] Number of sets and mixed sets: %"PRIu32"\n \
[BUECHI] Number of tuples: %"PRIu32"\n", nb_states, nb_sets, nb_tuples);
#elif __WORDSIZE == 64
  printf(" [BUECHI] Number of states: %"PRIu64"\n \
[BUECHI] Number of sets and mixed sets: %"PRIu64"\n \
[BUECHI] Number of tuples: %"PRIu64"\n", nb_states, nb_sets, nb_tuples);
#endif
  
  return 0;

alloc_error:
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif /* HAVE_MODULE_HASH && HAVE_MODULE_FAMOD */

#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_unifr(fa_t *automaton) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif /* HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD */

/*
 * buechi_unifr2:
 * This is a determinization-based Büchi complementation algorithm. It produces 
 * a complementary Büchi automaton from a complete non-deterministc Büchi 
 * automaton using one optimization over buechi_unifr.
 * Returns 0 on success and 1 on failure.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_unifr2(fa_t *fa, unsigned char verbose) {
  hash_t *hash_states, *hash_sets, *hash_tuples;
  hnode_t *node_state, *node_set, *node_tuple, *node;
  hnlst_t *work_list, *elt;
  hscan_t *scanner;
  int ret = 0;
  struct timespec *runtime = NULL;
  tuple_t *tuple;
  
#if __WORDSIZE == 32
  uint32_t nb_states, nb_sets, nb_tuples;
#elif __WORDSIZE == 64
  uint64_t nb_states, nb_sets, nb_tuples;
#endif

  /* Test if the finite automaton is a Büchi automaton and if it is 
     deterministic                                                            */
  if (fa->type != FA_TYPE_BUECHI) {
    errno = EINVAL;
    efa_errno = EFA_EBUECHI_UNIFR;
    efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
    return 1;
  }

  if (fa->class == FA_CLASS_DETERMINISTIC)
    return 0;
  
  if (verbose & VLVL_TIME) {
    ret = efa_start_timer();
    if (ret == -1)
      return 1;
  }
  
  /*******************************************************
   * Initialization:                                     *
   * - Allocate memory for the set hash and tuple hash   *
   * - Initialise set hash and tuple hash                *
   *******************************************************/
  hash_states = (hash_t *)fa->data_structure;

  ret = efa_alloc(&hash_sets, sizeof(hash_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  ret = efa_alloc(&hash_tuples, sizeof(hash_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  hash_sets = hash_init(hash_sets, 1);
  hash_tuples = hash_init(hash_tuples, 1);
  
  hash_sets->function = hash_states->function;
  hash_tuples->function = hash_states->function;
  
  // make the Büchi automaton complete
  ret = fa_hash_complete(fa);
  if (ret != 0)
    return 1;
  
  unifr_part = BUECHI_PART_FINITE;
  
  /****************************************************************************
   * Tuple construction without coloring                                      *
   * - Apply modified subset construction to each set of the tuple            *
   * - Construct finite part of complementary Büchi automaton                 *
   ****************************************************************************/
  
  // initialization
  elt = NULL;
  node = NULL;
  work_list = NULL;
  
  node_state = hash_states->initial;
  
  // allocate memory for hash node list element
  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
  if (ret == -1)
    goto alloc_error;
    
  elt->node = node_state;
    
  // create a set of states from the state list and add it to set hash table
  ret = fa_hash_create_insert_sos(elt, hash_sets, &node_set);
  if (ret == 1)
    return ret;
  
  hash_sets->initial = node_set;
  
  // compute successor sets
  elt->node = node_set;
  list_append(work_list, elt);
  ret = buechi_hash_mod_sub_const(work_list, hash_sets, fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  work_list = NULL;
  
  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  elt->node = node_set;
  
  // create a tuple from the set of states list and add it to tuple hash table
  ret = buechi_hash_create_insert_tuple(elt, hash_tuples, &node_tuple);
  if (ret == 1)
    return ret;
  
  hash_tuples->initial = node_tuple;
  initial_tuple_len = strlen(hash_tuples->initial->key);
  ((tuple_t *)node_tuple->data)->part = BUECHI_PART_INITIAL;
  elt->node = node_tuple;
     
  list_append(work_list, elt);
  
  /* The state has been transformed into a set of states and then into a tuple.
     We can now begin to construct the finite part of the complementary Büchi 
     automaton.                                                               */
  ret = buechi_hash_tuple_const2(work_list, hash_states, hash_sets, hash_tuples,
				 fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  /****************************************************************************
   * Tuple construction with coloring:                                        *
   * - Apply modified subset construction to each set of the tuple            *
   * - Construct ininite part of complementary Büchi automaton                *
   ****************************************************************************/
  
  // initialization
  elt = NULL;
  node_set = NULL;
  work_list = NULL;
  
  unifr_part = BUECHI_PART_INFINITE;
  
  // add the initial tuple to the work list again for infinite part construction
  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
  if (ret == -1)
    goto alloc_error;
 
  elt->node = hash_tuples->initial;
  list_append(work_list, elt);
  
  // compute the infinite part of the complementary Büchi automaton
  ret = buechi_hash_tuple_const2(work_list, hash_states, hash_sets,
				 hash_tuples, fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  /*****************************************************************************
   * Connect finite and infinite parts:                                        *
   * - Apply tuple construction with coloring to each tuple in the finite part *
   * - Construct complementary Büchi automaton                                 *
   ****************************************************************************/
  
  // initialization
  elt = NULL;
  node = NULL;
  tuple = NULL;
  work_list = NULL;
  
  // allocate memory for a hash table scanner
  ret = efa_alloc(&scanner, sizeof(hscan_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  hash_scan_begin(scanner, hash_tuples);
  node = hash_scan_next(scanner);
  
  while (node) {
    elt = NULL;
    tuple = (tuple_t *)node->data;
    
    /* To connect the finite and infinite part we only need the tuples in 
       the finite part.                                                       */
    if (tuple->part != BUECHI_PART_FINITE) {
      node = hash_scan_next(scanner);
      continue;
    }
    
    // allocate memory for a tuple list
    ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    // append the tuple element to the work list
    elt->node = node;
    list_append(work_list, elt);
    
    node = hash_scan_next(scanner);
  }
  
  efa_free(scanner);
  
  // connect the finite and infinite part
  ret = buechi_hash_tuple_const2(work_list, hash_states, hash_sets, 
				 hash_tuples, fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  
  /* Transform tuples into states such that they can be written to an output 
     file                                                                     */
  nb_states = hash_states->nodecount;
  nb_sets = hash_sets->nodecount;
  nb_tuples = hash_tuples->nodecount;
  hash_free_state(hash_states, 1, fa->nb_symbols);
  hash_free_sos(hash_sets, 0, fa->nb_symbols);
  
  hash_states = buechi_tuple_hash_to_state_hash(hash_tuples, fa->nb_symbols);
  if (hash_states == NULL)
    return 1;
  
  fa->data_structure = (void *)hash_states;
  
  if (verbose & VLVL_TIME) {
    runtime = efa_end_timer();
    if (runtime == NULL)
      return 1;
    
    printf(" [BUECHI] Complementing Büchi automaton '%s': %ld s %ld ns\n", 
	   fa->name, (signed long int) runtime->tv_sec,
	   (signed long int) runtime->tv_nsec);
    efa_free(runtime);
  }
  
#if __WORDSIZE == 32
  printf(" [BUECHI] Number of states: %"PRIu32"\n \
[BUECHI] Number of sets and mixed sets: %"PRIu32"\n \
[BUECHI] Number of tuples: %"PRIu32"\n", nb_states, nb_sets, nb_tuples);
#elif __WORDSIZE == 64
  printf(" [BUECHI] Number of states: %"PRIu64"\n \
[BUECHI] Number of sets and mixed sets: %"PRIu64"\n \
[BUECHI] Number of tuples: %"PRIu64"\n", nb_states, nb_sets, nb_tuples);
#endif
  
  return 0;

alloc_error:
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif /* HAVE_MODULE_HASH && HAVE_MODULE_FAMOD */

#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_unifr2(fa_t *automaton) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif /* HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD */

/*
 * buechi_unifr3:
 * This is a determinization-based Büchi complementation algorithm. It produces 
 * a complementary Büchi automaton from a complete non-deterministc Büchi 
 * automaton without optimization.
 * Returns 0 on success and 1 on failure.
 */
#if HAVE_MODULE_HASH && HAVE_MODULE_FAMOD
int buechi_hash_unifr3(fa_t *fa, unsigned char verbose) {
  hash_t *hash_states, *hash_sets, *hash_tuples;
  hnode_t *node_state, *node_set, *node_tuple, *node;
  hnlst_t *work_list, *elt;
  hscan_t *scanner;
  int ret = 0;
  struct timespec *runtime = NULL;
  tuple_t *tuple;
  
#if __WORDSIZE == 32
  uint32_t nb_states, nb_sets, nb_tuples;
#elif __WORDSIZE == 64
  uint64_t nb_states, nb_sets, nb_tuples;
#endif

  /* Test if the finite automaton is a Büchi automaton and if it is 
     deterministic                                                            */
  if (fa->type != FA_TYPE_BUECHI) {
    errno = EINVAL;
    efa_errno = EFA_EBUECHI_UNIFR;
    efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
    return 1;
  }

  if (fa->class == FA_CLASS_DETERMINISTIC)
    return 0;
  
  if (verbose & VLVL_TIME) {
    ret = efa_start_timer();
    if (ret == -1)
      return 1;
  }
  
  /*******************************************************
   * Initialization:                                     *
   * - Allocate memory for the set hash and tuple hash   *
   * - Initialise set hash and tuple hash                *
   *******************************************************/
  hash_states = (hash_t *)fa->data_structure;

  ret = efa_alloc(&hash_sets, sizeof(hash_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  ret = efa_alloc(&hash_tuples, sizeof(hash_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  hash_sets = hash_init(hash_sets, 1);
  hash_tuples = hash_init(hash_tuples, 1);
  
  hash_sets->function = hash_states->function;
  hash_tuples->function = hash_states->function;
  
  // make the Büchi automaton complete
  ret = fa_hash_complete(fa);
  if (ret != 0)
    return 1;
  
  unifr_part = BUECHI_PART_FINITE;
  
  /****************************************************************************
   * Tuple construction without coloring                                      *
   * - Apply modified subset construction to each set of the tuple            *
   * - Construct finite part of complementary Büchi automaton                 *
   ****************************************************************************/
  
  // initialization
  elt = NULL;
  node = NULL;
  work_list = NULL;
  
  node_state = hash_states->initial;
  
  // allocate memory for hash node list element
  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
  if (ret == -1)
    goto alloc_error;
    
  elt->node = node_state;
    
  // create a set of states from the state list and add it to set hash table
  ret = fa_hash_create_insert_sos(elt, hash_sets, &node_set);
  if (ret == 1)
    return ret;
  
  hash_sets->initial = node_set;
  
  // compute successor sets
  elt->node = node_set;
  list_append(work_list, elt);
  ret = buechi_hash_mod_sub_const(work_list, hash_sets, fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  work_list = NULL;
  
  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  elt->node = node_set;
  
  // create a tuple from the set of states list and add it to tuple hash table
  ret = buechi_hash_create_insert_tuple(elt, hash_tuples, &node_tuple);
  if (ret == 1)
    return ret;
  
  hash_tuples->initial = node_tuple;
  initial_tuple_len = strlen(hash_tuples->initial->key);
  ((tuple_t *)node_tuple->data)->part = BUECHI_PART_INITIAL;
  elt->node = node_tuple;
     
  list_append(work_list, elt);
  
  /* The state has been transformed into a set of states and then into a tuple.
     We can now begin to construct the finite part of the complementary Büchi 
     automaton.                                                               */
  ret = buechi_hash_tuple_const(work_list, hash_states, hash_sets, hash_tuples,
				fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  /****************************************************************************
   * Tuple construction with coloring:                                        *
   * - Apply modified subset construction to each set of the tuple            *
   * - Construct ininite part of complementary Büchi automaton                *
   ****************************************************************************/
  
  // initialization
  elt = NULL;
  node_set = NULL;
  work_list = NULL;
  
  unifr_part = BUECHI_PART_INFINITE;
  
  // add the initial tuple to the work list again for infinite part construction
  ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
  if (ret == -1)
    goto alloc_error;
 
  elt->node = hash_tuples->initial;
  list_append(work_list, elt);
  
  // compute the infinite part of the complementary Büchi automaton
  ret = buechi_hash_tuple_const(work_list, hash_states, hash_sets,
				hash_tuples, fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  /*****************************************************************************
   * Connect finite and infinite parts:                                        *
   * - Apply tuple construction with coloring to each tuple in the finite part *
   * - Construct complementary Büchi automaton                                 *
   ****************************************************************************/
  
  // initialization
  elt = NULL;
  node = NULL;
  tuple = NULL;
  work_list = NULL;
  
  // allocate memory for a hash table scanner
  ret = efa_alloc(&scanner, sizeof(hscan_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  hash_scan_begin(scanner, hash_tuples);
  node = hash_scan_next(scanner);
  
  while (node) {
    elt = NULL;
    tuple = (tuple_t *)node->data;
    
    /* To connect the finite and infinite part we only need the tuples in 
       the finite part.                                                       */
    if (tuple->part != BUECHI_PART_FINITE) {
      node = hash_scan_next(scanner);
      continue;
    }
    
    // allocate memory for a tuple list
    ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    // append the tuple element to the work list
    elt->node = node;
    list_append(work_list, elt);
    
    node = hash_scan_next(scanner);
  }
  
  efa_free(scanner);
  
  // connect the finite and infinite part
  ret = buechi_hash_tuple_const(work_list, hash_states, hash_sets, 
				hash_tuples, fa->nb_symbols);
  if (ret == 1)
    return ret;
  
  
  /* Transform tuples into states such that they can be written to an output 
     file                                                                     */
  nb_states = hash_states->nodecount;
  nb_sets = hash_sets->nodecount;
  nb_tuples = hash_tuples->nodecount;
  hash_free_state(hash_states, 1, fa->nb_symbols);
  hash_free_sos(hash_sets, 0, fa->nb_symbols);
  
  hash_states = buechi_tuple_hash_to_state_hash(hash_tuples, fa->nb_symbols);
  if (hash_states == NULL)
    return 1;
  
  fa->data_structure = (void *)hash_states;
  
  if (verbose & VLVL_TIME) {
    runtime = efa_end_timer();
    if (runtime == NULL)
      return 1;
    
    printf(" [BUECHI] Complementing Büchi automaton '%s': %ld s %ld ns\n", 
	   fa->name, (signed long int) runtime->tv_sec,
	   (signed long int) runtime->tv_nsec);
    efa_free(runtime);
  }
  
#if __WORDSIZE == 32
  printf(" [BUECHI] Number of states: %"PRIu32"\n \
[BUECHI] Number of sets and mixed sets: %"PRIu32"\n \
[BUECHI] Number of tuples: %"PRIu32"\n", nb_states, nb_sets, nb_tuples);
#elif __WORDSIZE == 64
  printf(" [BUECHI] Number of states: %"PRIu64"\n \
[BUECHI] Number of sets and mixed sets: %"PRIu64"\n \
[BUECHI] Number of tuples: %"PRIu64"\n", nb_states, nb_sets, nb_tuples);
#endif
  
  return 0;

alloc_error:
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif /* HAVE_MODULE_HASH && HAVE_MODULE_FAMOD */

#if HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD
int buechi_rbtree_unifr3(fa_t *automaton) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_BUECHI, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif /* HAVE_MODULE_RBTREE && HAVE_MODULE_FAMOD */