/*
 * Licence: Public domain
 *
 * Author: Cessu
 * Modified by: Christian Göttel <christian.goettel@unifr.ch>
 */

/* Compile with gcc -msse2 ... */

#include "libefa/error.h"
#include "libefa/memory.h"

#include <assert.h>
#include <stdio.h>
#include <xmmintrin.h>

#include "cessu.h"

typedef signed short s_int16_t; /* A 16-bit signed type. */

#if (__WORDSIZE == 32) && HAVE_XMMINTRIN_H
static s_int16_t mul_add_coeffs[16] __attribute__((aligned(16))) = {
  18743,  28829, -22921, -14039, -15247, -11117, -14293,  18911,
  -23407,  17987, -11321, -25943, -32287,  10001,  30773, -12541,
};

/* Compute a 256-bit hash into res[0..31] using SSE2.  buf and res
   need not be aligned, but len must be divisible by 32. */
uint32_t hasshe2(const char *buf, unsigned int len,
		 void *data __attribute__ ((unused))) {
  int ret = 0;
  uint32_t *res;
  __m128i madd_1, madd_2, in_1, in_2, hash_1, hash_2, tmp_1, tmp_2, *res128;

  ret = efa_alloc(&res128, sizeof(__m128i), sizeof(__m128i) + 16);
  if (ret == -1) {
    efa_error(EFA_MOD_HASH, __LINE__, __FILE__);
    return 0U;
  }

  madd_1 = _mm_load_si128((void *) mul_add_coeffs);
  madd_2 = _mm_load_si128((void *) (mul_add_coeffs + 8));

  in_1 = _mm_loadu_si128((void *) buf);
  in_2 = _mm_loadu_si128((void *) (buf + 16));

  hash_1 = _mm_madd_epi16(madd_1, in_1);
  hash_2 = _mm_madd_epi16(madd_2, in_2);
  goto rest_of_mix;

  do {
    in_1 = _mm_loadu_si128((void *) buf);
    in_2 = _mm_loadu_si128((void *) (buf + 16));

    hash_1 = _mm_xor_si128(hash_1, _mm_madd_epi16(madd_1, in_1));
    hash_2 = _mm_xor_si128(hash_2, _mm_madd_epi16(madd_2, in_2));

  rest_of_mix:
    hash_1 = _mm_add_epi64(hash_1, _mm_srli_si128(hash_1, 3));
    hash_1 = _mm_sub_epi64(hash_1, 
			   _mm_shuffle_epi32(hash_1, _MM_SHUFFLE(0, 1, 2, 3)));
    hash_2 = _mm_add_epi64(hash_2, _mm_srli_si128(hash_2, 3));
    hash_2 = _mm_sub_epi64(hash_2,
			   _mm_shuffle_epi32(hash_2, _MM_SHUFFLE(0, 1, 2, 3)));
    
    tmp_1 = _mm_xor_si128(in_1, _mm_unpackhi_epi8(hash_2, hash_1));
    tmp_2 = _mm_xor_si128(in_2, _mm_unpacklo_epi8(hash_1, hash_2));
    hash_1 = _mm_xor_si128(tmp_2, _mm_madd_epi16(madd_1, tmp_1));
    hash_1 = _mm_sub_epi64(hash_1, 
			   _mm_shuffle_epi32(hash_1, _MM_SHUFFLE(0, 1, 2, 3)));
    hash_2 = _mm_xor_si128(tmp_1, _mm_madd_epi16(madd_2, tmp_2));
    hash_2 = _mm_sub_epi64(hash_2,
			   _mm_shuffle_epi32(hash_2, _MM_SHUFFLE(0, 1, 2, 3)));
    
    /* Move these after the loop if good statistical properties not
       required. */
    tmp_1 = _mm_xor_si128(in_1, _mm_unpackhi_epi8(hash_1, hash_2));
    tmp_2 = _mm_xor_si128(in_2, _mm_unpacklo_epi8(hash_2, hash_1));
    hash_1 = _mm_xor_si128(tmp_2, _mm_madd_epi16(madd_1, tmp_1));
    hash_1 = _mm_sub_epi64(hash_1, 
			   _mm_shuffle_epi32(hash_1, _MM_SHUFFLE(2, 1, 0, 3)));
    hash_2 = _mm_xor_si128(tmp_1, _mm_madd_epi16(madd_2, tmp_2));
    hash_2 = _mm_sub_epi64(hash_2,
			   _mm_shuffle_epi32(hash_2, _MM_SHUFFLE(2, 1, 0, 3)));
    
    len -= 32;
    buf += 32;
  } while (len > 0);

  _mm_storeu_si128(res128, hash_1);
  _mm_storeu_si128((res128 + 16), hash_2);

  res = (uint32_t *)res128;

  return *res;
}

static uint32_t coeffs[12] __attribute__((aligned(16))) = {
  /* Four carefully selected coefficients and interleaving zeros. */
  2561893793UL, 0, 1388747947UL, 0,
  3077216833UL, 0, 3427609723UL, 0,
  /* 128 bits of random data. */
  0x564A4447, 0xC7265595, 0xE20C241D, 0x128FA608,
};

#define COMBINE_AND_MIX(c_1, c_2, s_1, s_2, in)                              \
  /* Phase 1: Perform four 32x32->64 bit multiplication with the             \
     input block and words 1 and 3 coeffs, respectively.  This               \
     effectively propagates a bit change in input to 32 more                 \
     significant bit positions.  Combine into internal state by              \
     subtracting the result of multiplications from the internal             \
     state. */                                                               \
  s_1 = _mm_sub_epi64(s_1, _mm_mul_epu32(c_1, _mm_unpackhi_epi32(in, in)));  \
  s_2 = _mm_sub_epi64(s_2, _mm_mul_epu32(c_2, _mm_unpacklo_epi32(in, in)));  \
                                                                             \
  /* Phase 2: Perform shifts and xors to propagate the 32-bit                \
     changes produced above into 64-bit (and even a little larger)           \
     changes in the internal state. */                                       \
  /* state ^= state >64> 29; */                                              \
  s_1 = _mm_xor_si128(s_1, _mm_srli_epi64(s_1, 29));                         \
  s_2 = _mm_xor_si128(s_2, _mm_srli_epi64(s_2, 29));                         \
  /* state +64= state <64< 16; */                                            \
  s_1 = _mm_add_epi64(s_1, _mm_slli_epi64(s_1, 16));                         \
  s_2 = _mm_add_epi64(s_2, _mm_slli_epi64(s_2, 16));                         \
  /* state ^= state >64> 21; */                                              \
  s_1 = _mm_xor_si128(s_1, _mm_srli_epi64(s_1, 21));                         \
  s_2 = _mm_xor_si128(s_2, _mm_srli_epi64(s_2, 21));                         \
  /* state +64= state <128< 32; */                                           \
  s_1 = _mm_add_epi64(s_1, _mm_slli_si128(s_1, 4));                          \
  s_2 = _mm_add_epi64(s_2, _mm_slli_si128(s_2, 4));                          \
                                                                             \
  /* Phase 3: Propagate the changes among the four 64-bit words by           \
     performing 64-bit subtractions and 32-bit word shuffling. */            \
  s_1 = _mm_sub_epi64(s_1, s_2);                                             \
  s_2 = _mm_sub_epi64(_mm_shuffle_epi32(s_2, _MM_SHUFFLE(0, 3, 2, 1)), s_1); \
  s_1 = _mm_sub_epi64(_mm_shuffle_epi32(s_1, _MM_SHUFFLE(0, 1, 3, 2)), s_2); \
  s_2 = _mm_sub_epi64(_mm_shuffle_epi32(s_2, _MM_SHUFFLE(2, 1, 0, 3)), s_1); \
  s_1 = _mm_sub_epi64(_mm_shuffle_epi32(s_1, _MM_SHUFFLE(2, 1, 0, 3)), s_2); \
                                                                             \
  /* With good coefficients any one-bit flip in the input has now            \
     changed all bits in the internal state with a probability               \
     between 45% to 55%. */

/*
 * hasshe2_v2:
 * The input buffer must have a length % 16 = 0!
 */
uint32_t hasshe2_v2(const char *input_buf, unsigned int len,
		    void *foo __attribute__ ((unused))) {
  int ret = 0;
  uint32_t *output_state;
  size_t n_bytes = (size_t) len;
  __m128i coeffs_1, coeffs_2, rnd_data, input, state_1, state_2, *res128;

  ret = efa_alloc(&res128, sizeof(__m128i), sizeof(__m128i) + 16);
  if (ret == -1) {
    efa_error(EFA_MOD_HASH, __LINE__, __FILE__);
    return 0U;
  }

  assert(n_bytes % 16 == 0);

  coeffs_1 = _mm_load_si128((void *) coeffs);
  coeffs_2 = _mm_load_si128((void *) (coeffs + 4));
  rnd_data = _mm_load_si128((void *) (coeffs + 8));

  /* Initialize internal state to something random.  (Alternatively,
     if hashing a chain of data, read in the previous hash result from
     somewhere.) */
  state_1 = state_2 = rnd_data;

  while (n_bytes >= 16) {
    /* Read in 16 bytes, or 128 bits, from buf.  Advance buf and
       decrement n_bytes accordingly. */
    input = _mm_loadu_si128((void *) input_buf);
    input_buf += 16;
    n_bytes -= 16;

    COMBINE_AND_MIX(coeffs_1, coeffs_2, state_1, state_2, input);
  }

  /* Postprocessing.  Copy half of the internal state into fake input,
     replace it with the constant rnd_data, and do one combine and mix
     phase more. */ 
  input = state_1;
  state_1 = rnd_data;
  COMBINE_AND_MIX(coeffs_1, coeffs_2, state_1, state_2, input);

  _mm_storeu_si128(res128, state_1);
  _mm_storeu_si128((res128 + 16), state_2);

  output_state = (uint32_t *)res128;

  return *output_state;
}
#endif /* (__WORDSIZE == 32) && HAVE_XMMINTRIN_H */
