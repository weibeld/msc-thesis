/*
 * error.h: extended finite automata error
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

#ifndef ERROR_H_
#define ERROR_H_

#include "libefa/efa.h"

/*
 * efa_error:
 * Prints an error message to STDERR when an error occurs in the EFA library.
 *
 * MODNUM:   module number (can be found in efa.h)
 * LINENUM:  line number (always use the the __LINE__ macro)
 * FILENAME: file name (always use the __FILE__ macro)
 */
void efa_error(efa_mod_t, const int, const char *);

#endif /* ERROR_H_ */
