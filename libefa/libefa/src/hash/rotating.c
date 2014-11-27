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

#include "rotating.h"

#if __WORDSIZE == 32
uint32_t rotating_v1(const char *key, unsigned int len, void *prime) {
  uint32_t hash;
  unsigned int i;

  for (i = 0, hash = (uint32_t) len; i < len; i++)
    hash = (hash << 4) ^ (hash >> 28) ^ ((uint32_t) key[i]);

  return (hash % (*(uint32_t *)prime));
}

uint32_t rotating_v2(const char *key, unsigned int len,
		      void *data __attribute__ ((unused))) {
  uint32_t hash;
  unsigned int i;

  for (i = 0, hash = (uint32_t) len; i < len; i++)
    hash = (hash << 4) ^ (hash >> 28) ^ ((uint32_t) key[i]);

  hash = (hash ^ (hash >> 10) ^ (hash >> 20));

  return hash;
}

#elif __WORDSIZE == 64

uint64_t rotating_v1(const char *key, unsigned int len, void *prime) {
  uint64_t hash;
  unsigned int i;

  for (i = 0, hash = (uint64_t) len; i < len; i++)
    hash = (hash << 8) ^ (hash >> 56) ^ ((uint64_t) key[i]);

  return (hash % (*(uint64_t *)prime));
}

uint64_t rotating_v2(const char *key, unsigned int len,
		     void *data __attribute__ ((unused))) {
  uint64_t hash;
  unsigned int i;

  for (i = 0, hash = (uint64_t) len; i < len; i++)
    hash = (hash << 8) ^ (hash >> 56) ^ ((uint64_t) key[i]);

  hash = (hash ^ (hash >> 20) ^ (hash >> 40));

  return hash;
}

#endif
