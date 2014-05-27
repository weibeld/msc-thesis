/*
 * Additive Hash
 * This hash needs to work with a hash table that has the length of a prime
 * number.
 *
 * Licence: unkown.
 *
 * Author: Bob Jenkins
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

#ifndef ADDITIVE_H
#define ADDITIVE_H

#include "libefa/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __WORDSIZE == 32
uint32_t additive(const char *, unsigned int, void *);
#elif __WORDSIZE == 64
uint64_t additive(const char *, unsigned int, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ADDITIVE_H */
