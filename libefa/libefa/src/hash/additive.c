/*
 * Additive Hash
 * This hash needs to work with a hash table that has the length of a prime
 * number.
 *
 * Licence: unknown
 *
 * Author: Bob Jenkins
 * Modified by: Christian Göttel
 */

#include "additive.h"

#if __WORDSIZE == 32

uint32_t additive(const char *key, unsigned int len, void *prime) {
  uint32_t hash;
  unsigned int i;

  for (i = 0U, hash = (uint32_t) len; i < len; i++) 
    hash += (uint32_t) key[i];

  return (hash % (*(uint32_t *)prime));
}

#elif __WORDSIZE == 64

uint64_t additive(const char *key, unsigned int len, void *prime) {
  uint64_t hash;
  unsigned int i;

  for (i = 0U, hash = (uint64_t) len; i < len; i++)
    hash += (uint64_t) key[i];

  return (hash % (*(uint64_t *)prime));
}

#endif
