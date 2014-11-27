/*
 * Licence: unknown
 *
 * Author:
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

#ifndef PAUL_HSIEH_H
#define PAUL_HSIEH_H

#include "libefa/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __WORDSIZE == 32
uint32_t SuperFastHash(const char *, unsigned int, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PAUL_HSIEH_H */

