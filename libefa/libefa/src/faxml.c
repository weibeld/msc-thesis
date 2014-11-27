/*
 * faxml.c: import and export automata in faxml format
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

/*
 * Debug code of the SAX2 callback functions was taken from libxml2 xmllint
 * tool written by Daniel Veillard <daniel@veillard.com> and adapted to the
 * EFA library.
 */

#include "config.h"

#include "libefa/errno.h"
#include "libefa/error.h"
#include "libefa/faxml.h"
#include "libefa/list.h"
#include "libefa/memory.h"
#include <libxml/SAX2.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlschemastypes.h>

#if HAVE_MODULE_HASH
#include "libefa/hash.h"
#endif

#if HAVE_MODULE_RBTREE
#include "libefa/rbtree.h"
#endif

#include <assert.h>
#include <ctype.h>
#include <string.h>

#if DEBUG
static const char *context_str = "ctx";
static const char *null_str = "NULL";
#endif

const xmlChar *const faxml_automaton_attributes[] = {
  (const xmlChar *)"class",
  (const xmlChar *)"minimal",
  (const xmlChar *)"name",
  (const xmlChar *)"trans_re",
  (const xmlChar *)"type",
  NULL
};

const xmlChar *const faxml_faxml_attributes[] = {
  (const xmlChar *)"version",
  NULL
};

const xmlChar *const faxml_node_names[] = {
  (const xmlChar *)"faxml",
  (const xmlChar *)"automaton",
  (const xmlChar *)"regexp",
  (const xmlChar *)"alphabet",
  (const xmlChar *)"states",
  (const xmlChar *)"state",
  (const xmlChar *)"transitions",
  (const xmlChar *)"transition",
  NULL
};

const xmlChar *const faxml_state_attributes[] = {
  (const xmlChar *)"accepting",
  (const xmlChar *)"initial",
  (const xmlChar *)"name",
  NULL
};

const xmlChar *const faxml_transition_attributes[] = {
  (const xmlChar *)"label",
  (const xmlChar *)"to",
  NULL
};

typedef struct xmlSAX2ParserState {
  const char *xml_file_path;
  fa_t *automaton;
  void *data_structure;
  void *current_state;
  unsigned int current_automaton;
  unsigned int next_automaton;
  struct TransitionRow *list;
  unsigned int nfa : 1;
  unsigned int initialized : 1;
  unsigned int preparsed : 1;
  unsigned int validated : 1;
  unsigned int hasAccepting : 1;
  unsigned int hasInitial : 1;
  unsigned int isAlphabet : 1;
  unsigned int next : 1;
  unsigned int read : 1;
  unsigned int skip : 1;
  unsigned int error : 1;
} xmlSAX2ParserState;

static xmlSAX2ParserState parser_data;

typedef struct TransitionRow {
  struct TransitionRow *next;
  int *transitions;
} TransitionRow;

typedef struct xmlSAX2PreParserState {
  const char *xml_file_path;
  unsigned int current_automaton;
  unsigned int next_automaton;
  unsigned int nb_states;
  unsigned int nb_symbols;
  const char *alphabet;
  struct TransitionRow *list;
  unsigned int skip : 1;
  unsigned int error : 1;
} xmlSAX2PreParserState;

static xmlSAX2PreParserState pre_parser_data;

static int copy_xml_attribute_value(const char **, const xmlChar **);
static int trim(const char **, int);

#if HAVE_MODULE_HASH
static void charactersHash(void *, const xmlChar *, int);
static void endElementNsHash(void *, const xmlChar *, const xmlChar *,
			     const xmlChar *);
static void startElementNsHash(void *, const xmlChar *, const xmlChar *,
			       const xmlChar *, int, const xmlChar **, int,
			       int, const xmlChar **);
#endif

#if HAVE_MODULE_RBTREE
static void charactersRbtree(void *, const xmlChar *, int);
static void endElementNsRbtree(void *, const xmlChar *, const xmlChar *,
			       const xmlChar *);
static void startElementNsRbtree(void *, const xmlChar *, const xmlChar *,
				 const xmlChar *, int, const xmlChar **, int,
				 int, const xmlChar **);
#endif

#if DEBUG
static void attributeDeclDebug(void *, const xmlChar *, const xmlChar *,
			       int, int, const xmlChar *, xmlEnumerationPtr);
static void cdataBlockDebug(void *, const xmlChar *, int);
static void charactersDebug(void *, const xmlChar *, int);
static void commentDebug(void *, const xmlChar *);
static void elementDeclDebug(void *, const xmlChar *, int,
			     xmlElementContentPtr);
static void endDocumentDebug(void *);
static void endElementNsDebug(void *, const xmlChar *, const xmlChar *,
			      const xmlChar *);
static void entityDeclDebug(void *, const xmlChar *, int, const xmlChar *,
			    const xmlChar *, xmlChar *);
static void errorDebug(void *, const char *, ...);
static void externalSubsetDebug(void *, const xmlChar *, const xmlChar *,
				const xmlChar *);
static void fatalErrorDebug(void *, const char *, ...);
static xmlEntityPtr getEntityDebug(void *, const xmlChar *);
static xmlEntityPtr getParameterEntityDebug(void *, const xmlChar *);
static int hasExternalSubsetDebug(void *);
static int hasInternalSubsetDebug(void *);
static void ignorableWhitespaceDebug(void *, const xmlChar *, int);
static void internalSubsetDebug(void *, const xmlChar *, const xmlChar *,
				const xmlChar *);
static int isStandaloneDebug(void *);
static void notationDeclDebug(void *, const xmlChar *, const xmlChar *,
			      const xmlChar *);
static void processingInstructionDebug(void *, const xmlChar *,
				       const xmlChar *);
static void referenceDebug(void *, const xmlChar *);
static xmlParserInputPtr resolveEntityDebug(void *, const xmlChar *,
					    const xmlChar *);
static void setDocumentLocatorDebug(void *, xmlSAXLocatorPtr);

static void startDocumentDebug(void *);
static void startElementNsDebug(void *, const xmlChar *, const xmlChar *,
				const xmlChar *, int, const xmlChar **, int,
				int, const xmlChar **);
static void unparsedEntityDeclDebug(void *, const xmlChar *, const xmlChar *,
				    const xmlChar *, const xmlChar *);
static void warningDebug(void *, const char *, ...);
#endif

#if HAVE_MODULE_HASH
static xmlSAXHandler sax2_hash_handler = {
    NULL,		// internalSubset
    NULL,		// isStandalone
    NULL,		// hasInternalSubset
    NULL,		// hasExternalSubset
    NULL,		// resolveEntity
    NULL,		// getEntity
    NULL,		// entityDecl
    NULL,		// notationDecl
    NULL,		// attributeDecl
    NULL,		// elementDecl
    NULL,		// unparsedEntityDecl
    NULL,		// setDocumentLocator
    NULL,		// startDocument
    NULL,		// endDocument
    NULL,		// startElement
    NULL,		// endElement
    NULL,		// reference
    charactersHash,	// characters
    NULL,		// ignorableWhitespace
    NULL,		// processingInstruction
    NULL,		// comment
    NULL,		// xmlParserWarning
    NULL,		// xmlParserError
    NULL,		// xmlParserError
    NULL,		// getParameterEntity
    NULL,		// cdataBlock
    NULL,		// externalSubset
    XML_SAX2_MAGIC,	// initialized
    NULL,		// _private
    startElementNsHash,	// startElementNs
    endElementNsHash,	// endElementNs
    NULL		// serror
};

#endif

#if HAVE_MODULE_RBTREE
static xmlSAXHandler sax2_rbtree_handler = {
    NULL,			// internalSubset
    NULL,			// isStandalone
    NULL,			// hasInternalSubset
    NULL,			// hasExternalSubset
    NULL,			// resolveEntity
    NULL,			// getEntity
    NULL,			// entityDecl
    NULL,			// notationDecl
    NULL,			// attributeDecl
    NULL,			// elementDecl
    NULL,			// unparsedEntityDecl
    NULL,			// setDocumentLocator
    NULL,			// startDocument
    NULL,			// endDocument
    NULL,			// startElement
    NULL,			// endElement
    NULL,			// reference
    charactersRbtree,		// characters
    NULL,			// ignorableWhitespace
    NULL,			// processingInstruction
    NULL,			// comment
    NULL,			// xmlParserWarning
    NULL,			// xmlParserError
    NULL,			// xmlParserError
    NULL,			// getParameterEntity
    NULL,			// cdataBlock
    NULL,			// externalSubset
    XML_SAX2_MAGIC,		// initialized
    NULL,			// _private
    startElementNsRbtree,	// startElementNs
    endElementNsRbtree,		// endElementNs
    NULL			// serror
};
#endif

#if DEBUG

/*
 * sax_handler_debug:
 * Used to check the behavior of the SAX2 parser.
 */
static xmlSAXHandler sax2_handler_debug = {
    internalSubsetDebug,
    isStandaloneDebug,
    hasInternalSubsetDebug,
    hasExternalSubsetDebug,
    resolveEntityDebug,
    getEntityDebug,
    entityDeclDebug,
    notationDeclDebug,
    attributeDeclDebug,
    elementDeclDebug,
    unparsedEntityDeclDebug,
    setDocumentLocatorDebug,
    startDocumentDebug,
    endDocumentDebug,
    NULL,
    NULL,
    referenceDebug,
    charactersDebug,
    ignorableWhitespaceDebug,
    processingInstructionDebug,
    commentDebug,
    warningDebug,
    errorDebug,
    fatalErrorDebug,
    getParameterEntityDebug,
    cdataBlockDebug,
    externalSubsetDebug,
    XML_SAX2_MAGIC,
    NULL,
    startElementNsDebug,
    endElementNsDebug,
    NULL
};
#endif

/*
 * copy_xml_attribute_value:
 * Copys a XML attribute value to a variable by copying its string value 
 * pointed to in the XML document.
 * NOTE: the attribute value can be found at attr[3] and the value end at
 * attr[4].
 * Returns -1 for internal error, 0 for no error and 1 for error.
 */
static int copy_xml_attribute_value(const char **copy, const xmlChar **attr) {
  const char *attribute_value = NULL;
  char *xml_attribute_value = NULL;
  int ret = 0, str_size = 0;
  
  // compute the length of the attribute value
  str_size = attr[4] - attr[3];
  
  ret = efa_alloc(&xml_attribute_value, sizeof(char), str_size + 1);
  if (ret == -1) {
    efa_error(EFA_MOD_FAXML, __LINE__, __FILE__);
    return 1;
  }
  
  attribute_value = (const char *)attr[3];
  
  strncpy(xml_attribute_value, attribute_value, (size_t) str_size);
  
  // null-terminate the string as strncpy does not do it
  xml_attribute_value[str_size] = '\0';
  
  *copy = (const char *)xml_attribute_value;
  xml_attribute_value = NULL;
  
  return 0;
}

#if DEBUG

/*
 * attributeDeclDebug:
 * Callback function used if an attribute definition has been parsed.
 */
static void attributeDeclDebug(void *ctx, const xmlChar *elem,
			       const xmlChar *name, int type, int def,
			       const xmlChar *defaultValue,
			       xmlEnumerationPtr tree) {
  const char *context;
  const char *defVal;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;

  if (defaultValue)
    defVal = (const char *)defaultValue;
  else
    defVal = null_str;
  
  fprintf(stdout, " attributeDecl(%s, %s, %s, %d, %d, %s, ...)\n", context,
	  (const char *)elem, (const char *)name, type, def, defVal);

  xmlFreeEnumeration(tree);
}

#endif

#if DEBUG

/*
 * cdataBlockDebug:
 * Callback function used when a pcdata block has been parsed.
 */
static void cdataBlockDebug(void *ctx, const xmlChar *value, int len) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " pcdata(%s, %s, %d)\n", context, (const char *)value, len);
}

#endif

#if DEBUG

/*
 * charactersDebug:
 * Callback function used if some chars are recieved from the parser.
 */
static void charactersDebug(void *ctx, const xmlChar *ch, int len) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;

  fprintf(stdout, " characters(%s, %.10s, %d)\n", context, (const char *)ch, len);
}

#endif

#if HAVE_MODULE_HASH

/*
 * charactersHash:
 * Callback function which is called from the SAX2 parser when it reads
 * characters inside and outside of tags.
 */
static void charactersHash(void *ctx, const xmlChar *ch, int len) {
  char *buffer;
  int ret = 0, str_size = 0;
  const xmlChar *char_ptr = NULL;
  xmlChar bra = (xmlChar) '<';
  xmlSAX2ParserState *parser_state = (xmlSAX2ParserState *)ctx;
  
  if (parser_state->isAlphabet && !parser_state->error &&
      (parser_state->automaton->alphabet == NULL)) {
    char_ptr = xmlStrchr(ch, bra);
    str_size = char_ptr - ch;
  
    ret = efa_alloc(&buffer, sizeof(char), str_size + 1);
    if (ret == -1) {
      parser_state->error = 1;
      efa_error(EFA_MOD_FAXML, __LINE__, __FILE__);
      return;
    }
  
    strncpy(buffer, (const char *)ch, str_size);
  
    // null-terminate the string as strncpy does not do it
    buffer[str_size] = '\0';
	 
    ret = trim((const char **)&buffer, len);
    if (ret == -1) {
      parser_state->error = 1;
      fprintf(stderr, "Not enough memory to allocate a new string.\n");
    }
  
    parser_state->automaton->nb_symbols = ret;
    parser_state->automaton->alphabet = (const char *)buffer;
    buffer = NULL;
  }
}

#endif

#if DEBUG

/*
 * commentDebug:
 * Callback function used if a comment has been parsed.
 */
static void commentDebug(void *ctx, const xmlChar *value) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " comment(%s, %s)\n", context, (char *)value);
}

#endif

#if DEBUG

/*
 * elementDeclDebug:
 * Callback function used if an element definition has been parsed.
 */
static void elementDeclDebug(void *ctx, const xmlChar *name, int type,
			     xmlElementContentPtr content) {
  const char *context;
  const char *cont;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  if (content)
    cont = "content";
  else
    cont = null_str;
  
  fprintf(stdout, " elementDecl(%s, %s, %d, %s)\n", context,
	  (const char *)name, type, cont);
}

#endif

#if DEBUG

/*
 * endDocumentDebug:
 * Callback function used when the document end has been detected.
 */
static void endDocumentDebug(void *ctx) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " endDocument(%s)\n", context);
}

#endif

#if DEBUG

/*
 * endElementDebug:
 * Callback function used when the end of an element has been detected.
 */
static void endElementNsDebug(void *ctx, const xmlChar *localname,
			      const xmlChar *prefix, const xmlChar *URI) {
  const char *context;
  const char *pref;
  const char *uri;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  if (prefix)
    pref = (const char *)prefix;
  else
    pref = null_str;

  if (URI)
    uri = (const char *)URI;
  else
    uri = null_str;
  
  fprintf(stdout, " endElementNs(%s, %s, %s, %s)\n", context, (char *)localname,
	  pref, uri);
}

#endif

#if HAVE_MODULE_HASH

/*
 * endElementNsHash:
 * Callback function used when the end of an element has been detected.
 */
static void endElementNsHash(void *ctx, const xmlChar *localname,
			     const xmlChar *prefix __attribute__ ((unused)),
			     const xmlChar *URI __attribute__ ((unused))) {
  int i, ret = 0;
  TransitionRow *temp_tr = NULL;
  xmlSAX2ParserState *parser_state = NULL;

  parser_state = (xmlSAX2ParserState *)ctx;
  
  if (parser_state->error)
    return;
  
  for (i = 0; i < FAXML_NB; i++) {
    ret = xmlStrncasecmp(localname, faxml_node_names[i], 11);
    if (ret == 0)
      break;
  }
  if (i == FAXML_NB) {
    fprintf(stderr, " [FAXML] Wrong or incorrect FAXML node name '%s'.\n",
	    localname);
    parser_state->error = 1;
    return;
  }

  if ((i != FAXML_FAXML) && (i != FAXML_AUTOMATON) && parser_state->skip)
    return;
  
  switch (i) {
  case FAXML_ALPHABET:
    // reset the isAlphabet field in the parser state
    parser_state->isAlphabet = 0;		
    break;
  case FAXML_AUTOMATON:
    assert(parser_state->current_automaton > 0);

    // automaton that needed to be read is now in memeory -> switch on skipping
    if ((parser_state->current_automaton == parser_state->next_automaton) &&
	(parser_state->skip == 0)) {
      parser_state->current_automaton = 0;
      parser_state->next_automaton++;
      parser_state->next = 1;
      parser_state->read = 1;
      parser_state->skip = 1;
    }
    
    break;
  case FAXML_FAXML:
    // check if all automata have been read from the file
    if (parser_state->next && parser_state->read && parser_state->skip)
      parser_state->next_automaton = 0;
    else
      parser_state->read = 0;

    break;
  case FAXML_TRANSITIONS:
    // free the memory used by transition list element
    temp_tr = parser_state->list;
    if (parser_state->list->next != NULL)
      parser_state->list = parser_state->list->next;
    else
      parser_state->list = NULL;
    efa_free(temp_tr->transitions);
    efa_free(temp_tr);
    break;
  default:
    break;
  }
}

#endif

#if DEBUG

/*
 * entityDeclDebug:
 * Callback function used  if an entity definition has been parsed.
 */
static void entityDeclDebug(void *ctx, const xmlChar *name, int type,
			    const xmlChar *publicId, const xmlChar *systemId,
			    xmlChar *content) {
  const char *context;
  const char *pubId;
  const char *sysId;
  const char *cont;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  if (publicId)
    pubId = (const char *)publicId;
  else
    pubId = null_str;
  
  if (systemId)
    sysId = (const char *)systemId;
  else
    sysId = null_str;
  
  if (content)
    cont = (const char *)content;
  else
    cont = null_str;

  fprintf(stdout, " entityDecl(%s, %s, %d, %s, %s, %s)\n", context,
	  (const char *)name, type, pubId, sysId, cont);
}

#endif

#if DEBUG

/*
 * errorDebug:
 * Callback function used to display and format a error messages.
 */
static void errorDebug(void *ctx, const char *msg, ...) {
  const char *context;
  va_list args;

  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  va_start(args, msg);
  fprintf(stdout, " [FAXML] error(%s): ", context);
  vfprintf(stdout, msg, args);
  va_end(args);
}

#endif

#if DEBUG

/*
 * externalSubsetDebug:
 * Callback function used if the XML file has an external subset.
 */
static void externalSubsetDebug(void *ctx, const xmlChar *name,
				const xmlChar *ExternalID,
				const xmlChar *SystemID) {
  const char *context;
  const char *extID;
  const char *sysID;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  if (ExternalID)
    extID = (const char *)ExternalID;
  else
    extID = null_str;
  
  if (SystemID)
    sysID = (const char *)SystemID;
  else
    sysID = null_str;
  
  fprintf(stdout, " externalSubset(%s, %s, %s, %s)\n", context,
	  (const char *)name, extID, sysID);
}

#endif

#if DEBUG

/*
 * fatalErrorDebug:
 * Callback function used to display and format a fatalError messages.
 */
static void fatalErrorDebug(void *ctx, const char *msg, ...) {
  const char *context;
  va_list args;

  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  va_start(args, msg);
  fprintf(stdout, " fatalError(%s): ", context);
  vfprintf(stdout, msg, args);
  va_end(args);
}

#endif

#if DEBUG

/*
 * getEntityDebug:
 * Callback function used to get an entity by name.
 */
static xmlEntityPtr getEntityDebug(void *ctx, const xmlChar *name) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " getEntity(%s, %s)\n", context, (const char *)name);
  
  return NULL;
}

#endif

#if DEBUG

/*
 * getParameterEntityDebug:
 * Callback function used to get a parameter entity by name.
 */
static xmlEntityPtr getParameterEntityDebug(void *ctx, const xmlChar *name) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " getParameterEntity(%s, %s)\n", context,
	  (const char *)name);
  
  return NULL;
}

#endif

#if DEBUG

/*
 * hasExternalSubsetDebug:
 * Callback function used if the XML file has an external subset.
 */
static int hasExternalSubsetDebug(void *ctx) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " hasExternalSubset(%s)\n", context);
  
  return 0;
}

#endif

#if DEBUG

/*
 * hasInternalSubsetDebug:
 * Callback function used if XML file has an internal subset.
 */
static int hasInternalSubsetDebug(void *ctx) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " hasInternalSubset(%s)\n", context);
  
  return 0;
}

#endif

#if DEBUG

/*
 * ignorableWhitespaceDebug:
 * Callback function used if some ignorable whitespaces is recieved from the
 * parser.
 */
static void ignorableWhitespaceDebug(void *ctx, const xmlChar *ch, int len) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " ignorableWhitespace(%s, %s, %d)\n", context,
	  (const char *)ch, len);
}

#endif

#if DEBUG

/*
 * internalSubsetDebug:
 * Callback function used if the XML file has an internal subset.
 */
static void internalSubsetDebug(void *ctx, const xmlChar *name,
				const xmlChar *ExternalID,
				const xmlChar *SystemID) {
  const char *context;
  const char *extID;
  const char *sysID;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  if (ExternalID)
    extID = (const char *)ExternalID;
  else
    extID = null_str;
  
  if (SystemID)
    sysID = (const char *)SystemID;
  else
    sysID = null_str;
  
  fprintf(stdout, " internalSubset(%s, %s, %s, %s)\n", context,
	  (const char *)name, extID, sysID);
}

#endif

#if DEBUG

/*
 * isStandaloneDebug:
 * Callback function used if XML file is tagged standalone.
 */
static int isStandaloneDebug(void *ctx) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " isStandalone(%s)\n", context);
  
  return 0;
}

#endif

#if DEBUG

/*
 * notationDeclDebug:
 * Callback function used when a notation declaration has been parsed.
 */
static void notationDeclDebug(void *ctx, const xmlChar *name,
			      const xmlChar *publicId,
			      const xmlChar *systemId) {
  const char *context;
  const char *pubId;
  const char *sysId;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  if (publicId)
    pubId = (const char *)publicId;
  else
    pubId = null_str;
  
  if (systemId)
    sysId = (const char *)systemId;
  else
    sysId = null_str;
  
  fprintf(stdout, " notationDecl(%s, %s, %s, %s)\n", context,
	  (const char *)name, pubId, sysId);
}

#endif

#if DEBUG

/*
 * processingInstructionDebug:
 * Callback function used if a processing instruction has been parsed.
 */
static void processingInstructionDebug(void *ctx, const xmlChar *target,
				       const xmlChar *data) {
  const char *context;
  const char *dat;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  if (data)
    dat = (const char *)data;
  else
    dat = null_str;
  
  fprintf(stdout, " processingInstruction(%s, %s, %s)\n", context,
	  (const char *)target, dat);
}

#endif

#if DEBUG

/*
 * referenceDebug:
 * Callback function used when an entity reference is detected.
 */
static void referenceDebug(void *ctx, const xmlChar *name) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " reference(%s, %s)\n", context, (const char *)name);
}

#endif

#if DEBUG

/*
 * resolveEntityDebug:
 * Callback function used to resolve entities in the XML file.
 */
static xmlParserInputPtr resolveEntityDebug(void *ctx, const xmlChar *publicId,
					    const xmlChar *systemId) {
  const char *context;
  const char *pubId;
  const char *sysId;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  if (publicId)
    pubId = (const char *)publicId;
  else
    pubId = null_str;
  
  if (systemId)
    sysId = (const char *)systemId;
  else
    sysId = null_str;
  
  fprintf(stdout, " resolveEntity(%s, %s, %s)\n", context, pubId, sysId);
  
  return NULL;
}

#endif

#if DEBUG

/*
 * setDocumentLocatorDebug:
 * Callback function used to receive the document locator at startup.
 */
static void setDocumentLocatorDebug(void *ctx, xmlSAXLocatorPtr loc) {
  const char *context;
  const char *locator;
  const char *locator_str = "loc";
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  if (loc)
    locator = locator_str;
  else
    locator = null_str;
  
  fprintf(stdout, " setDocumentLocator(%s, %s)\n", context, locator);
}

#endif

#if DEBUG

/*
 * startDocumentDebug:
 * Callback function used when the document start being processed.
 */
static void startDocumentDebug(void *ctx) {
  const char *context;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  fprintf(stdout, " startDocument(%s)\n", context);
}

#endif

#if DEBUG

/*
 * startElementNsDebug:
 * Callback function used when an opening tag has been processed.
 */
static void startElementNsDebug(void *ctx, const xmlChar *localname,
				const xmlChar *prefix, const xmlChar *URI,
				int nb_namespaces, const xmlChar **namespaces,
				int nb_attributes, int nb_defaulted,
				const xmlChar **attributes) {
  const char *context;
  const char *pref;
  const char *uri;
  int i;

  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  if (prefix)
    pref = (const char *)prefix;
  else
    pref = null_str;
  
  if (URI)
    uri = (const char *)URI;
  else
    uri = null_str;
  
  fprintf(stdout, " startElementNs(%s, %s, %s, %s, %d", context,
	  (const char *)localname, pref, uri, nb_namespaces);

  if (namespaces != NULL) {
    for (i = 0;i < nb_namespaces * 2;i++) {
      fprintf(stdout, ", xmlns");
      if (namespaces[i] != NULL)
	fprintf(stdout, ":%s", namespaces[i]);
      i++;
      fprintf(stdout, "='%s'", namespaces[i]);
    }
  }
  fprintf(stdout, ", %d, %d", nb_attributes, nb_defaulted);
  if (attributes != NULL) {
    for (i = 0;i < nb_attributes * 5;i += 5) {
      if (attributes[i + 1] != NULL)
	fprintf(stdout, ", %s:%s='", attributes[i + 1], attributes[i]);
      else
	fprintf(stdout, ", %s='", attributes[i]);
      fprintf(stdout, "%.4s...', %d", attributes[i + 3],
	      (int)(attributes[i + 4] - attributes[i + 3]));
    }
  }
  fprintf(stdout, ")\n");
}

#endif

#if HAVE_MODULE_HASH

/*
 * startElementNsHash:
 * Callback function used when an opening tag has been processed. This function
 * does essentially contibute to the internal representation of the automaton.
 */
static void startElementNsHash(void *ctx, const xmlChar *localname,
			       const xmlChar *prefix __attribute__ ((unused)),
			       const xmlChar *URI __attribute__ ((unused)),
			       int nb_namespaces __attribute__ ((unused)),
			       const xmlChar **namespaces __attribute__ ((unused)),
			       int nb_attributes,
			       int nb_defaulted __attribute__ ((unused)),
			       const xmlChar **attributes) {
  char *pos = NULL;
  const char *state_label = NULL, *transition_label = NULL;
  const char *xml_attribute_value = NULL;
  int i, j, k, l, idx = 0, len = 0, ret = 0;
  const char *true_str = "true";
  hnode_t *nstate = NULL, *node = NULL;
  state_t *state;
  xmlSAX2ParserState *parser_state = NULL;


  parser_state = (xmlSAX2ParserState *)ctx;

  if (parser_state->error)
    return;
  
  // get the enumeration value of the FAXML element
  for (i = 0; i < FAXML_NB; i++) {
    ret = xmlStrncasecmp(localname, faxml_node_names[i], 11);
    if (ret == 0)
      break;
  }

  // do not parse if skip is switched on except for AUTOMATON elements
  if (((i != FAXML_AUTOMATON) && parser_state->skip) ||
      (!parser_state->next && parser_state->read && parser_state->skip))
    return;
  
  switch (i) {
  case FAXML_FAXML:
    parser_state->current_automaton = 0;

    assert(parser_state->current_automaton == 0);
    assert(parser_state->next_automaton > 0);
    assert(parser_state->next == 0);
    assert(parser_state->read == 0);
    break;
  case FAXML_AUTOMATON:
    parser_state->current_automaton++;

    assert(parser_state->current_automaton <= parser_state->next_automaton);

    /* check if there is another automaton comming after the automaton that was
       read                                                                   */
    if (parser_state->next && parser_state->read && parser_state->skip) {
      parser_state->next = 0;
      return;
    }

    /* switch off skipping if c_a equals n_a and next as well as read flags are
       not set                                                                */
    if ((parser_state->current_automaton == parser_state->next_automaton)
	&& !parser_state->next && !parser_state->read &&
	parser_state->skip)
      parser_state->skip = 0;

    // assign the attributes to the Automaton structure
    /* NOTE: the attributes array has the following structure per
     * attribute:
     * localname / prefix / URI / value / end
     * In order to parse the attributes only the localname
     * (attributes[i]) and the value (attributes[i + 3] are needed.
     */
    if (attributes != NULL) {
      
      // loop through all attributes
      for (j = 0; j < nb_attributes * 5; j += 5) {
			 
	// get the enumeration of the FAXML AUTOMATON attribute
	for (k = 0; k < FAXML_AUTOMATON_NB; k++) {
	  ret = xmlStrncasecmp(attributes[j], faxml_automaton_attributes[k], 7);
	  if (ret == 0)
	    break;
	}
			 
	switch (k) {
	case FAXML_AUTOMATON_CLASS:
	  // get the enumeration of the automaton class
	  ret = copy_xml_attribute_value(&xml_attribute_value,
					 &attributes[j]);
	  if (ret != 0)
	    goto error;
	  
	  for (l = 0; l < FA_CLASS_NB; l++) {
	    ret = strncasecmp(xml_attribute_value,
			      efa_fa_class_names[l], FA_CLASS_LEN);
	    if (ret == 0)
	      break;
	  }
	  
	  /* free up the temporary allocated memory of the copied XML
	     attribute value                                           */
	  free((void *)xml_attribute_value);
	  xml_attribute_value = NULL;
	  
	  // assign the enumeration value to the Automaton structure
	  parser_state->automaton->class = l;
	  
	  break;
	case FAXML_AUTOMATON_MINIMAL:
	  ret = copy_xml_attribute_value(&xml_attribute_value,
					 &attributes[j]);
	  if (ret != 0)
		   goto error;
	  
	  // value is eighter 'true' or 'false'
	  ret = strncasecmp(xml_attribute_value, true_str, 4);
	  if (ret == 0)
	    parser_state->automaton->minimal = 1;
	  else
	    parser_state->automaton->minimal = 0;
				  
	  /* free up the temporary allocated memory of the copied XML
	     attribute value                                           */
	  free((void *)xml_attribute_value);
	  xml_attribute_value = NULL;
	  
	  break;
	case FAXML_AUTOMATON_NAME:
	  ret = copy_xml_attribute_value(&parser_state->automaton->name,
					 &attributes[j]);
	  if (ret != 0)
	    goto error;
	  
	  break;
	case FAXML_AUTOMATON_TYPE:
	  // get the enumeration of the automaton type
	  copy_xml_attribute_value(&xml_attribute_value, &attributes[j]);
	  if (ret != 0)
	    goto error;
	  
	  for (l = 0; l < FA_TYPE_NB; l++) {
	    ret = strncasecmp(xml_attribute_value,
			      efa_fa_type_names[l], FA_TYPE_LEN);
	    if (ret == 0)
	      break;
	  }
	  
	  /* free up the temporary allocated memory of the copied XML
	     attribute value                                           */
	  free((void *)xml_attribute_value);
	  xml_attribute_value = NULL;
	  
	  // assign the enumeration value to the Automaton structure
	  parser_state->automaton->type = l;
	  
	  break;
	default:
	  fprintf(stderr, " [%s] Error while parsing FAXML AUTOMATON \
element '%s' of '%s'.\n", efa_module_names[EFA_MOD_FAXML], (const char 
*)attributes[j], parser_state->xml_file_path);
	  goto error;
	}
      }
    }
		
    break;
  case FAXML_REGEXP:
    break;
  case FAXML_ALPHABET:
    parser_state->isAlphabet = 1;
    break;
  case FAXML_STATES:
    break;
  case FAXML_STATE:
    // create a new hash node (equivalent of a state)
    ret = efa_alloc(&nstate, sizeof(hnode_t), 1);
    if (ret == -1)
      goto error;
    
    ret = efa_alloc(&state, sizeof(state_t), 1);
    if (ret == -1)
      goto error;
    
    nstate->data = (void *)state;
		
    /* NOTE: initialisation of the hash node structure is not needed as ALLOC
     * zero-allocates a new memory area                                       */
		
    // assign the attributes to the hash node structure
    if (attributes != NULL) {
		  
      // loop through all attributes
      for (j = 0; j < nb_attributes * 5; j += 5) {
			 
	// get the enumeration value of the FAXML STATE attribute
	for (k = 0; k < FAXML_STATE_NB; k++) {
	  ret = xmlStrncasecmp(attributes[j], faxml_state_attributes[k], 7);
	  if (ret == 0)
	    break;
	}
			 
	switch (k) {
	case FAXML_STATE_ACCEPTING:
	  // set the accepting field in the state structure
	  state->accept = 1;
				  
	  break;
	case FAXML_STATE_INITIAL:
	  ret = copy_xml_attribute_value(&xml_attribute_value,
					 &attributes[j]);
	  if (ret != 0)
	    goto error;
				  
	  // value is eighter 'true' or 'false'
	  ret = strncasecmp(xml_attribute_value, true_str, 4);
	  if (ret == 0) {
	    // set the hasInitial variable
	    parser_state->hasInitial = 1;
	    // assign the initial state to the current automaton
	    ((hash_t *)parser_state->data_structure)->initial = nstate;
	    // set its 'reachable' attribute
	    state->reachable = 1;
	  }
				  
	  /* free up the temporary allocated memory of the copied XML
	     attribute value in                                             */
	  free((void *)xml_attribute_value);
	  xml_attribute_value = NULL;
	
	  break;
	case FAXML_STATE_NAME:
	  ret = copy_xml_attribute_value(&nstate->key,
					 &attributes[j]);
	  if (ret != 0)
	    goto error;
	  
	  state->label = nstate->key;
	    
	  break;
	default:
	  fprintf(stderr, " [%s] Error while parsing FAXML STATE \
element '%s' of '%s'.\n", efa_module_names[EFA_MOD_FAXML], attributes[j],
parser_state->xml_file_path);
	  goto error;
	}
      }
    }

    node = hash_lookup((hash_t *)parser_state->data_structure, nstate->key);
    if ((node != NULL) && (((state_t *)node->data)->reachable == 0)) {
      fprintf(stderr, " [%s] Node '%s' is not unique. Found another node in \
the hash table with same key '%s'.\n", 
efa_module_names[EFA_MOD_FAXML], nstate->key, node->key);
      parser_state->error = 1;
      return;
    } else if (node == NULL)
      hash_insert((hash_t *)parser_state->data_structure, nstate,
		  nstate->key);
    else if (node != NULL) {
      // copy read attributes to already inserted hash node
      ((state_t *)node->data)->accept |= state->accept;
      ((state_t *)node->data)->transition_row = state->transition_row;
      efa_free(state); // free the temporary state
      efa_free(nstate); // free the temporary hash node
      state = (state_t *)node->data;
      node = NULL;
    }

    parser_state->current_state = (void *)state;
    assert(state != NULL);
    break;
  case FAXML_TRANSITIONS:
    break;
  case FAXML_TRANSITION:
    // zero-allocate memory for a new transition row
    state = (state_t *)parser_state->current_state;
    if (parser_state->nfa && state->transition_row == NULL) { // NFA
      ret = efa_alloc(&state->transition_row, sizeof(hnode_t ***), 
		      (size_t) parser_state->automaton->nb_symbols);
      if (ret == -1)
	goto alloc_error;
    } else if (state->transition_row == NULL) { // DFA
      ret = efa_alloc(&state->transition_row, sizeof(hnode_t **),
		      (size_t) parser_state->automaton->nb_symbols);
      if (ret == -1)
	goto alloc_error;
    }

    if (parser_state->nfa && ((hnode_t ***)state->transition_row)[0] == NULL) {
      // zero-allocate memory for each transition cell + 1 (NULL terminated)
      for (j = 0; j < parser_state->automaton->nb_symbols; j++) {
	ret = efa_alloc(&((hnode_t ***)state->transition_row)[j], 
sizeof(hnode_t **), (size_t) (parser_state->list->transitions[j] + 1));
	if (ret == -1)
	  goto alloc_error;
      }
    }
		
    // assign the attributes to the new Transition structure
    if (attributes != NULL) {
		  
      // loop through all attributes
      for (j = 0; j < nb_attributes * 5; j += 5) {
	
	// get the enumeration value of the FAXML TRANSITION attribute
	for (k = 0; k < FAXML_TRANSITION_NB; k++) {
	  ret = xmlStrncasecmp(attributes[j], faxml_transition_attributes[k],
			       5);
	  if (ret == 0)
	    break;
	}
			 
	switch (k) {
	case FAXML_TRANSITION_LABEL:
	  /* assign the transition label to the temporary buffer
	     'xml_attribute_value'                                            */
	  ret = copy_xml_attribute_value(&xml_attribute_value,
					 &attributes[j]);
	  if (ret != 0)
	    goto error;
	    
	  transition_label = xml_attribute_value;
	  xml_attribute_value = NULL;
	  break;
	case FAXML_TRANSITION_TO:
	  ret = copy_xml_attribute_value(&xml_attribute_value,
					 &attributes[j]);
	  if (ret != 0)
	    goto error;

	  state_label = xml_attribute_value;
	  xml_attribute_value = NULL;
				  
	  break;
	default:
	  fprintf(stderr, " [%s] Could not identify TRANSITION \
attribute '%s' for given transition.\n", efa_module_names[EFA_MOD_FAXML], 
(const char *)attributes[j]);
	  goto error;
	}
      }
    }
		
    // append the adjacent state to the transition row
    len = (int) strlen(transition_label);
    assert(len > 0);
    for (j = 0; j < len; j++) {
      k = 0;
      pos = strchr(parser_state->automaton->alphabet, (int) transition_label[j]);
      idx = pos - parser_state->automaton->alphabet;

      if (parser_state->nfa)
	for (k = 0; k < parser_state->list->transitions[idx]; k++)
	  if (((hnode_t ***)state->transition_row)[idx][k] == NULL)
	    break;

      node = hash_lookup((hash_t *)parser_state->data_structure,
			 state_label);
      if (node == NULL) {
	node = hash_alloc_insert((hash_t *)parser_state->data_structure,
				    state_label);
	if (node == NULL)
	  goto error;
	
	ret = efa_alloc(&node->data, sizeof(state_t), 1);
	if (ret == -1)
	  goto error;
	
	((state_t *)node->data)->label = state_label;
      }

      ((state_t *)node->data)->reachable = 1;

      if (parser_state->nfa)
	((hnode_t ***)state->transition_row)[idx][k] = node;
      else // DFA
	((hnode_t **)state->transition_row)[idx] = node;
    }
		
    // better set the local variables to NULL
    node = NULL;
    state = NULL;
		
    break;
  default:
    fprintf(stderr, " [%s] Error while parsing '%s'\n",
	    efa_module_names[EFA_MOD_FAXML], parser_state->xml_file_path);
    goto error;
  }
  
  return;
  
error:
  parser_state->error = 1;
  return;
  
alloc_error:
  efa_error(EFA_MOD_FAXML, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}

#endif

/*
 * trim:
 * Remove any space character (e.g. ' ', '\t', '\n' etc.) from the string STR
 * and store the trimmed string back in string STR given to this function.
 * The function returns the length of the trimed string STR.
 */
static int trim(const char **str, int len) {
  char *trimed;
  int i;
  int j;
  
  i = efa_alloc(&trimed, sizeof(char), len);
  if (i == -1)
    return i;
  
  for (i = 0, j = 0; i < len; i++) {
    if (!isspace((int) (*str)[i]))
      trimed[j++] = (*str)[i];
  }
  
  trimed[j] = '\0';
  *str = (const char *)trimed;
  trimed = NULL;
  
  return j;
}

#if DEBUG

/*
 * unparsedEntityDeclDebug:
 * Callback function used when an unparsed entity declaration is parsed.
 */
static void unparsedEntityDeclDebug(void *ctx, const xmlChar *name,
				    const xmlChar *publicId,
				    const xmlChar *systemId,
				    const xmlChar *notationName) {
  const char *context;
  const char *pubId;
  const char *sysId;
  const char *notName;
  
  if (ctx)
    context = context_str;
  else
    context = null_str;

  if (publicId)
    pubId = (const char *)publicId;
  else
    pubId = null_str;
  
  if (systemId)
    sysId = (const char *)systemId;
  else
    sysId = null_str;
  
  if (notationName)
    notName = (const char *)notationName;
  else
    notName = null_str;
  
  fprintf(stdout, " unparsedEntityDecl(%s, %s, %s, %s, %s)\n", context,
	  (const char *)name, pubId, sysId, notName);
}

#endif

#if DEBUG

/*
 * warningDebug:
 * Callback function used to display and format a warning messages.
 */
static void warningDebug(void *ctx, const char *msg, ...) {
  const char *context;
  va_list args;

  if (ctx)
    context = context_str;
  else
    context = null_str;
  
  va_start(args, msg);
  fprintf(stdout, " [FAXML] warning(%s): ", context);
  vfprintf(stdout, msg, args);
  va_end(args);
}

#endif


int faxml_initialize(fa_t *automaton, const char *xml_file_path) {

  // initialize the pre parser data
  pre_parser_data.xml_file_path = xml_file_path;
  pre_parser_data.current_automaton = 0;
  pre_parser_data.next_automaton = 1;
  pre_parser_data.nb_states = 0;
  pre_parser_data.nb_symbols = 0;
  pre_parser_data.alphabet = NULL;
  pre_parser_data.list = NULL;
  pre_parser_data.skip = 0;
  pre_parser_data.error = 0;

  // initialize the parser data
  parser_data.xml_file_path = xml_file_path;
  parser_data.automaton = automaton;
  parser_data.data_structure = NULL;
  parser_data.current_state = NULL;
  parser_data.current_automaton = 0;
  parser_data.next_automaton = 1;
  parser_data.list = NULL;
  parser_data.nfa = 1;
  parser_data.initialized = 1;
  parser_data.preparsed = 0;
  parser_data.validated = 0;
  parser_data.hasAccepting = 0;
  parser_data.hasInitial = 0;
  parser_data.isAlphabet = 0;
  parser_data.next = 0;
  parser_data.read = 0;
  parser_data.skip = 0;
  parser_data.error = 0;

  return 0;
}

int faxml_preparse(fa_t *automaton) {
  int i, ret = 0, ret2 = 0;
  xmlTextReaderPtr reader = NULL;

  reader = xmlReaderForFile(pre_parser_data.xml_file_path, NULL, 0);
  if (reader == NULL) {
    fprintf(stderr, " [FAXML] Error while creating a xmlReader for '%s' reading \
automaton '%d'.\n", pre_parser_data.xml_file_path,
	    pre_parser_data.next_automaton);
    return -1;
  }

  ret = xmlTextReaderRead(reader);
  if (ret == 0 || ret == -1) {
   fprintf(stderr, " [FAXML] Error while parsing '%s' for automaton '%d'.\n",
	    pre_parser_data.xml_file_path, pre_parser_data.next_automaton);
    return -1;
  }
 
  do {
    ret2 = faxml_preparse_process_node(reader);
    ret = xmlTextReaderRead(reader);
  } while (ret == 1 && ret2 == 0);
    
  // XML file has been parsed, free the reader
  xmlFreeTextReader(reader);

  if (ret == -1 || ret2 == 1 || ret2 == -1) {
    fprintf(stderr, " [FAXML] Error while parsing '%s' for automaton '%d'.\n",
	    pre_parser_data.xml_file_path, pre_parser_data.next_automaton);
    return -1;
  }

  // traverse the list and see if it is a NFA or DFA
  list_for_each(ctr, pre_parser_data.list) {
    for (i = 0; ctr->transitions[i] != 0; i++) {
      // found a NFA
      if (ctr->transitions[i] > 1) {
	parser_data.nfa = 1;
	ctr = NULL;
	break;
      } else
	parser_data.nfa = 0;
    }
    if (ctr == NULL)
      break;
  }

  parser_data.list = pre_parser_data.list;
  parser_data.preparsed = 1;

  switch (automaton->data_type) {
  case DT_NO:
#if HAVE_MODULE_HASH
    parser_data.data_structure = (void *)hash_init((hash_t *)parser_data.data_structure,
                                                   pre_parser_data.nb_states);
#else
    parser_data.data_structure = (void *)rbtree_init();
#endif
    break;
#if HAVE_MODULE_HASH
  case DT_HASH_TABLE:
    parser_data.data_structure = (void *)hash_init((hash_t *)parser_data.data_structure,
                                                   pre_parser_data.nb_states);
    break;
#endif
#if HAVE_MODULE_RBTREE
  case DT_RB_TREE:
    parser_data.data_structure = (void *)rbtree_init();
    break;
#endif
  default:
    fprintf(stderr, " [FAXML] Error while reading data type %d from \
automaton.\n", automaton->data_type);
    return 1;
    break;
  }
  
  if (parser_data.data_structure == NULL) {
    fprintf(stderr, " [FAXML] Not enough memory to allocate a new data \
structure.\n");
    return 1;
  }

  free((void *)pre_parser_data.alphabet);
  pre_parser_data.alphabet = NULL;

  // reset the preparser state
  pre_parser_data.current_automaton = 0;
  pre_parser_data.nb_states = 0;
  pre_parser_data.nb_symbols = 0;
  pre_parser_data.alphabet = NULL;
  pre_parser_data.list = NULL;
  pre_parser_data.error = 0;

  return 0;
}

int faxml_preparse_process_node(xmlTextReaderPtr reader) {
  xmlChar *attr = NULL;
  const xmlChar *name, *ch = NULL;
  char *buf = NULL, *trav = NULL, *pos = NULL;
  int i, l, idx = 0, len = 0, node_type = 0, ret = 0, str_size = 0;
  TransitionRow *new_tr_row = NULL;

  name = xmlTextReaderConstName(reader);
  if (name == NULL) {
    fprintf(stderr, " [FAXML] No further node name available for '%s' reading \
automaton '%d'.\n", pre_parser_data.xml_file_path,
	    pre_parser_data.next_automaton);
    return -1;
  }

  if (pre_parser_data.error)
    return 1;

  // get the enumeration value of the FAXML element
  for (i = 0; i < FAXML_NB; i++) {
    ret = xmlStrncasecmp(name, faxml_node_names[i], 11);
    if (ret == 0)
      break;
  }

  node_type = xmlTextReaderNodeType(reader);
  if (node_type == -1) { // error
    fprintf(stderr, " [FAXML] Error while determining node type.\n");
    return -1;
  }

  if (((node_type == 15) && (i != FAXML_FAXML) && (i != FAXML_AUTOMATON) &&
       pre_parser_data.skip) || ((node_type == 1) && (i != FAXML_AUTOMATON) &&
				 pre_parser_data.skip))
    return 0;

  switch (i) {
  case FAXML_FAXML:
    if (node_type == 1) {
      pre_parser_data.current_automaton = 0;

      assert(pre_parser_data.current_automaton == 0);
      assert(pre_parser_data.next_automaton > 0);
    }
    break;
  case FAXML_AUTOMATON:
    if (node_type == 15) { // element end
      assert(pre_parser_data.current_automaton > 0);

      /* automaton that needed to be parsed is now preparsed -> switch on
	 skipping                                                             */
      if ((pre_parser_data.current_automaton == pre_parser_data.next_automaton)
	  && (pre_parser_data.skip == 0)) {
	pre_parser_data.current_automaton = 0;
	pre_parser_data.next_automaton++;
	pre_parser_data.skip = 1;
	return 2;
      }
    } else if (node_type == 1) { // element start
      pre_parser_data.current_automaton++;

      assert(pre_parser_data.current_automaton <= pre_parser_data.next_automaton);

      /* switch off skipping if c_a equals n_a and next as well as read flags
	 are not set                                                          */
      if ((pre_parser_data.current_automaton == pre_parser_data.next_automaton)
	  && pre_parser_data.skip)
	pre_parser_data.skip = 0;
    }
    break;
  case FAXML_ALPHABET:
    if ((node_type == 1) && (pre_parser_data.alphabet == NULL)) {
      ret = xmlTextReaderRead(reader); // read the text node of the <alphabet> node
      if (ret == 0) {
	fprintf(stderr, " [FAXML] Text node of <alphabet> node has no text \
value reading automaton '%d'.\n", pre_parser_data.current_automaton);
	return 1;
      }	else if (ret == -1) {
	fprintf(stderr, " [FAXML] Internal error while reading text node of \
<alphabet> node of automaton '%d'.\n", pre_parser_data.current_automaton);
	return -1;
      }
      ret = xmlTextReaderHasValue(reader);
      if (ret == 0) {
	fprintf(stderr, " [FAXML] Text node of <alphabet> node has no text \
value reading automaton '%d'.\n", pre_parser_data.current_automaton);
	return 1;
      } else if (ret == -1) {
	fprintf(stderr, " [FAXML] Internal error while preparsing <alphabet> \
node of automaton '%d'.\n", pre_parser_data.current_automaton);
	return ret;
      }
      ch = xmlTextReaderConstValue(reader);
      str_size = xmlStrlen(ch);
  
      ret = efa_alloc(&buf, sizeof(char), str_size + 1);
      if (ret == -1) {
	efa_error(EFA_MOD_FAXML, __LINE__, __FILE__);
	pre_parser_data.error = 1;
	return 1;
      }
  
      strncpy(buf, (const char *)ch, str_size);
  
      // null-terminate the string as strncpy does not do it
      buf[str_size] = '\0';
	 
      ret = trim((const char **)&buf, str_size);
      if (ret == -1) {
	pre_parser_data.error = 1;
	fprintf(stderr, "Not enough memory to allocate a new string.\n");
      }
  
      pre_parser_data.nb_symbols = ret;
      pre_parser_data.alphabet = (const char *)buf;
      buf = NULL;
    }
    break;
  case FAXML_STATE:
    if (node_type == 1)
      pre_parser_data.nb_states++;
    break;
  case FAXML_TRANSITIONS:
    if (node_type == 1) { // element start
      // zero-allocate memory for the new transition row list element
      ret = efa_alloc(&new_tr_row, sizeof(TransitionRow), 1);
      if (ret == -1) {
	efa_error(EFA_MOD_FAXML, __LINE__, __FILE__);
	pre_parser_data.error = 1;
	break;
      }

      // zero-allocate memory for the transition row
      ret = efa_alloc(&new_tr_row->transitions, sizeof(int),
pre_parser_data.nb_symbols);
      if (ret == -1) {
	efa_error(EFA_MOD_FAXML, __LINE__, __FILE__);
	pre_parser_data.error = 1;
	break;
      }

      // append the new transition row list element to the list
      list_append(pre_parser_data.list, new_tr_row);
    }
    break;
  case FAXML_TRANSITION:
    if (node_type == 15) // element end
      break;

    ret = xmlTextReaderHasAttributes(reader);
    // preparse the attributes of the transition node
    if (ret == 1) {
		  
      attr = xmlTextReaderGetAttribute(reader, faxml_transition_attributes[FAXML_TRANSITION_LABEL]);
      // attribute not found or error
      if (attr == NULL)
	break;
			 
      len = (int) xmlStrlen(attr);
      trav = (char *)attr;
      
      for (new_tr_row = pre_parser_data.list; new_tr_row->next != NULL;
	   new_tr_row = new_tr_row->next);

      // find the index for each symbol in the alphabet and ...
      for(l = 0; l < len; l++) {
	pos = strchr(pre_parser_data.alphabet, (int) trav[l]);
	idx = pos - pre_parser_data.alphabet;
	  
	// ... increment the number of transitions for symbol at 'idx'
	new_tr_row->transitions[idx]++;
      }

      trav = NULL;
      efa_free(attr);
    }

    break;
  default:
    break;
  }

  return 0;
}

int faxml_validate(const char *schema, unsigned char verbose) {
  int ret = 0;
  struct timespec *runtime = NULL;
  xmlSchemaParserCtxtPtr xmlSchema_pc = NULL;
  xmlSchemaValidCtxtPtr xmlSchema_vc = NULL;
  xmlSchemaPtr internal_schema = NULL;

#if DEBUG
  const char *user_data = "DEBUGGING";
  xmlParserInputBufferPtr xml_buffer = NULL;
  xmlSAXHandlerPtr sax_handler_ptr = &sax2_handler_debug;
  xmlSchemaValidCtxtPtr xmlSchemaDeb_vc = NULL;
#endif

  // create a new XML Schema parser context based on the given XML Schema
  xmlSchema_pc = xmlSchemaNewParserCtxt(schema);
  if (xmlSchema_pc == NULL) {
    fprintf(stderr, " [FAXML] Could not allocate memory for a XML Schema \
parser context.\n");
    return -1;
  } 
  
  if (verbose & VLVL_TIME) {
    ret = efa_start_timer();
    if (ret == -1)
      return ret;
  }
  
  // create an LIBXML internal representation of the given XML Schema
  internal_schema = xmlSchemaParse(xmlSchema_pc);
  // free the XML Schema paser context resources
  if (verbose & VLVL_TIME) {
    runtime = efa_end_timer();
    if (runtime == NULL)
      return -1;
      
    printf(" [FAXML] DOM parsing XML Schema '%s': %ld s %ld ns\n", schema,
	   (signed long int) runtime->tv_sec,
	   (signed long int) runtime->tv_nsec);
    efa_free(runtime);
  }
  
  xmlSchemaFreeParserCtxt(xmlSchema_pc);

#if DEBUG
  printf("-------------------------------------------------------------------\
-------------\n [FAXML]: XML SCHEMA DUMP:\n");
  xmlSchemaDump(stdout, internal_schema); /* print the XML Schema structure to
					     STDOUT                          */
  printf("-------------------------------------------------------------------\
-------------\n");
  
  xml_buffer = xmlParserInputBufferCreateFilename(parser_data.xml_file_path,
						  XML_CHAR_ENCODING_NONE);
  if (xml_buffer == NULL) {
    fprintf(stderr, " [FAXML] Could not create a buffer for %s\n",
	    parser_data.xml_file_path);
    return 1;
  }
  
  xmlSchemaDeb_vc = xmlSchemaNewValidCtxt(internal_schema);
  if (xmlSchemaDeb_vc == NULL) {
    xmlFreeParserInputBuffer(xml_buffer);
    fprintf(stderr, " [FAXML] Could not create a XML Schema validation \
context.\n");
    return -1;
  }

  xmlSchemaSetValidErrors(xmlSchemaDeb_vc, (xmlSchemaValidityErrorFunc) fprintf,
			  (xmlSchemaValidityWarningFunc) fprintf, stderr);
  
  printf(" [FAXML]: XML FILE DEBUG PARSING:\n");
  ret = xmlSchemaValidateStream(xmlSchemaDeb_vc, xml_buffer,
				XML_CHAR_ENCODING_NONE, sax_handler_ptr,
				(void *)user_data);
  if (ret != 0)
    goto error;

  printf("-------------------------------------------------------------------\
-------------\n");

  xmlSchemaFreeValidCtxt(xmlSchemaDeb_vc);
#endif

  // create a new XML Schema validation context based on the given XML Schema
  xmlSchema_vc = xmlSchemaNewValidCtxt(internal_schema);
  if (xmlSchema_vc == NULL) {
    fprintf(stderr, " [FAXML] Could not create a XML Schema validation \
context.\n");
    return -1;
  }

  // set callback functions for validation context in case of errors
  xmlSchemaSetValidErrors(xmlSchema_vc, (xmlSchemaValidityErrorFunc) fprintf,
			  (xmlSchemaValidityWarningFunc) fprintf, stderr);

  if (verbose & VLVL_TIME) {
    ret = efa_start_timer();
    if (ret == -1)
      return ret;
  }

  // SAX2 validating
  ret = xmlSchemaValidateFile(xmlSchema_vc, parser_data.xml_file_path, 0);

  if (verbose & VLVL_TIME) {
    runtime = efa_end_timer();
    if (runtime == NULL)
      return -1;
    
    printf(" [FAXML] SAX valiating XML file '%s': %ld s %ld ns\n",
	   parser_data.xml_file_path, (signed long int) runtime->tv_sec,
	   (signed long int) runtime->tv_nsec);
    efa_free(runtime);
  }
  
  xmlSchemaFreeValidCtxt(xmlSchema_vc); /* free the resources of the XML
					   Schema validation context       */

  if (internal_schema != NULL)
    xmlSchemaFree(internal_schema); /* free the resources of the internal XML
				       Schema                                */
  // NOTE: something seams to be wrong with xmlFreeParserInputBuffer as it
  // crashes the program with an double free or corruption error. Better not
  // use it and risk instead a memory leak.
  // make shure to test if xml_buffer was already deallocated
  //if (xml_buffer != NULL)
  //	xmlFreeParserInputBuffer(xml_buffer); /* free the buffer allocated for the
  //	         				 XML file                          */
  
  xmlSchemaCleanupTypes(); // cleanup the XML Schemas type library
  
#if DEBUG
error:
#endif
  if (ret == 0) {
    printf(" [FAXML] '%s' is a valid FAXML file.\n",
           parser_data.xml_file_path);
  } else if (ret > 0) {
    printf(" [FAXML] '%s' is NOT a valid FAXML file.\n",
           parser_data.xml_file_path);
    return ret;
  } else {
    fprintf(stderr, " [FAXML] '%s' validation generated an internal error.\n",
	    parser_data.xml_file_path);
    return ret;
  }

  parser_data.validated = 1;

  return 0;
}

/*
 * faxml_validate_parse:
 * Validate and parse the XML file given by the '-i' / '--input' option with
 * the SAX2 parser. The XML file is validated against the default XML Schema or
 * validate it against the XML Schema given by the '-s' / '--schema' option.
 * Returns 0 if the XML file is valid, the could be read and indicates that
 * there are more automata to be read. A posivite integer if the XML file fails
 * to validate and a negative integer if the XML file has no more automata to
 * read.
 */
int faxml_validate_parse(fa_t *automaton, const char *xml_file_path,
			 unsigned char verbose, void *data) {
  const char *schema = ((frd_t *)data)->schema_file_path;
  int i, ret = 0;
  struct timespec *runtime = NULL;
  xmlSAXHandlerPtr sax_handler_ptr = NULL;
  
#if HAVE_MODULE_HASH  
  const char *function = ((frd_t *)data)->function;
#endif

#if HAVE_MODULE_HASH
  if (automaton->data_type == DT_HASH_TABLE)
    sax_handler_ptr = &sax2_hash_handler;
#endif

#if HAVE_MODULE_RBTREE
  if (automaton->data_type == DT_RB_TREE)
    sax_handler_ptr = &sax2_rbtree_handler;
#endif

  if (!parser_data.initialized) {
    ret = faxml_initialize(automaton, xml_file_path);
    if (ret)
      return ret;
  }
  
  xmlLineNumbersDefault(1); // enable line numbers in elements contents

  if (!parser_data.validated) {
    ret = faxml_validate(schema, verbose);
    if (ret)
      return ret;
  }
  
  // select data type and initialize the parser state
  switch (automaton->data_type) {
  case DT_NO:
#if HAVE_MODULE_HASH
    ret = efa_alloc(&parser_data.data_structure, sizeof(hash_t), 1);
#else
    ret = efa_alloc(&parser_data.data_structure, sizeof(rbtree_t), 1);
#endif
    if (ret == -1)
      goto alloc_error;
    
    break;
#if HAVE_MODULE_HASH
  case DT_HASH_TABLE:
    ret = efa_alloc(&parser_data.data_structure, sizeof(hash_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    break;
#endif
#if HAVE_MODULE_RBTREE
  case DT_RB_TREE:
    ret = efa_alloc(&parser_data.data_structure, sizeof(rbtree_t), 1);
    if (ret == -1)
      goto alloc_error;
    
    break;
#endif
  default:
    fprintf(stderr, " [FAXML] Error while reading data type %d from \
automaton.\n", automaton->data_type);
    return 1;
    break;
  }
  
  if (!parser_data.preparsed) {
    ret = faxml_preparse(automaton);
    if (ret)
      return ret;
  }
  
  if (parser_data.data_structure == NULL) {
    fprintf(stderr, " [FAXML] Not enough memory to allocate a new data \
structure.\n");
    return 1;
  }
  
#if HAVE_MODULE_HASH
  if (automaton->data_type == DT_HASH_TABLE) {
    for (i = 0; i < HASH_FUN_NB; i++) {
      ret = strncmp(function, hash_function_names[i], HASH_FUN_LEN);
      if (ret == 0)
	break;
    }
    
    if (i == HASH_FUN_NB)
      return 1;
    
    ((hash_t *)parser_data.data_structure)->function = hash_functions[i];
  }
#endif
  
  if (verbose & VLVL_TIME) {
    ret = efa_start_timer();
    if (ret == -1)
      return ret;
  }
  
  // SAX2 parsing starts here
  ret = xmlSAXUserParseFile(sax_handler_ptr, (void *)&parser_data, 
			    xml_file_path);
  if (verbose & VLVL_TIME) {
    runtime = efa_end_timer();
    if (runtime == NULL)
      return -1;
    
    printf(" [FAXML] SAX parsing XML file '%s': %ld s %ld ns\n", 
	   xml_file_path, (signed long int) runtime->tv_sec,
	   (signed long int) runtime->tv_nsec);
    efa_free(runtime);
  }
  
  if (ret) {
    fprintf(stderr, " [FAXML] '%s' parsing generated an internal error.\n",
	    xml_file_path);
    return 1;
  } else {
    /* assign all relevant data form the parser state to the automaton
       descriptor                                                             */
    automaton->data_structure = parser_data.data_structure;
  }
	 
  // reset the parser state partially
  parser_data.data_structure = NULL;
  parser_data.current_automaton = 0;
  parser_data.current_state = NULL;
  parser_data.preparsed = 0;
  parser_data.hasAccepting = 0;
  parser_data.hasInitial = 0;
  parser_data.isAlphabet = 0;
  parser_data.next = 0;
  parser_data.read = 0;
  parser_data.error = 0;
  
  if (parser_data.next_automaton == 0)
    return -1;

  return 0;
  
alloc_error:
  efa_errno = EFA_EFAXML_VALIDATE_PARSE;
  efa_error(EFA_MOD_FAXML, __LINE__, __FILE__);
  exit(EXIT_FAILURE);
}

#if HAVE_MODULE_HASH

/*
 * faxml_hash_write:
 * Writes an automaton in hash table to a XML file in FAXML format.
 */
int faxml_hash_write(fa_t *automaton, const char *xml_file_path,
		     unsigned char verbose, void *data) {
  hnode_t *nstate = NULL, *to_state;
  hscan_t *scanner = NULL;
  int i, j, k, l, n, append = 0, ret = 0;
  struct timespec *runtime = NULL;
  hnlst_t *visited_list = NULL, *visited_state = NULL;
  state_t *state = NULL;
  xmlChar *transition_label = NULL;
  const xmlChar *bool_value;
  const xmlChar *false_str = (const xmlChar *)"false";
  const xmlChar *true_str = (const xmlChar *)"true";
  xmlTextWriterPtr writer = NULL;
  
  if (automaton == NULL)
    return 0;
  
  writer = ((fwd_t *)data)->writer;

  ret = efa_alloc(&scanner, sizeof(hscan_t), 1);
  if (ret == -1) {
    efa_error(EFA_MOD_FAXML, __LINE__, __FILE__);
    exit(EXIT_FAILURE);
  }

  if (verbose & VLVL_TIME) {
    ret = efa_start_timer();
    if (ret == -1)
      return ret;
  }
	 	 
  // write the AUTOMATON element
  ret = xmlTextWriterStartElement(writer, faxml_node_names[FAXML_AUTOMATON]);
  if (ret == -1)
    goto error;
	 
  // write and close the CLASS attribute
  ret = xmlTextWriterWriteAttribute(writer,
				    (const xmlChar *)faxml_automaton_attributes[FAXML_AUTOMATON_CLASS],
				    (const xmlChar *) 
efa_fa_class_names[automaton->class]);
  if (ret == -1)
    goto error;
	 
  // write and close the MINIMAL attribute
  if (automaton->minimal)
    bool_value = true_str;
  else
    bool_value = false_str;
  
  ret = xmlTextWriterWriteAttribute(writer,
				    (const xmlChar *)faxml_automaton_attributes[FAXML_AUTOMATON_MINIMAL],
				    bool_value);
  if (ret == -1)
    goto error;
	 
  // write and close the NAME attribute
  ret = xmlTextWriterWriteAttribute(writer,
				    (const xmlChar *)faxml_automaton_attributes[FAXML_AUTOMATON_NAME],
				    (const xmlChar *)automaton->name);
  if (ret == -1)
    goto error;
	 
  // write and close the TRANS_RE attribute
  if (automaton->trans_re)
    bool_value = true_str;
  else
    bool_value = false_str;
  
  ret = xmlTextWriterWriteAttribute(writer,
				    (const xmlChar *)faxml_automaton_attributes[FAXML_AUTOMATON_TRANS_RE],
				    bool_value);
  if (ret == -1)
    goto error;
	 
  // write and close the TYPE attribute
  ret = xmlTextWriterWriteAttribute(writer,
				    (const xmlChar *)faxml_automaton_attributes[FAXML_AUTOMATON_TYPE],
				    (const xmlChar *) 
efa_fa_type_names[automaton->type]);
  if (ret == -1)
    goto error;
	 
  // write and close the ALPHABET element
  ret = xmlTextWriterWriteElement(writer, faxml_node_names[FAXML_ALPHABET],
				  (const xmlChar *)automaton->alphabet);
  if (ret == -1)
    goto error;
	 
  // write the STATES element
  ret = xmlTextWriterStartElement(writer, faxml_node_names[FAXML_STATES]);
  if (ret == -1)
    goto error;

  hash_scan_begin(scanner, (hash_t *)automaton->data_structure);
  nstate = hash_scan_next(scanner);
  
  while (nstate != NULL) {
    state = (state_t *)nstate->data;
    
    // write the STATE element
    ret = xmlTextWriterStartElement(writer, faxml_node_names[FAXML_STATE]);
    if (ret == -1)
      goto error;
		
    // write and close the ACCEPTING attribute
    if (state->accept)
      bool_value = true_str;
    else
      bool_value = false_str;
		  
    ret = xmlTextWriterWriteAttribute(writer,
				      (const xmlChar *)faxml_state_attributes[FAXML_STATE_ACCEPTING],
				      bool_value);
    if (ret == -1)
      goto error;
		
    // write and close the INITIAL attribute
    if (((hash_t *)automaton->data_structure)->initial == nstate)
      bool_value = true_str;
    else
      bool_value = false_str;
		
    ret = xmlTextWriterWriteAttribute(writer,
				      (const xmlChar *)faxml_state_attributes[FAXML_STATE_INITIAL],
				      bool_value);
    if (ret == -1)
      goto error;
		
    // write and close the NAME attribute
    ret = xmlTextWriterWriteAttribute(writer,
				      (const xmlChar *)faxml_state_attributes[FAXML_STATE_NAME],
				      (const xmlChar *)nstate->key);
    if (ret == -1)
      goto error;
		
    // write the TRANSITIONS element
    ret = xmlTextWriterStartElement(writer, faxml_node_names[FAXML_TRANSITIONS]);
    if (ret == -1)
      goto error;
	
    ret = efa_alloc(&transition_label, sizeof(xmlChar), automaton->nb_symbols);
    if (ret == -1) {
      fprintf(stderr, " [FAXML] Not enouth memory to allocate a temporary \
transition label.\n");
      goto error;
    }

    if (automaton->class == FA_CLASS_DETERMINISTIC) {
      for (i = 0; i < automaton->nb_symbols; i++) {
	// test for a non-complete automaton
	if (((hnode_t **)state->transition_row)[i] != NULL) {
	  // write the TRANSITION element
	  ret = xmlTextWriterStartElement(writer, faxml_node_names[FAXML_TRANSITION]);
	  if (ret == -1)
	    goto error;
		  
	  // write and close the LABEL attribute
	  ret = xmlTextWriterWriteAttribute(writer,
					    (const xmlChar *)faxml_transition_attributes[FAXML_TRANSITION_LABEL],
					    (const xmlChar *)&automaton->alphabet[i]);
	  if (ret == -1)
	    goto error;
	  
	  // wirte and close the TO attribute
	  to_state = ((hnode_t **)state->transition_row)[i];
	  ret = xmlTextWriterWriteAttribute(writer,
					    (const xmlChar *)faxml_transition_attributes[FAXML_TRANSITION_TO],
					    (const xmlChar *)to_state->key);
	  if (ret == -1)
	    goto error;
		  
	  // close the TRANSITION element
	  ret = xmlTextWriterEndElement(writer);
	  if (ret == -1)
	    goto error;
	}
      }
      // automaton is non-deterministic
    } else {
      if ((hnode_t ***)state->transition_row != NULL) {
	// zero-allocate memory for the transition label
	ret = efa_alloc(&transition_label, sizeof(xmlChar),
			automaton->nb_symbols + 1);
	if (ret == -1) {
	  fprintf(stderr, " [FAXML] Not enough memory to allocate a string.\n");
	  goto error;
	}

	// loop through the transition row
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
		// write the TRANSITION element
		ret = xmlTextWriterStartElement(writer,
					    faxml_node_names[FAXML_TRANSITION]);
		if (ret == -1)
		  goto error;

		to_state = ((hnode_t ***)state->transition_row)[i][j];
		/* see if 'to_state' is also reached by a transition with 
		   another symbol                                            */
		// loop through the rest of the transition row
		for (k = i + 1; k < automaton->nb_symbols; k++) {
		  l = 0;
		  if (((hnode_t ***)state->transition_row)[k] != NULL) {
		    while (((hnode_t ***)state->transition_row)[k][l] != NULL) {
		      // does the state appear for another symbol?
		      if (((hnode_t ***)state->transition_row)[k][l] == 
to_state) 
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
		((state_t *)to_state->data)->visited = 1;

		// add it to the visited list for faster resetting
		ret = efa_alloc(&visited_state, sizeof(hnlst_t), 1);
		if (ret == -1) {
		  fprintf(stderr, " [FAXML] Not enough memory to allocate a \
'slist_t' structure.\n");
		  goto error;
		}
		visited_state->node = to_state;
		list_append(visited_list, visited_state);

		// write and close the LABEL attribute
		ret = xmlTextWriterWriteAttribute(writer,
					      (const xmlChar *)faxml_transition_attributes[FAXML_TRANSITION_LABEL],
					      (const xmlChar *)transition_label);
		if (ret == -1)
		  goto error;
			 
		// wirte and close the TO attribute
		ret = xmlTextWriterWriteAttribute(writer,
					      (const xmlChar *)faxml_transition_attributes[FAXML_TRANSITION_TO],
					      (const xmlChar *)to_state->key);
		if (ret == -1)
		  goto error;

		// close the TRANSITON element
		ret = xmlTextWriterEndElement(writer);
		if (ret == -1)
		  goto error;
	    
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
		
    // close the TRANSITIONS element
    ret = xmlTextWriterEndElement(writer);
    if (ret == -1)
      goto error;
		
    // close the STATE element
    ret = xmlTextWriterEndElement(writer);
    if (ret == -1)
      goto error;

    // get the next state from the scanner
    nstate = hash_scan_next(scanner);
  }
	 
  // close the STATES element
  ret = xmlTextWriterEndElement(writer);
  if (ret == -1)
    goto error;

  // close the AUTOMATON element
  ret = xmlTextWriterEndElement(writer);
  if (ret == -1)
    goto error;
  
  if (verbose & VLVL_TIME) {
    runtime = efa_end_timer();
    if (runtime == NULL)
      return -1;
    
    printf(" [FAXML] ");
    if (append)
      printf("Appending to ");
    else
      printf("Writing ");

    printf("XML file '%s': %ld s %ld ns\n", xml_file_path,
	   (signed long int) runtime->tv_sec,
	   (signed long int) runtime->tv_nsec);
    efa_free(runtime);
  }
  
  efa_free(scanner);

  return 0;
  
error:
  fprintf(stderr, " [FAXML] An internal error occured while the XML \
writer was writing the XML document\n");

  efa_free(scanner);
  
  return ret;
}

#endif

#if HAVE_MODULE_RBTREE

/*
 * faxml_rbtree_write:
 * Writes an automaton in hash table to a XML file in FAXML format.
 */
int faxml_rbtree_write(Automaton *automaton, const char *xml_file_path,
		       void *data) {

  return 0;
}

#endif
