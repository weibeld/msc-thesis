/*
 * fadot.c: example usage of modified finite automata library
 *
 * Copyright (C) 2009, Francis Giraldeau
 * Copyright (C) 2013, Christian Göttel
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
 * Author: Francis Giraldeau <francis.giraldeau@usherbrooke.ca>
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

#include "config.h"

#include "libefa/efa.h"
#include "libefa/errno.h"
#include "libefa/error.h"
#include "libefa/fadot.h"
#include "libefa/list.h"
#include "libefa/memory.h"

#if HAVE_MODULE_FAMOD
# include "libefa/famod.h"
#endif

#if HAVE_MODULE_HASH
# include "libefa/hash.h"
#endif

#if HAVE_MODULE_RBTREE
# include "libefa/rbtree.h"
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if HAVE_MODULE_HASH
int fadot_hash_write(fa_t *automaton, const char *dot_file_path,
		     unsigned char verbose, void *data __attribute__((unused))) 
{
  const char *file_path;
  char *path, *transition_label = NULL;
  FILE *dot_file;
  hnode_t *node = NULL, *to_node = NULL;
  hnlst_t *visited_list = NULL, *visited_state = NULL;
  hscan_t *scanner = NULL;
  int i, j, k, l, n, ret = 0;
  struct timespec *runtime = NULL;
  size_t len = 0, len1 = 0, len2 = 0, len3 = 0;
  state_t *state = NULL, *to_state = NULL;

  ret = efa_alloc(&scanner, sizeof(hscan_t), 1);
  if (ret == -1)
    goto alloc_error;
  
  path = strrchr(dot_file_path, (int) '/');

  if (path == NULL)
    len1 = 0;
  else
    len1 = path - dot_file_path;

  len2 = strlen(automaton->name);
  len3 = 4;
  len = len1 + len2 + len3 + 1;

  ret = efa_alloc(&file_path, sizeof(char), len);
  if (ret == -1)
    goto alloc_error;

  if (path != NULL)
    file_path = (const char *)strncat((char *)file_path,
				      dot_file_path,
				      len1);

  file_path = (const char *)strncat((char *)file_path, automaton->name, len2);
  file_path = (const char *)strncat((char *)file_path,
				    file_extension_names[FILE_EXTENSION_DOT],
				    len3);

  if (verbose & VLVL_TIME) {
    ret = efa_start_timer();
    if (ret == -1)
      efa_error(EFA_MOD_FADOT, __LINE__, __FILE__);
  }
	 
  dot_file = fopen(file_path, "w");
  if (dot_file == NULL) {
    efa_errno = EFA_EFOPEN;
    efa_error(EFA_MOD_FADOT, __LINE__, __FILE__);
    return 1;
  }
  
  fprintf(dot_file, "digraph \"%s\" {\n\tgraph [center rankdir=LR]\n\t\"%p\" \
[shape=circle style=invis width=0.01 height=0.01];\n", automaton->name,
	  automaton);

  hash_scan_begin(scanner, (hash_t *)automaton->data_structure);
  node = hash_scan_next(scanner);

  while (node != NULL) {
    state = (state_t *)node->data;
    if (state->accept)
      fprintf(dot_file, "\t\"%s\" [shape=doublecircle];\n", state->label);
    else
      fprintf(dot_file, "\t\"%s\" [shape=circle];\n", state->label);

    // get the next state from the scanner
    node = hash_scan_next(scanner);
  }
  fprintf(dot_file, "\t\"%p\" -> \"%s\";\n", automaton,
	  ((hash_t *)automaton->data_structure)->initial->key);

  hash_scan_begin(scanner, (hash_t *)automaton->data_structure);
  node = hash_scan_next(scanner);

  while (node != NULL) {
    state = (state_t *)node->data;
    if (automaton->class == FA_CLASS_DETERMINISTIC) {
      for (i = 0; i < automaton->nb_symbols; i++) {
        // test for a non-complete automaton
        if (((hnode_t **)state->transition_row)[i] != NULL) {
	  fprintf(dot_file, "\t\"%s\" -> \"%s\" [ label = \"%c\" ];\n",
		  state->label, ((hnode_t **)state->transition_row)[i]->key,
		  (int) automaton->alphabet[i]);
        }
      }
      // automaton is non-deterministic
    } else {
      if ((hnode_t ***)state->transition_row != NULL) {
	// zero-allocate memory for the transition label
	ret = efa_alloc(&transition_label, sizeof(char),
			automaton->nb_symbols + 1);
	if (ret == -1)
	  goto alloc_error;
	
	for (i = 0; i < automaton->nb_symbols; i++) {
	  // reset the counter 'j'
	  j = 0;

	  if (((hnode_t **)state->transition_row)[i] != NULL) {
	    // now i gets complicated ...
	    // loop through all transitions of the symbol
	    while (((hnode_t ***)state->transition_row)[i][j] != NULL) {
	      n = 0;
	  
	      // assign the current symbol to the transition label
	      transition_label[n++] = automaton->alphabet[i];
	  
	      /* select the state and store it for further processing only if 
		 it was not visited                                          */
	      if (!((state_t *)((hnode_t 
***)state->transition_row)[i][j]->data)->visited) {
		to_node = ((hnode_t ***)state->transition_row)[i][j];
		to_state = (state_t *)to_node->data;
		/* see if 'to_state' is also reached by a transition with 
		   another symbol                                            */
		// loop through the rest of the transition row
		for (k = i + 1; k < automaton->nb_symbols; k++) {
		  l = 0;
		  if (((hnode_t ***)state->transition_row)[k] != NULL) {
		    while (((hnode_t ***)state->transition_row)[k][l] != NULL) {
		      // does the state appear for another symbol?
		      if (((hnode_t ***)state->transition_row)[k][l] == to_node)
		      {
			// has the state already been visited?
			if (!((state_t *)((hnode_t 
***)state->transition_row)[k][l]->data)->visited) {
			  // add the symbol to the transition label
			  transition_label[n++] = automaton->alphabet[k];
			}
		      }

		      l++;
		    }
		  }
		}
		// mark the state as visited
		to_state->visited = 1;

		// add it to the visited list for faster resetting
		ret = efa_alloc(&visited_state, sizeof(hnlst_t), 1);
		if (ret == -1)
		  goto alloc_error;
	      
		visited_state->node = to_node;
		list_append(visited_list, visited_state);

		fprintf(dot_file, "\t\"%s\" -> \"%s\" [ label = \"%s\" ];\n",
			state->label, to_state->label, transition_label);
	    
		// zero the transition label
		memset((void *)transition_label, 0,
		       (size_t) (automaton->nb_symbols + 1));
	      }

	      // go to the next state
	      j++;
	    }
	  }
	}

	// reset the 'visited' field in all transition hash nodes
	list_for_each(vis_state, visited_list) {
	  ((state_t *)vis_state->node->data)->visited = 0;
	}
      
	list_free(visited_list);
	efa_free(transition_label);
      }
    }

    // get the next state from the scanner
    node = hash_scan_next(scanner);
  }

  fprintf(dot_file, "}\n");
	 
  fclose(dot_file);
	 
  if (verbose & VLVL_TIME) {
    runtime = efa_end_timer();
    printf(" [FADOT] Writing DOT file '%s': %ld s %ld ns\n", file_path,
           (signed long int) runtime->tv_sec,
	   (signed long int) runtime->tv_nsec);
    efa_free(runtime);
  }
  
  return 0;
  
alloc_error:
  exit(EXIT_FAILURE);
}
#endif

#if HAVE_MODULE_RBTREE
int fadot_rbtree_write(fa_t *automaton, const char *dot_file_path,
		       void *data) {

  return 0;
}
#endif
