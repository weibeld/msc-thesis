/*
 * Licence: Public domain
 *
 * Author: Cessu
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

#ifndef CESSU_H
#define CESSU_H

#include "libefa/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#if (__WORDSIZE == 32) && HAVE_XMMINTRIN_H
uint32_t hasshe2(const char *, unsigned int, void *);
uint32_t hasshe2_v2(const char *, unsigned int, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CESSU_H */
