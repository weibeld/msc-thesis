/*
 * Rotating Hash
 * Version 1 of the rotating hash need to work with a hash table that has the
 * length of a prime number.
 *
 * Licence for v1: unknown
 * Licence for v2: Public domain
 *
 * Author: Bob Jenkins
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

#ifndef ROTATING_H
#define ROTATING_H

#include "libefa/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __WORDSIZE == 32
uint32_t rotating_v1(const char *, unsigned int, void *);
uint32_t rotating_v2(const char *, unsigned int, void *);
#elif __WORDSIZE == 64
uint64_t rotating_v1(const char *, unsigned int, void *);
uint64_t rotating_v2(const char *, unsigned int, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ROTATING_H */
