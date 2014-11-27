/*
 * fadot.h: header with import and export methods for dot format
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

#ifndef FADOT_H_
#define FADOT_H_

#include "libefa/efa.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if HAVE_MODULE_HASH
/*
 * fadot_hash_write:
 * Write an automaton stored in a hash table to a DOT file.
 */
int fadot_hash_write(fa_t *, const char *, unsigned char, void *);
#endif

#if HAVE_MODULE_RBTREE
/*
 * fadot_rbtree_write:
 * Write an automaton stored in a red-black-tree to a DOT file.
 */
int fadot_rbtree_write(fa_t *, const char *, unsigned char, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FADOT_H_ */
