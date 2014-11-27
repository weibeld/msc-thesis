/*
 * efatool.c: standard extended finite automata library tool
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

/*
 * This executable file is used to test different kinds of algorithms on 
 * different types of automata.
 */

#include "config.h"

#include "libefa/efa.h"
#include "libefa/errno.h"
#include "libefa/error.h"
#include "libefa/memory.h"

#if HAVE_MODULE_FADOT
# include "libefa/fadot.h"
#endif

#if HAVE_MODULE_FAXML
# include "libefa/faxml.h"
#endif

#if HAVE_MODULE_HASH
# include "libefa/hash.h"
#endif

#if HAVE_MODULE_RBTREE
# include "libefa/rbtree.h"
#endif

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
 * main:
 * Main function. Setup the environment and work on the automaton.
 */
int main(int argc, char *argv[]) {
  char *c;
  efa_ioh_t io_handler;
  efa_opts_t *options;
  fa_t *a1 = NULL, *a2 = NULL, *automaton;
  int i, ret, ret2;
  void *read_data = NULL, *write_data = NULL;

#if HAVE_MODULE_FAXML
  frd_t *faxml_read_data;
  fwd_t *faxml_write_data;
#endif
  
  const char *word = NULL;

  // make the program portable to all locales
  setlocale(LC_ALL, "");
  
  ret = efa_alloc(&options, sizeof(efa_opts_t), 1);
  if (ret == -1) {
    efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }
  
  ret = efa_alloc(&automaton, sizeof(fa_t), 1);
  if (ret == -1) {
    efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }
  
  // initialize options
#if HAVE_MODULE_FAXML
  options->schema_file_path = FAXML_SCHEMA ;
#endif
  options->program_name = "efatool";
  automaton->type = FA_TYPE_ORDINARY;
  automaton->class = FA_CLASS_NON_DETERMINISTIC;
#if HAVE_MODULE_HASH
  automaton->data_type = DT_HASH_TABLE;
  io_handler = hash_io;
  options->function = hash_function_names[HASH_FUN_KAZLIB];
#else
  automatno->data_type = DT_RB_TREE;
  io_handler = rbtree_io;
#endif
  
  // parse the options given on the command line
  efa_parse_opts(argc, argv, automaton, options);

  if (options->output_file_path) {
    for (i = 0; i < FILE_EXTENSION_NB; i++) {
      c = strcasestr(options->output_file_path, file_extension_names[i]);
      if (c)
	break;
    }
    switch (i) {
#if HAVE_MODULE_FADOT
    case FILE_EXTENSION_DOT:
      write_data = NULL;
      options->out_ext = FILE_EXTENSION_DOT;
      break;
#endif
    case FILE_EXTENSION_REGEXP:
      write_data = NULL;
      options->out_ext = FILE_EXTENSION_REGEXP;
      break;
#if HAVE_MODULE_FAXML
    case FILE_EXTENSION_XML:
      ret = efa_alloc(&faxml_write_data, sizeof(fwd_t), 1);
      if (ret == -1) {
	efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
	exit(EXIT_FAILURE);
      }

      write_data = (void *)faxml_write_data;
      options->out_ext = FILE_EXTENSION_XML;
      break;
#endif
    default:
      errno = EDOM;
      efa_errno = EFA_EMAIN;
      efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
      exit(EXIT_FAILURE);
      break;
    }
  }
  
  /* make sure to read, transform or generate the automaton or the regular
     expression before doing anything else                                   */
  if (options->input_file_path) {
    for (i = 0; i < FILE_EXTENSION_NB; i++) {
      c = strcasestr(options->input_file_path, file_extension_names[i]);
      if (c)
	break;
    }
    switch (i) {
#if HAVE_MODULE_FADOT
    case FILE_EXTENSION_DOT:
      options->in_ext = FILE_EXTENSION_DOT;
      break;
#endif
    case FILE_EXTENSION_REGEXP:
      options->in_ext = FILE_EXTENSION_REGEXP;
      break;
#if HAVE_MODULE_FAXML
    case FILE_EXTENSION_XML:
      options->in_ext = FILE_EXTENSION_XML;
      
      // initialise the FAXML module
      FAXML_INITIALIZE
      
#if DEBUG
      ret = strncmp(options->schema_file_path, "-", 1);
      if (ret == 0) {
	errno = EPIPE;
	efa_errno = EFA_EMAIN;
	efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
	exit(EXIT_FAILURE);
      } else
#endif /* DEBUG */
      
      ret = efa_alloc(&faxml_read_data, sizeof(frd_t), 1);
      if (ret == -1)
	return EXIT_FAILURE;
      
      faxml_read_data->schema_file_path = options->schema_file_path;
      faxml_read_data->function = options->function;
      read_data = (void *)faxml_read_data;
      break;
#endif /* HAVE_MODULE_FAXML */
    default:
      ret = strncmp(options->input_file_path, "-", 1);
      if (ret == 0) {
	errno = EPIPE;
	efa_errno = EFA_EMAIN;
	efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
	exit(EXIT_FAILURE);
      } else {
	errno = EDOM;
	efa_errno = EFA_EMAIN;
	efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
	exit(EXIT_FAILURE);
      }
      break;
    }
  }

  io_handler = *efa_dt_io[automaton->data_type];
  ret2 = (*io_handler.fa_read[options->in_ext])(automaton, 
options->input_file_path, options->opt_verbose, read_data);
  if (ret2 == 1)
    goto error;

#if HAVE_MODULE_FAXML
  if (options->output_file_path && (options->out_ext == FILE_EXTENSION_XML)) {
    // create a new xmlWriter that writer to file 'xml_file_path'
    faxml_write_data->writer = 
xmlNewTextWriterFilename(options->output_file_path, 0);
    if (faxml_write_data->writer == NULL) {
      errno = ENOMEM;
      efa_errno = EFA_EXMLNEWTEXTWRITERFILENAME;
      efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
      goto error;
    }

    // switch on indentation
    ret = xmlTextWriterSetIndent(faxml_write_data->writer, 1);
    if (ret == -1) {
      errno = 231;
      efa_errno = EFA_EXMLTEXTWRITERSETINDENT;
      efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
    }

    // start the XML document and set some attributes for the XML writer
    ret = xmlTextWriterStartDocument(faxml_write_data->writer, NULL, NULL, 
				     NULL);
    if (ret == -1) {
      errno = 231;
      efa_errno = EFA_EXMLTEXTWRITERSTARTDOCUMENT;
      efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
      goto error;
    }

    // begin the XML document with FAXML element
    ret = xmlTextWriterStartElement(faxml_write_data->writer, 
				    faxml_node_names[FAXML_FAXML]);
    if (ret == -1) {
      errno = 231;
      efa_errno = EFA_EXMLTEXTWRITERSTARTELEMENT;
      efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
      goto error;
    }

    // write and close the VERSION attribute
    ret = xmlTextWriterWriteAttribute(faxml_write_data->writer,
				    faxml_faxml_attributes[FAXML_FAXML_VERSION],
				      (const xmlChar *)VERSION);
    if (ret == -1) {
      errno = 231;
      efa_errno = EFA_EXMLTEXTWRITERWRITEATTRIBUTE;
      efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
      goto error;
    }
  }
#endif

  /* Main program loop: Read an automaton from input and apply all algorithms to
     it, before writing the resulting automaton to an output file. The order of
     execution of the algorithms is given by the parameters of the algorithm
     option.                                                                  */
  do {
    if (options->algorithms) {
      /* NOTE: Cleanup of the FAXML library is done later */  
      for (i = 0; options->algorithms[i].function != NULL; i++) {
	switch (options->algorithms[i].type) {
	  case ALGO_T_COMPLEMENTATION:
	    ret = ((algorithm_comp_t) 
options->algorithms[i].function)(automaton, options->opt_verbose);
	    break;
	  case ALGO_T_EQUIVALENCE:
	    ret = ((algorithm_equiv_t) options->algorithms[i].function)(a1, 
a2);
	    break;
	  case ALGO_T_GENERATOR:
	    ret = ((algorithm_gen_t) 
options->algorithms[i].function)(automaton);
	    break;
	  case ALGO_T_MINIMIZATION:
	    ret = ((algorithm_min_t) 
options->algorithms[i].function)(automaton);
	    break;
	  case ALGO_T_RUN:
	    ret = ((algorithm_run_t) 
options->algorithms[i].function)(automaton, word);
	    break;
	  case ALGO_T_TRANSFORMATION:
	    ret = ((algorithm_trans_t) 
options->algorithms[i].function)(automaton);
	    break;
	  default:
	    errno = EDOM;
	    efa_errno = EFA_EMAIN;
	    efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
	    exit(EXIT_FAILURE);
	    break;
	}
	if (ret != 0)
	  goto error;
      }
    }

    if (options->output_file_path) {
      io_handler = *efa_dt_io[automaton->data_type];
      ret = (*io_handler.fa_write[options->out_ext])(automaton, 
options->output_file_path, options->opt_verbose, write_data);
      if (ret)
	goto error;
    }

#if HAVE_MODULE_HASH
    if (automaton->data_type == DT_HASH_TABLE)
      hash_free_state((hash_t *)automaton->data_structure, automaton->class,
		      automaton->nb_symbols);
#endif

#if HAVE_MODULE_RBTREE
    if (automaton->data_type == DT_RB_TREE)
      rbtree_free((rbtree_t *)automaton->data_structure, automaton->class,
		  automaton->nb_symbols);
#endif

    if (ret2 == -1)
      break;

    io_handler = *efa_dt_io[automaton->data_type];
    ret2 = (*io_handler.fa_read[options->in_ext])(automaton, 
		options->input_file_path, options->opt_verbose, read_data);
  } while (ret2 == 0 || ret2 == -1);

  if (ret2 == 1)
    goto error;

#if HAVE_MODULE_FAXML
  if (options->input_file_path && (options->out_ext == FILE_EXTENSION_XML)) {
    if (options->output_file_path) {
      // close the FAXML element
      ret = xmlTextWriterEndElement(faxml_write_data->writer);
      if (ret == -1) {
	errno = 231;
	efa_errno = EFA_EXMLTEXTWRITERENDELEMENT;
	efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
	goto error;
      }

      // close the XML document
      ret = xmlTextWriterEndDocument(faxml_write_data->writer);
      if (ret == -1) {
	errno = 231;
	efa_errno = EFA_EXMLTEXTWRITERENDDOCUMENT;
	efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);
	goto error;
      }

      xmlFreeTextWriter(faxml_write_data->writer);

      efa_free(faxml_write_data);
      write_data = NULL;
    }
    
    // cleanup the FAXML module
    FAXML_CLEANUP
  }
#endif
  
  efa_free(options->algorithms);

  free((void *)automaton->alphabet);
  free((void *)automaton->name);
  efa_free(automaton);
  
  if (a2) {
    free((void *)a2->alphabet);
    free((void *)a2->name);
#if HAVE_MODULE_HASH
    if (a2->data_type == DT_HASH_TABLE)
      hash_free_state((hash_t *)a2->data_structure, a2->class, a2->nb_symbols);
#endif
#if HAVE_MODULE_RBTREE
    if (a2->data_type == DT_RB_TREE)
      rbtree_free((rbtree_t *)a2->data_structure, a2->class, a2->nb_symbols);
#endif
    efa_free(a2);
  }
  
  return EXIT_SUCCESS;
  
error:
  efa_free(options->algorithms);

#if HAVE_MODULE_HASH
  if (automaton->data_type == DT_HASH_TABLE)
    hash_free_state((hash_t *)automaton->data_structure, automaton->class,
	      automaton->nb_symbols);
#endif

#if HAVE_MODULE_RBTREE
  if (automaton->data_type == DT_RB_TREE)
    rbtree_free((rbtree_t *)automaton->data_structure, automaton->class,
		automaton->nb_symbols);
#endif

  efa_free(automaton);
  
  efa_error(EFA_MOD_TOOL, __LINE__, __FILE__);

  return EXIT_FAILURE;
}
