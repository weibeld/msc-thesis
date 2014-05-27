/*
 * famod.c: finite automata plus
 *
 * Copyright (C) 2007-2011 David Lutterkort
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
 * Author: David Lutterkort <dlutter@redhat.com>
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

/*
 * This implementation follows closely the Java dk.brics.automaton package
 * by Anders Moeller. The project's website is
 * http://www.brics.dk/automaton/.
 *
 * It is by no means a complete reimplementation of that package; only a
 * subset of what Automaton provides is implemented here.
 */

/*
 * This is a modification of the finite automaton library by David Lutterkort.
 * The projects's website is
 * http://augeas.net/libfa/index.html
 * 
 * The library interface has been modified such that it is compatible with the
 * libefa interface.
 */

#include "config.h"

#include "libefa/errno.h"
#include "libefa/error.h"
#include "libefa/famod.h"
#include "libefa/list.h"
#include "libefa/memory.h"

#if HAVE_MODULE_BUECHI
# include "libefa/buechi.h"
#endif

#if HAVE_MODULE_HASH
# include "libefa/hash.h"
#endif

#if HAVE_MODULE_RBTREE
# include "libefa/rbtree.h"
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>

#if HAVE_MODULE_HASH
int fa_hash_boltzmann_samplers(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_boltzmann_samplers(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_HASH
int fa_hash_brzozowski(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_brzozowski(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

/* 
 * fa_collect:
 * Clean up FA by removing dead transitions and states and reducing
 * transitions. Unreachable states are freed. The return value is the same
 * as FA.
 */
#if HAVE_MODULE_HASH
int fa_hash_collect(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_collect(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_HASH
int fa_hash_complement(fa_t *automaton __attribute__((unused)),
		       unsigned char verbose __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_complement(fa_t *automaton __attribute__((unused))
			 unsigned char verbose __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

/*
 * fa_complete:
 * Completes the automaton by adding a state.
 * Returns 0 on success.
 */
#if HAVE_MODULE_HASH
int fa_hash_complete(fa_t *automaton) {
  hnode_t *node, *trash_node = NULL;
  hscan_t *scanner;
  int i, j, ret = 0;
  size_t len = 0;
  state_t *state, *trash_state;

  ret = efa_alloc(&scanner, sizeof(hscan_t), 1);
  if (ret == -1)
    goto alloc_error;

  hash_scan_begin(scanner, (hash_t *)automaton->data_structure);
  node = hash_scan_next(scanner);

  while (node) {
    state = (state_t *)node->data;
    for (i = 0; i < automaton->nb_symbols; i++) {
      if (automaton->class == FA_CLASS_DETERMINISTIC) {
	if ((hnode_t **)state->transition_row == NULL) {
	  ret = efa_alloc((hnode_t **)state->transition_row,
			  sizeof(hnode_t *), automaton->nb_symbols);
	  if (ret == -1)
	    goto alloc_error;
	}
	if (((hnode_t **)state->transition_row)[i] == NULL) {
	  if (trash_node == NULL) {
	    ret = efa_alloc(&trash_node, sizeof(hnode_t), 1);
	    if (ret == -1)
	      goto alloc_error;
	    
	    ret = efa_alloc(&trash_state, sizeof(state_t), 1);
	    if (ret == -1)
	      goto alloc_error;
	    
	    trash_node->data = (void *)trash_state;

#if __WORDSIZE == 32
	    len = 10 + 1;
#elif __WORDSIZE == 64
	    len = 20 + 1;
#endif
	    ret = efa_alloc(&trash_state->label, sizeof(char), len);
	    if (ret == -1)
	      goto alloc_error;

#if __WORDSIZE == 32
	    snprintf((char *)trash_state->label, len, "%010lu",
		     (unsigned long int) trash_node);
#elif __WORDSIZE == 64
	    snprintf((char *)trash_state->label, len, "%020lu",
		     (unsigned long int) trash_node);
#endif
	    trash_state->reachable = 1;
	    ret = efa_alloc(&trash_state->transition_row, sizeof(hnode_t **), 
automaton->nb_symbols);
	    if (ret == -1)
	      goto alloc_error;

	    for (j = 0; j < automaton->nb_symbols; j++)
	      ((hnode_t **)trash_state->transition_row)[j] = trash_node;

	    hash_insert((hash_t *)automaton->data_structure, trash_node,
			trash_state->label);
	  }
	  ((hnode_t **)state->transition_row)[i] = trash_node;
	}
	// non-deterministic automaton
      } else {
	if ((hnode_t ***)state->transition_row == NULL) {
	  ret = efa_alloc((hnode_t ****)&state->transition_row,
			  sizeof(hnode_t **), automaton->nb_symbols);
	  if (ret == -1)
	    goto alloc_error;
	  
	  for (j = 0; j < automaton->nb_symbols; j++) {
	    ret = efa_alloc(&(((hnode_t ***)state->transition_row)[j]),
			    sizeof(hnode_t *), automaton->nb_symbols);
	    if (ret == -1)
	      goto alloc_error;
	  }
	}
	if (((hnode_t ***)state->transition_row)[i][0] == NULL) {
	  if (trash_node == NULL) {
	    ret = efa_alloc(&trash_node, sizeof(hnode_t), 1);
	    if (ret == -1)
	      goto alloc_error;
	    
	    ret = efa_alloc(&trash_state, sizeof(state_t), 1);
	    if (ret == -1)
	      goto alloc_error;
	    
	    trash_node->data = (void *)trash_state;

#if __WORDSIZE == 32
	    len = 10 + 1;
#elif __WORDSIZE == 64
	    len = 20 + 1;
#endif
	    ret = efa_alloc(&trash_state->label, sizeof(char), len);
	    if (ret == -1)
	      goto alloc_error;

#if __WORDSIZE == 32
	    snprintf((char *)trash_state->label, len, "%010lu",
		     (unsigned long int) trash_node);
#elif __WORDSIZE == 64
	    snprintf((char *)trash_state->label, len, "%020lu",
		     (unsigned long int) trash_node);
#endif
	    trash_state->reachable = 1;
	    ret = efa_alloc(&trash_state->transition_row, sizeof(hnode_t **), 
			    automaton->nb_symbols);
	    if (ret == -1)
	      goto alloc_error;

	    for (j = 0; j < automaton->nb_symbols; j++) {
	      ((hnode_t ***)trash_state->transition_row)[j] = calloc(2,
							sizeof(hnode_t **));
	      if (((hnode_t ***)trash_state->transition_row)[j] == NULL)
		goto alloc_error;

	      ((hnode_t ***)trash_state->transition_row)[j][0] = trash_node;
	    }

	    hash_insert((hash_t *)automaton->data_structure, trash_node,
			trash_state->label);
	  }

	  ret = efa_alloc(&((hnode_t ***)state->transition_row)[i], 
sizeof(hnode_t **), 2);
	  if (ret == -1)
	    goto alloc_error;

	  ((hnode_t ***)state->transition_row)[i][0] = trash_node;
	}
      }
    }

    node = hash_scan_next(scanner);
  }

  efa_free(scanner);

  return 0;
  
alloc_error:
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif /* HAVE_MODULE_HASH */

#if HAVE_MODULE_RBTREE
int fa_rbtree_complete(fa_t *automaton) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

/*
 * fa_copy_sos:
 * Physically copies a deterministic set of states. The user has to free the 
 * memory of the returned pointer.
 * Returns a pointer to a set of states on success or NULL on failure.
 */
#if HAVE_MODULE_HASH
hnode_t *fa_hash_copy_sos(hnode_t *set, unsigned int nb_symbols) {
  hnode_t *node;
  int i, ret;
  size_t len;
  sos_t *set_copy;
  
  if (set == NULL) {
    errno = EINVAL;
    efa_errno = EFA_EFA_COPY_SOS;
    return NULL;
  }
  
  assert((hnode_t **)((sos_t *)set->data)->transition_row != NULL);
  
  // allocate memory for the copied set of states
  ret = efa_alloc(&set_copy, sizeof(sos_t), 1);
  if (ret == -1)
    goto alloc_error;
      
  memcpy((void *)set_copy, (const void *)set->data, sizeof(sos_t));
      
  len = strlen(set->key);
  ret = efa_alloc(&set_copy->label, sizeof(char), len + 1);
  if (ret == -1)
    goto alloc_error;
      
  memcpy((void *)set_copy->label, (const void *)((sos_t *)set->data)->label,
	     len);
     
  for (len = 0; ((hnode_t **)((sos_t *)set->data)->states)[len] != NULL; len++);
      
  ret = efa_alloc(&set_copy->states, sizeof(hnode_t *), len + 1);
  if (ret == -1)
    goto alloc_error;
      
  for (i = 0; i <= len; i++) {
    ((hnode_t **)set_copy->states)[i] = ((hnode_t **)((sos_t 
*)set->data)->states)[i];
  }
  
  ret = efa_alloc(&set_copy->transition_row, sizeof(hnode_t *), nb_symbols);
  if (ret == -1)
    goto alloc_error;
  
#if HAVE_MODULE_BUECHI
  if (!((sos_t *)set->data)->mixed) {
#endif
  for (i = 0; i < nb_symbols; i++) {
    ((hnode_t **)set_copy->transition_row)[i] = ((hnode_t **)((sos_t 
*)set->data)->transition_row)[i];
  }
#if HAVE_MODULE_BUECHI
  }
#endif
  
  ret = efa_alloc(&node, sizeof(hnode_t), 1);
  if (ret == -1)
    goto alloc_error;
      
  node->data = (void *)set_copy;
  node->key = set_copy->label;
  
  assert(node != NULL &&
	 node->key != NULL &&
	 node->data != NULL &&
	 set_copy->label != NULL &&
	 set_copy->states != NULL &&
	 set_copy->transition_row != NULL);
  
  return node;
  
alloc_error:
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif

#if HAVE_MODULE_RBTREE
rbnode_t *fa_rbtree_copy_sos(rbnode_t *set, unsigned int nb_symbols) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return NULL;
}
#endif

/*
 * fa_create_insert_sos:
 * Creates a set of states in SOS from the list of states LST and adds it to 
 * the hash table HASH. The created set of states SOS will have an empty 
 * transition row! The user has to free the allocated memory of the returned 
 * hash node.
 * Returns on success 0 or 1 if an error occurred.
 */
#if HAVE_MODULE_HASH
int fa_hash_create_insert_sos(hnlst_t *lst, hash_t *hash, hnode_t **sos) {
  const char *label;
  unsigned int reset_dyn = 0;
  
  if (!hash->dynamic) {
    hash->dynamic = 1;
    reset_dyn = 1;
  }
  
  // make sure the list of states LST is not NULL
  if (lst == NULL) {
    errno = EINVAL;
    efa_errno = EFA_EFA_CREATE_INSERT_SOS;
    return 1;
  }
  
  *sos = NULL;
  
  // assemble the label of the set of states
  label = fa_hash_create_sos_label(lst);
  if (label == NULL)
    return 1;
  
  // check if the new set of states is in HASH table
  *sos = hash_lookup(hash, label);
    
  // new set of states is not in HASH table - add it
  if (*sos == NULL) {
    // create a set of states from the list of states
    *sos = fa_hash_create_sos(lst);
    if (*sos == NULL)
      return 1;
    
    // insert the new set of states into the hash table
    hash_insert(hash, *sos, label);
  } else {
    // new set of states is already in HASH table
    free((void *)label);
  }

  if (reset_dyn)
    hash->dynamic = 0;
  
  assert(*sos != NULL &&
	 (*sos)->key != NULL &&
	 (*sos)->data != NULL &&
	 ((sos_t *)(*sos)->data)->label != NULL &&
	 ((sos_t *)(*sos)->data)->states != NULL);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_create_insert_sos(rbnlst_t *lst, rbtree_t *tree, rbnode_t **sos) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return NULL;
}
#endif

/*
 * fa_create_sos:
 * Creates a set of states from a list of states. The user has to free the 
 * allocated memory.
 * If compiled with module BUECHI, the function sets the BUECHI_INFO field by 
 * default to BUECHI_INFO_FORDINARY.
 * Returns a pointer to a set of states on success or NULL on failure.
 */
#if HAVE_MODULE_HASH
hnode_t *fa_hash_create_sos(hnlst_t *lst) {
  const char *label;
  hnode_t *node;
  unsigned int i, mixed, ret;
  sos_t *set;
  size_t state_count;
  
  // make sure the list of states LST is not NULL
  if (lst == NULL) {
    errno = EINVAL;
    efa_errno = EFA_EFA_CREATE_SOS;
    return NULL;
  }
  
  // assemble the label of the set of states
  label = fa_hash_create_sos_label(lst);
  if (label == NULL)
    return NULL;
   
  // allocate memory for the new set of states
  ret = efa_alloc(&node, sizeof(hnode_t), 1);
  if (ret == -1)
    goto alloc_error;
      
  ret = efa_alloc(&set, sizeof(sos_t), 1);
  if (ret == -1)
    goto alloc_error;
    
  state_count = 0;
  list_for_each(iter, lst) {
    state_count++;
  }
    
  ret = efa_alloc(&set->states, sizeof(hnode_t *), state_count + 1);
  if (ret == -1)
    goto alloc_error;
    
  node->data = (void *)set;
      
  // set properties for the new set of states
#if HAVE_MODULE_BUECHI
  mixed = set->accept |= ((state_t *)lst->node->data)->accept;
  set->buechi_info = BUECHI_INFO_FORDINARY;
#endif
  i = 0;
  list_for_each(iter, lst) {
    set->accept |= ((state_t *)iter->node->data)->accept;
    set->reachable |= ((state_t *)iter->node->data)->reachable;
    ((hnode_t **)set->states)[i++] = iter->node;
#if HAVE_MODULE_BUECHI
    if (mixed != ((state_t *)iter->node->data)->accept)
	set->mixed = 1;
#endif
  }
    
  set->label = label;
  node->key = label;
    
  assert((node != NULL) && (node->data != NULL));
  assert(((hnode_t **)set->states)[0] != NULL);
    
  return node;
  
alloc_error:
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif

#if HAVE_MODULE_RBTREE
rbnode_t *fa_rbtree_create_sos(rbnlst_t *lst) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return NULL;
}
#endif

/*
 * fa_create_sos_label:
 * Creates a label for a set of states from the list of states LST. The user 
 * has to free the allocated memory of the label.
 * Return a character array on success or NULL on failure.
 */
#if HAVE_MODULE_HASH
const char *fa_hash_create_sos_label(hnlst_t *lst) {
  char *buffer;
  unsigned int i, ret, state_count;
  size_t len;
  
  // make sure the list of states LST is not NULL
  if (lst == NULL) {
    errno = EINVAL;
    efa_errno = EFA_EFA_CREATE_SOS;
    return NULL;
  }
  
  len = 0;
  state_count = 0;
  list_for_each(iter, lst) {
    len += strlen(iter->node->key);
    state_count++;
  }

  // assemble the label of the new set of states
  ret = efa_alloc(&buffer, sizeof(char), len + state_count + 2);
  if (ret == -1) {
    efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }
    
  i = 0;
  buffer[i++] = '{';
  list_for_each(iter, lst) {
    len = strlen(iter->node->key);
    strncat(&buffer[i], iter->node->key, len);
    i += len;
      
    if (iter->next != NULL)
      buffer[i++] = ',';
  }
  buffer[i] = '}';
  
  return (const char *)buffer;
}
#endif

#if HAVE_MODULE_RBTREE
const char *fa_rbtree_create_sos_label(rbnlst_t *lst) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return NULL;
}
#endif

#if HAVE_MODULE_HASH
int fa_hash_equivalence(fa_t *a1 __attribute__((unused)),
			fa_t *a2 __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_equivalence(fa_t *a1 __attribute__((unused)),
			  fa_t *a2 __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_HASH
int fa_hash_hopcroft(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_hopcroft(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

/*
 * fa_nfa_move:
 * Subset construction move function. Computes the successor set of states of a 
 * set of states with a given symbol. The user has to free the allocated 
 * memory.
 * Returns 0 on success and 1 on failure.
 */
#if HAVE_MODULE_HASH
int fa_hash_nfa_move(hash_t *hash, hnode_t *in_set, hnode_t **out_set,
		     unsigned int symbol) {
  int i, j, ret;
  hnode_t *node, *to_node;
  hnlst_t *e, *elt, *sos;
  state_t *state;
  
  sos = NULL;
  
  assert(((char *)in_set->key)[0] != '\0');
  
  // loop through all states in the set of states IN_SET
  i = 0;
  node = ((hnode_t **)((sos_t *)in_set->data)->states)[i++];
  while (node) {
    state = (state_t *)node->data;
    
    // check for states with empty transition row
    if (state->transition_row == NULL) {
      node = ((hnode_t **)((sos_t *)in_set->data)->states)[i++];
      continue;
    }
    
    // loop through the transition cell of symbol to generate a list of states
    j = 0;
    to_node = ((hnode_t ***)state->transition_row)[symbol][j++];
    while (to_node) {
      // allocate memory for a hash node list element
      ret = efa_alloc(&elt, sizeof(hnlst_t), 1);
      if (ret == -1)
	goto alloc_error;
      
      elt->node = to_node;
      
      // check if the list is empty
      if (sos == NULL)
	list_cons(sos, elt);
      else {
	/* check where to insert the state ELT, such that the set of states is
	   ordered                                                            */
	for (e = sos; e != NULL; e = e->next) {
	  ret = hash->compare(to_node->key, e->node->key);
	  // only act if key is less than 0 or 0
	  if (ret < 0) {
	    list_insert_before(elt, e, sos);
	    break;
	    // states are equal - free the space of state ELT
	  } else if (ret == 0) {
	    efa_free(elt);
	    break;
	  }
	}
	// insert state ELT at the end of the list
	if (e == NULL)
	  list_append(sos, elt);
      }
      
      to_node = ((hnode_t ***)state->transition_row)[symbol][j++];
    }
   
    node = ((hnode_t **)((sos_t *)in_set->data)->states)[i++];
  }
  
  // check for empty set
  if (sos == NULL) {
    *out_set = NULL;
    return 0;
  }
 
  node = fa_hash_create_sos(sos);
  list_free(sos);
  
  *out_set = node;
  if (*out_set == NULL)
    return 1;
  
  return 0;

alloc_error:
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_nfa_move(rbtree_t *tree, rbnode_t *in_set, rbnode_t **out_set,
		       unsigned int symbol) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);

  return 0;
}
#endif

#if HAVE_MODULE_HASH
int fa_hash_recursive_method(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);

  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_recursive_method(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_HASH
int fa_hash_run(fa_t *automaton __attribute__((unused)),
		const char *word __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_run(fa_t *automaton __attribute__((unused)),
		  const char *word __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_HASH
int fa_hash_subset_construction(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif

#if HAVE_MODULE_RBTREE
int fa_rbtree_subset_construction(fa_t *automaton __attribute__((unused))) {
  errno = ENOSYS;
  efa_errno = EFA_EUNKNOWN;
  efa_error(EFA_MOD_FAMOD, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
  
  return 0;
}
#endif
