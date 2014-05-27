/*
 * Licence: Public domain
 *
 * Author: Bob Jenkins
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

#ifndef BERNSTEIN_H
#define BERNSTEIN_H

#include "libefa/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __WORDSIZE == 32
uint32_t bernstein(const char *, unsigned int, void *);
#elif __WORDSIZE == 64
uint64_t bernstein(const char *, unsigned int, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BERNSTEIN_H */
