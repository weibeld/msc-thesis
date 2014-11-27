/*
 * Licence: Public domain
 *
 * Author: Bob Jenkins
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

#include "bernstein.h"

#if __WORDSIZE == 32
uint32_t bernstein(const char *key, unsigned int len,
		   void *level __attribute__((unused)))
#elif __WORDSIZE == 64
uint64_t bernstein(const char *key, unsigned int len,
		   void *level __attribute__((unused)))
#endif
{
  /*
#if __WORDSIZE == 32
  uint32_t hash = *(uint32_t *)level;
#elif __WORDSIZE == 64
  uint64_t hash = *(uint64_t *)level;
#endif
  */
  
#if __WORDSIZE == 32
  uint32_t hash = 0U;
#elif __WORDSIZE == 64
  uint64_t hash = 0U;
#endif

  unsigned int i;

  for (i = 0U; i < len; i++)
    hash = 33 * hash + key[i];

  return hash;
}
