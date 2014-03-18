/*
 * Licence: Public domain
 *
 * Author: Bob Jenkins
 * Modified by: Christian Göttel
 */

#include "one-at-a-time.h"

#if __WORDSIZE == 32
uint32_t one_at_a_time(const char *key, unsigned int len,
			 void *data __attribute__ ((unused))) {
  uint32_t hash;
  unsigned int i;
  for (i = 0, hash = 0; i < len; i++) {
    hash += (uint32_t) key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash;
}
#endif
