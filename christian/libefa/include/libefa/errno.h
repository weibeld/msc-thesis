/*
 * errno.h: libefa error numbers
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

#ifndef ERRNO_H_
#define ERRNO_H_

/*
 * emsg_name:
 * Enumeration of libefa errors.
 */
enum emsg_name {
  EFA_EUNKNOWN,
  EFA_EALLOC,
#if HAVE_MODULE_BUECHI
  EFA_EBUECHI_BUILD_SET_LIST,
  EFA_EBUECHI_CREATE_INSERT_TUPLE,
  EFA_EBUECHI_CREATE_TUPLE,
  EFA_EBUECHI_CREATE_TUPLE_LABEL,
  EFA_EBUECHI_MOD_SUB_CONST,
  EFA_EBUECHI_TUPLE_CONST,
  EFA_EBUECHI_UNIFR,
#endif
  EFA_EEND_TIMER,
#if HAVE_MODULE_FAMOD
  EFA_EFA_COPY_SOS,
  EFA_EFA_CREATE_INSERT_SOS,
  EFA_EFA_CREATE_SOS,
  EFA_EFA_NFA_MOVE,
  #endif
#if HAVE_MODULE_FAXML
  EFA_EFAXML_VALIDATE_PARSE,
#endif
  EFA_EFOPEN,
  EFA_EMAIN,
  EFA_EPARSE_OPT_ALGORITHM,
  EFA_EPARSE_OPT_AT,
  EFA_EPARSE_OPT_CLASS,
  EFA_EPARSE_OPT_DS,
  EFA_EPARSE_OPT_VERBOSE,
  EFA_EPARSE_OPTS,
  EFA_EPRINTF,
  EFA_EPUTS,
  EFA_EREALLOC,
  EFA_EREGEXP_READ,
  EFA_ESETUID,
  EFA_ESTART_TIMER,
  EFA_ETIMER,
#if HAVE_MODULE_FAXML
  EFA_EXMLNEWTEXTWRITERFILENAME,
  EFA_EXMLTEXTWRITERENDDOCUMENT,
  EFA_EXMLTEXTWRITERENDELEMENT,
  EFA_EXMLTEXTWRITERSETINDENT,
  EFA_EXMLTEXTWRITERSTARTDOCUMENT,
  EFA_EXMLTEXTWRITERSTARTELEMENT,
  EFA_EXMLTEXTWRITERWRITEATTRIBUTE,
#endif
  EFA_ENB
};

extern int efa_errno;

#endif /* ERRNO_H_ */