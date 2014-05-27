//-----------------------------------------------------------------------------
// MurmurHash was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.
// Modified by: Christian Göttel <christian.goettel@unifr.ch>

#include "MurmurHash1.h"

//-----------------------------------------------------------------------------

#if __WORDSIZE == 32

uint32_t MurmurHash1(const char *key, unsigned int len,
		     void *foo __attribute__ ((unused))) {
  uint32_t seed = 0x5602350aU; // hardcoded seed
  const uint32_t m = 0xc6a4a793U;
  const int r = 16;
  uint32_t h = seed ^ (len * m);

  //----------
 
  const unsigned char *data = (const unsigned char *)key;

  while(len >= 4)
    {
      unsigned int k = *(unsigned int *)data;

      h += k;
      h *= m;
      h ^= h >> 16;

      data += 4;
      len -= 4;
    }
 
  //----------
 
  switch(len)
    {
    case 3:
      h += data[2] << 16;
    case 2:
      h += data[1] << 8;
    case 1:
      h += data[0];
      h *= m;
      h ^= h >> r;
    };
 
  //----------

  h *= m;
  h ^= h >> 10;
  h *= m;
  h ^= h >> 17;

  return h;
}

//-----------------------------------------------------------------------------
// MurmurHash1Aligned, by Austin Appleby

// Same algorithm as MurmurHash1, but only does aligned reads - should be safer
// on certain platforms.

// Performance should be equal to or better than the simple version.

uint32_t MurmurHash1Aligned(const char *key, unsigned int len,
			    void *foo __attribute__ ((unused))) {
  uint32_t seed = 0x5602350aU; // hardcoded seed
  const uint32_t m = 0xc6a4a793U;
  const int r = 16;

  const unsigned char *data = (const unsigned char *)key;

  uint32_t h = seed ^ (len * m);

  int align = (uint32_t) data & 3;

  if(align && (len >= 4)) {
      // Pre-load the temp registers

      unsigned int t = 0, d = 0;

      switch(align){
	case 1: t |= data[2] << 16;
	case 2: t |= data[1] << 8;
	case 3: t |= data[0];
      }

      t <<= (8 * align);

      data += 4-align;
      len -= 4-align;

      int sl = 8 * (4-align);
      int sr = 8 * align;

      // Mix

      while(len >= 4)
	{
	  d = *(unsigned int *)data;
	  t = (t >> sr) | (d << sl);
	  h += t;
	  h *= m;
	  h ^= h >> r;
	  t = d;

	  data += 4;
	  len -= 4;
	}

      // Handle leftover data in temp registers

      int pack = len < align ? len : align;

      d = 0;

      switch(pack)
	{
	case 3: d |= data[2] << 16;
	case 2: d |= data[1] << 8;
	case 1: d |= data[0];
	case 0: h += (t >> sr) | (d << sl);
	  h *= m;
	  h ^= h >> r;
	}

      data += pack;
      len -= pack;
    }
  else
    {
      while(len >= 4)
	{
	  h += *(unsigned int *)data;
	  h *= m;
	  h ^= h >> r;

	  data += 4;
	  len -= 4;
	}
    }

  //----------
  // Handle tail bytes

  switch(len)
    {
    case 3: h += data[2] << 16;
    case 2: h += data[1] << 8;
    case 1: h += data[0];
      h *= m;
      h ^= h >> r;
    };

  h *= m;
  h ^= h >> 10;
  h *= m;
  h ^= h >> 17;

  return h;
}

#elif __WORDSIZE == 64

uint64_t MurmurHash1(const char *key, unsigned int len,
		     void *foo __attribute__ ((unused))) {
  uint32_t seed = 0x5602350aU; // hardcoded seed
  const uint64_t m = 0xc6a4a7936111982fU;
  const int r = 32;
  uint64_t h = seed ^ (len * m);

  //----------
 
  const unsigned char *data = (const unsigned char *)key;

  while(len >= 4)
    {
      unsigned int k = *(unsigned int *)data;

      h += k;
      h *= m;
      h ^= h >> 16;

      data += 4;
      len -= 4;
    }
 
  //----------
 
  switch(len)
    {
    case 3:
      h += data[2] << 16;
    case 2:
      h += data[1] << 8;
    case 1:
      h += data[0];
      h *= m;
      h ^= h >> r;
    };
 
  //----------

  h *= m;
  h ^= h >> 19;
  h *= m;
  h ^= h >> 33;

  return h;
}

//-----------------------------------------------------------------------------
// MurmurHash1Aligned, by Austin Appleby

// Same algorithm as MurmurHash1, but only does aligned reads - should be safer
// on certain platforms.

// Performance should be equal to or better than the simple version.

uint64_t MurmurHash1Aligned(const char *key, unsigned int len,
			    void *foo __attribute__ ((unused))) {
  uint64_t seed = 0x5602350aU; // hardcoded seed
  const uint64_t m = 0xc6a4a7936111982fU;
  const int r = 32;

  const unsigned char *data = (const unsigned char *)key;

  uint64_t h = seed ^ (len * m);

  int align = (uint64_t) data & 3;

  if(align && (len >= 8)) {
      // Pre-load the temp registers

      unsigned int t = 0, d = 0;

      switch(align){
	case 1: t |= data[2] << 16;
	case 2: t |= data[1] << 8;
	case 3: t |= data[0];
      }

      t <<= (8 * align);

      data += 4-align;
      len -= 4-align;

      int sl = 8 * (4-align);
      int sr = 8 * align;

      // Mix

      while(len >= 4)
	{
	  d = *(unsigned int *)data;
	  t = (t >> sr) | (d << sl);
	  h += t;
	  h *= m;
	  h ^= h >> r;
	  t = d;

	  data += 4;
	  len -= 4;
	}

      // Handle leftover data in temp registers

      int pack = len < align ? len : align;

      d = 0;

      switch(pack)
	{
	case 3: d |= data[2] << 16;
	case 2: d |= data[1] << 8;
	case 1: d |= data[0];
	case 0: h += (t >> sr) | (d << sl);
	  h *= m;
	  h ^= h >> r;
	}

      data += pack;
      len -= pack;
    }
  else
    {
      while(len >= 4)
	{
	  h += *(unsigned int *)data;
	  h *= m;
	  h ^= h >> r;

	  data += 4;
	  len -= 4;
	}
    }

  //----------
  // Handle tail bytes

  switch(len)
    {
    case 3: h += data[2] << 16;
    case 2: h += data[1] << 8;
    case 1: h += data[0];
      h *= m;
      h ^= h >> r;
    };

  h *= m;
  h ^= h >> 19;
  h *= m;
  h ^= h >> 33;

  return h;
}

#endif /* __WORDSIZE == XX */
