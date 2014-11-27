/* By Bob Jenkins, (c) 2006, Public Domain
   Modified by: Christian Göttel <christian.goettel@unifr.ch>  */

#ifndef CRC_H
#define CRC_H

#include "libefa/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __WORDSIZE == 32
uint32_t crc(const char *, unsigned int, void *);
#elif __WORDSIZE == 64
uint64_t crc(const char *, unsigned int, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CRC_H */

