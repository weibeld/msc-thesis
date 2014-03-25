/*
 * memory.c: safer memory allocation
 *
 * Copyright (C) 2008-2011 Daniel P. Berrange
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
 * Author: Daniel P. Berrange
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */


#ifndef MEMORY_H_
#define MEMORY_H_

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * efa_free:
 * Safe variant to free memory.
 * 
 * PTR: pointer to memory that gets freed.
 */
#define efa_free(ptr)                                                          \
  do {                                                                         \
    if ((void *)(ptr) != NULL) {                                               \
      free((ptr));                                                             \
      (ptr) = NULL;                                                            \
    }                                                                          \
  } while (0)
  
/* 
 * efa_alloc:
 * Zero allocate memory.
 * 
 * PTRPTR: address of the pointer holding the newly allocated memory.
 * 
 * SIZE: size of the data structure.
 * 
 * COUNT: number of elements in the memory array.
 */
int efa_alloc(void *ptrptr, size_t size, size_t count) \
__attribute__((__warn_unused_result__));

/*
 * efa_realloc:
 * Reallocates memory.
 * 
 * PTRPTR: address of the pointer holding the memory that gets reallocated.
 * 
 * SIZE: size of the data structure.
 * 
 * COUNT: number of elements in the reallocated memory.
 */
int efa_realloc(void *ptrptr, size_t size, size_t count) \
__attribute__((__warn_unused_result__));
  
#ifdef __cplusplus
}
#endif /* __cplusplus */
  
#endif /* MEMORY_H_ */
