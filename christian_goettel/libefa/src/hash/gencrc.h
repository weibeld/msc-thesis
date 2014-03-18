/* By Bob Jenkins, (c) 2006, Public Domain
   Modified by: Christian Göttel <christian.goettel@unifr.ch> */

#ifndef GENCRC_H
#define GENCRC_H

#include "libefa/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __WORDSIZE == 32
uint32_t gencrc(const char *, unsigned int, void *);
#elif __WORDSIZE == 64
uint64_t gencrc(const char *, unsigned int, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GENCRC_H */
