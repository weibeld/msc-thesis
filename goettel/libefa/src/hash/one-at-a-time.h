/*
 * Licence: Public domain
 *
 * Author: Bob Jenkins
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

#ifndef ONE_AT_A_TIME_H
#define ONE_AT_A_TIME_H

#include "libefa/hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __WORDSIZE == 32
uint32_t one_at_a_time(const char *, unsigned int, void *);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ONE_AT_A_TIME_H */
