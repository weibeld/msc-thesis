/*
 * error.c: extended finite automata error
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

#include "libefa/errno.h"
#include "libefa/error.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int efa_errno;

static const char *const emsg[] = {
  "Unknown error",
  "Could not allocate data structure",
#if HAVE_MODULE_BUECHI
  "Could not build list of sets of states",
  "Could not create and insert tuple",
  "Could not create tuple",
  "Could not create tuple label",
  "Could not proceed with modified subset construction",
  "Could not construct tuple",
  "Could not complement Büchi automaton",
#endif
  "Could not stop timer",
#if HAVE_MODULE_FAMOD
  "Could not copy set of states",
  "Could not create and insert set of states",
  "Could not create set of states",
  "Could not move from set of states",
#endif
#if HAVE_MODULE_FAXML
  "Could not validate / parse XML file",
#endif
  "Could not open file",
  "Error in main function",
  "Could not parse algorithm option",
  "Could not parse automaton type option",
  "Could not parse automaton class option",
  "Could not parse data structure option",
  "Could not parse verbose option",
  "Error while assigning argument options",
  "Could not write formatted string to STDOUT",
  "Could not write string to STDOUT",
  "Could not reallocate data structure",
  "Could not read regular expression",
  "Could not set processes effective UID",
  "Could not start timer",
  "Could not retrieve time from CLOCK_REALTIME",
#if HAVE_MODULE_FAXML
  "Could not allocate memory for a new XML writer",
  "Could not end writing XML document",
  "Could not write end XML element",
  "Could not set indentation for XML writer",
  "Could not start writing XML document",
  "Could not write start XML element",
  "Could not write XML attribute",
#endif
  NULL
};

/*
 * efa_error:
 * Prints an error message to STDERR.
 */
void efa_error(efa_mod_t modnum, const int linenum, const char *filename) {
  char buffer[128];
  int i = 0;
  size_t len;

  memset((void *)buffer, 0, 128);
  buffer[i++] = ' ';
  buffer[i++] = '[';
  len = strlen(efa_module_names[modnum]);
  memcpy((void *)&buffer[i], (const void *)efa_module_names[modnum], len);
  i += (int) len;
  buffer[i++] = ']';
  buffer[i++] = ' ';
  len = strlen(filename);
  memcpy((void *)&buffer[i], (const void *)filename, len);
  i += (int) len;
  buffer[i++] = ':';
  len = ((size_t) log10f((float) linenum)) + 1;
  snprintf(&buffer[i], len + 1, "%d", linenum);
  i += (int) len;
  buffer[i++] = ' ';
  len = strlen(emsg[efa_errno]);
  memcpy((void *)&buffer[i], (const void *)emsg[efa_errno], len);
  perror((const char *)buffer);
}
