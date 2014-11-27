/*
 * faxml.h: header with import and export methods for faxml format
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

#ifndef FAXML_H_
#define FAXML_H_

#include "libefa/efa.h"

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlstring.h>
#include <libxml/xmlwriter.h>
#include <libxml/xmlversion.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* FAXML_CLEANUP:
 * - Cleanup the XML library (misleading function name)
 * - Dump memory blocks allocated to the file '.memorylist'
 */
#define FAXML_CLEANUP \
  xmlCleanupParser(); \
  xmlMemoryDump();

// Initialize and test the XML library
#define FAXML_INITIALIZE LIBXML_TEST_VERSION

#define FAXML_SCHEMA DATADIR "/libefa/faxml.xsd"

/*
 * frd_t:
 * Data structure for faxml_read() data.
 */
typedef struct frd_t {
  const char *schema_file_path;
#if HAVE_MODULE_HASH
  const char *function;
#endif
} frd_t;

/*
 * fwd_t:
 * Data structure for faxml_write() data.
 */
typedef struct fwd_t {
  int compression;
  xmlTextWriterPtr writer;
} fwd_t;

/*
 * faxml_automaton_attributes:
 * Array with FAXML automaton attributes.
 */
extern const xmlChar *const faxml_automaton_attributes[];

/*
 * faxml_automaton_attibute:
 * Enumeration of all available FAXML automaton attributes.
 */
enum faxml_automaton_attribute {
  FAXML_AUTOMATON_CLASS,
  FAXML_AUTOMATON_MINIMAL,
  FAXML_AUTOMATON_NAME,
  FAXML_AUTOMATON_TRANS_RE,
  FAXML_AUTOMATON_TYPE,
  FAXML_AUTOMATON_NB
};

/*
 * faxml_faxml_attributes:
 * Array with FAXML automaton attributes.
 */
extern const xmlChar *const faxml_faxml_attributes[];

/*
 * faxml_faxml_attribute:
 * Enumeration of all available FAXML faxml attributes.
 */
enum faxml_faxml_attribute {
  FAXML_FAXML_VERSION,
  FAXML_FAXML_NB
};

/*
 * faxml_node_names:
 * Array with FAXML node names.
 */
extern const xmlChar *const faxml_node_names[];

/*
 * faxml_node_name:
 * Enumeration of all available FAXML node names.
 */
enum faxml_node_name {
  FAXML_FAXML,
  FAXML_AUTOMATON,
  FAXML_REGEXP,
  FAXML_ALPHABET,
  FAXML_STATES,
  FAXML_STATE,
  FAXML_TRANSITIONS,
  FAXML_TRANSITION,
  FAXML_NB
};

/*
 * faxml_state_attributes:
 * Array with FAXML state attributes.
 */
extern const xmlChar *const faxml_state_attributes[];

/*
 * faxml_state_attribute:
 * Enumeration of all available FAXML state attributes.
 */
enum faxml_state_attribute {
  FAXML_STATE_ACCEPTING,
  FAXML_STATE_INITIAL,
  FAXML_STATE_NAME,
  FAXML_STATE_NB
};

/*
 * faxml_transition_attributes:
 * Array with FAXML transition attributes.
 */
extern const xmlChar *const faxml_transition_attributes[];

/*
 * faxml_transition_attribute:
 * Enumeration of all available FAXML transition attributes.
 */
enum faxml_transition_attribute {
  FAXML_TRANSITION_LABEL,
  FAXML_TRANSITION_TO,
  FAXML_TRANSITION_NB
};

#if HAVE_MODULE_HASH
/*
 * faxml_hash_write:
 * Writes an automaton in hash data type to a XML file in FAXML format.
 */
int faxml_hash_write(fa_t *, const char *, unsigned char, void *);
#endif

/*
 * faxml_initialize:
 * Initializes the pre parser and parser data.
 */
int faxml_initialize(fa_t *, const char *);

/*
 * faxml_preparse:
 * Preparse the automaton to decrease the memory usage.
 */
int faxml_preparse(fa_t *);

/*
 * faxml_preparse_process_node:
 * Preparse a given automaton by processing its node.
 */
int faxml_preparse_process_node(xmlTextReaderPtr);

#if HAVE_MODULE_RBTREE
/*
 * faxml_rbtree_write:
 * Writes an automaton in red-black-tree data type to a XML file in FAXML format.
 */
int faxml_rbtree_write(fa_t *, const char *, unsigned char, void *);
#endif

/*
 * faxml_validate:
 * Validate a XML Schema.
 */
int faxml_validate(const char *, unsigned char);

/*
 * faxml_validate_parse:
 * Validate and parse a XML file in faxml format with the SAX2 parser. The user
 * has to provide a pointer to an empty linked list of automata.
 */
int faxml_validate_parse(fa_t *, const char *, unsigned char, void *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FAXML_H_ */
