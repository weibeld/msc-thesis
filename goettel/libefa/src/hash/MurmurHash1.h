//-----------------------------------------------------------------------------
// MurmurHash1 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
// Modified by: Christian Göttel <christian.goettel@unifr.ch>

#ifndef _MURMURHASH1_H_
#define _MURMURHASH1_H_

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER)

typedef unsigned char uint8_t;
typedef unsigned long uint32_t;
typedef unsigned __int64 uint64_t;

// Other compilers

#else   // defined(_MSC_VER)

#include <stdint.h>

#endif // !defined(_MSC_VER)

#include "libefa/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------

#if __WORDSIZE == 32
uint32_t MurmurHash1        (const char *, unsigned int, void *);
uint32_t MurmurHash1Aligned (const char *, unsigned int, void *);
#elif __WORDSIZE == 64
uint64_t MurmurHash1        (const char *, unsigned int, void *);
uint64_t MurmurHash1Aligned (const char *, unsigned int, void *);
#endif

//-----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MURMURHASH1_H_ */
