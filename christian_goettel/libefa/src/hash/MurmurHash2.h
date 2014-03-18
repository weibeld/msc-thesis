//-----------------------------------------------------------------------------
// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
// Modified by: Christian Göttel <christian.goettel@unifr.ch>

#ifndef _MURMURHASH2_H_
#define _MURMURHASH2_H_

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
uint32_t MurmurHash2        (const char *, unsigned int, void *);
uint32_t MurmurHash2A       (const char *, unsigned int, void *);
uint32_t MurmurHashNeutral2 (const char *, unsigned int, void *);
uint32_t MurmurHashAligned2 (const char *, unsigned int, void *);
uint64_t MurmurHash64B      (const char *, unsigned int, void *); // 64-bit hash for 32-bit machines
#elif __WORDSIZE == 64
uint64_t MurmurHash64A      (const char *, unsigned int, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

//-----------------------------------------------------------------------------

#endif /* _MURMURHASH2_H_ */
