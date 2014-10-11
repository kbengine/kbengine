/* Set of hash utility functions to help maintaining the invariant that
    if a==b then hash(a)==hash(b)

   All the utility functions (_Py_Hash*()) return "-1" to signify an error.
*/
#include "Python.h"

#ifdef __APPLE__
#  include <libkern/OSByteOrder.h>
#elif defined(HAVE_LE64TOH) && defined(HAVE_ENDIAN_H)
#  include <endian.h>
#elif defined(HAVE_LE64TOH) && defined(HAVE_SYS_ENDIAN_H)
#  include <sys/endian.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

_Py_HashSecret_t _Py_HashSecret;

#if Py_HASH_ALGORITHM == Py_HASH_EXTERNAL
extern PyHash_FuncDef PyHash_Func;
#else
static PyHash_FuncDef PyHash_Func;
#endif

/* Count _Py_HashBytes() calls */
#ifdef Py_HASH_STATS
#define Py_HASH_STATS_MAX 32
static Py_ssize_t hashstats[Py_HASH_STATS_MAX + 1] = {0};
#endif

/* For numeric types, the hash of a number x is based on the reduction
   of x modulo the prime P = 2**_PyHASH_BITS - 1.  It's designed so that
   hash(x) == hash(y) whenever x and y are numerically equal, even if
   x and y have different types.

   A quick summary of the hashing strategy:

   (1) First define the 'reduction of x modulo P' for any rational
   number x; this is a standard extension of the usual notion of
   reduction modulo P for integers.  If x == p/q (written in lowest
   terms), the reduction is interpreted as the reduction of p times
   the inverse of the reduction of q, all modulo P; if q is exactly
   divisible by P then define the reduction to be infinity.  So we've
   got a well-defined map

      reduce : { rational numbers } -> { 0, 1, 2, ..., P-1, infinity }.

   (2) Now for a rational number x, define hash(x) by:

      reduce(x)   if x >= 0
      -reduce(-x) if x < 0

   If the result of the reduction is infinity (this is impossible for
   integers, floats and Decimals) then use the predefined hash value
   _PyHASH_INF for x >= 0, or -_PyHASH_INF for x < 0, instead.
   _PyHASH_INF, -_PyHASH_INF and _PyHASH_NAN are also used for the
   hashes of float and Decimal infinities and nans.

   A selling point for the above strategy is that it makes it possible
   to compute hashes of decimal and binary floating-point numbers
   efficiently, even if the exponent of the binary or decimal number
   is large.  The key point is that

      reduce(x * y) == reduce(x) * reduce(y) (modulo _PyHASH_MODULUS)

   provided that {reduce(x), reduce(y)} != {0, infinity}.  The reduction of a
   binary or decimal float is never infinity, since the denominator is a power
   of 2 (for binary) or a divisor of a power of 10 (for decimal).  So we have,
   for nonnegative x,

      reduce(x * 2**e) == reduce(x) * reduce(2**e) % _PyHASH_MODULUS

      reduce(x * 10**e) == reduce(x) * reduce(10**e) % _PyHASH_MODULUS

   and reduce(10**e) can be computed efficiently by the usual modular
   exponentiation algorithm.  For reduce(2**e) it's even better: since
   P is of the form 2**n-1, reduce(2**e) is 2**(e mod n), and multiplication
   by 2**(e mod n) modulo 2**n-1 just amounts to a rotation of bits.

   */

Py_hash_t
_Py_HashDouble(double v)
{
    int e, sign;
    double m;
    Py_uhash_t x, y;

    if (!Py_IS_FINITE(v)) {
        if (Py_IS_INFINITY(v))
            return v > 0 ? _PyHASH_INF : -_PyHASH_INF;
        else
            return _PyHASH_NAN;
    }

    m = frexp(v, &e);

    sign = 1;
    if (m < 0) {
        sign = -1;
        m = -m;
    }

    /* process 28 bits at a time;  this should work well both for binary
       and hexadecimal floating point. */
    x = 0;
    while (m) {
        x = ((x << 28) & _PyHASH_MODULUS) | x >> (_PyHASH_BITS - 28);
        m *= 268435456.0;  /* 2**28 */
        e -= 28;
        y = (Py_uhash_t)m;  /* pull out integer part */
        m -= y;
        x += y;
        if (x >= _PyHASH_MODULUS)
            x -= _PyHASH_MODULUS;
    }

    /* adjust for the exponent;  first reduce it modulo _PyHASH_BITS */
    e = e >= 0 ? e % _PyHASH_BITS : _PyHASH_BITS-1-((-1-e) % _PyHASH_BITS);
    x = ((x << e) & _PyHASH_MODULUS) | x >> (_PyHASH_BITS - e);

    x = x * sign;
    if (x == (Py_uhash_t)-1)
        x = (Py_uhash_t)-2;
    return (Py_hash_t)x;
}

Py_hash_t
_Py_HashPointer(void *p)
{
    Py_hash_t x;
    size_t y = (size_t)p;
    /* bottom 3 or 4 bits are likely to be 0; rotate y by 4 to avoid
       excessive hash collisions for dicts and sets */
    y = (y >> 4) | (y << (8 * SIZEOF_VOID_P - 4));
    x = (Py_hash_t)y;
    if (x == -1)
        x = -2;
    return x;
}

Py_hash_t
_Py_HashBytes(const void *src, Py_ssize_t len)
{
    Py_hash_t x;
    /*
      We make the hash of the empty string be 0, rather than using
      (prefix ^ suffix), since this slightly obfuscates the hash secret
    */
    if (len == 0) {
        return 0;
    }

#ifdef Py_HASH_STATS
    hashstats[(len <= Py_HASH_STATS_MAX) ? len : 0]++;
#endif

#if Py_HASH_CUTOFF > 0
    if (len < Py_HASH_CUTOFF) {
        /* Optimize hashing of very small strings with inline DJBX33A. */
        Py_uhash_t hash;
        const unsigned char *p = src;
        hash = 5381; /* DJBX33A starts with 5381 */

        switch(len) {
            /* ((hash << 5) + hash) + *p == hash * 33 + *p */
            case 7: hash = ((hash << 5) + hash) + *p++; /* fallthrough */
            case 6: hash = ((hash << 5) + hash) + *p++; /* fallthrough */
            case 5: hash = ((hash << 5) + hash) + *p++; /* fallthrough */
            case 4: hash = ((hash << 5) + hash) + *p++; /* fallthrough */
            case 3: hash = ((hash << 5) + hash) + *p++; /* fallthrough */
            case 2: hash = ((hash << 5) + hash) + *p++; /* fallthrough */
            case 1: hash = ((hash << 5) + hash) + *p++; break;
            default:
                assert(0);
        }
        hash ^= len;
        hash ^= (Py_uhash_t) _Py_HashSecret.djbx33a.suffix;
        x = (Py_hash_t)hash;
    }
    else
#endif /* Py_HASH_CUTOFF */
        x = PyHash_Func.hash(src, len);

    if (x == -1)
        return -2;
    return x;
}

void
_PyHash_Fini(void)
{
#ifdef Py_HASH_STATS
    int i;
    Py_ssize_t total = 0;
    char *fmt = "%2i %8" PY_FORMAT_SIZE_T "d %8" PY_FORMAT_SIZE_T "d\n";

    fprintf(stderr, "len   calls    total\n");
    for (i = 1; i <= Py_HASH_STATS_MAX; i++) {
        total += hashstats[i];
        fprintf(stderr, fmt, i, hashstats[i], total);
    }
    total += hashstats[0];
    fprintf(stderr, ">  %8" PY_FORMAT_SIZE_T "d %8" PY_FORMAT_SIZE_T "d\n",
            hashstats[0], total);
#endif
}

PyHash_FuncDef *
PyHash_GetFuncDef(void)
{
    return &PyHash_Func;
}

/* Optimized memcpy() for Windows */
#ifdef _MSC_VER
#  if SIZEOF_PY_UHASH_T == 4
#    define PY_UHASH_CPY(dst, src) do {                                    \
       dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; \
       } while(0)
#  elif SIZEOF_PY_UHASH_T == 8
#    define PY_UHASH_CPY(dst, src) do {                                    \
       dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; \
       dst[4] = src[4]; dst[5] = src[5]; dst[6] = src[6]; dst[7] = src[7]; \
       } while(0)
#  else
#    error SIZEOF_PY_UHASH_T must be 4 or 8
#  endif /* SIZEOF_PY_UHASH_T */
#else /* not Windows */
#  define PY_UHASH_CPY(dst, src) memcpy(dst, src, SIZEOF_PY_UHASH_T)
#endif /* _MSC_VER */


#if Py_HASH_ALGORITHM == Py_HASH_FNV
/* **************************************************************************
 * Modified Fowler-Noll-Vo (FNV) hash function
 */
static Py_hash_t
fnv(const void *src, Py_ssize_t len)
{
    const unsigned char *p = src;
    Py_uhash_t x;
    Py_ssize_t remainder, blocks;
    union {
        Py_uhash_t value;
        unsigned char bytes[SIZEOF_PY_UHASH_T];
    } block;

#ifdef Py_DEBUG
    assert(_Py_HashSecret_Initialized);
#endif
    remainder = len % SIZEOF_PY_UHASH_T;
    if (remainder == 0) {
        /* Process at least one block byte by byte to reduce hash collisions
         * for strings with common prefixes. */
        remainder = SIZEOF_PY_UHASH_T;
    }
    blocks = (len - remainder) / SIZEOF_PY_UHASH_T;

    x = (Py_uhash_t) _Py_HashSecret.fnv.prefix;
    x ^= (Py_uhash_t) *p << 7;
    while (blocks--) {
        PY_UHASH_CPY(block.bytes, p);
        x = (_PyHASH_MULTIPLIER * x) ^ block.value;
        p += SIZEOF_PY_UHASH_T;
    }
    /* add remainder */
    for (; remainder > 0; remainder--)
        x = (_PyHASH_MULTIPLIER * x) ^ (Py_uhash_t) *p++;
    x ^= (Py_uhash_t) len;
    x ^= (Py_uhash_t) _Py_HashSecret.fnv.suffix;
    if (x == -1) {
        x = -2;
    }
    return x;
}

static PyHash_FuncDef PyHash_Func = {fnv, "fnv", 8 * SIZEOF_PY_HASH_T,
                                     16 * SIZEOF_PY_HASH_T};

#endif /* Py_HASH_ALGORITHM == Py_HASH_FNV */


#if Py_HASH_ALGORITHM == Py_HASH_SIPHASH24
/* **************************************************************************
 <MIT License>
 Copyright (c) 2013  Marek Majkowski <marek@popcount.org>

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 </MIT License>

 Original location:
    https://github.com/majek/csiphash/

 Solution inspired by code from:
    Samuel Neves (supercop/crypto_auth/siphash24/little)
    djb (supercop/crypto_auth/siphash24/little2)
    Jean-Philippe Aumasson (https://131002.net/siphash/siphash24.c)

 Modified for Python by Christian Heimes:
    - C89 / MSVC compatibility
    - PY_UINT64_T, PY_UINT32_T and PY_UINT8_T
    - _rotl64() on Windows
    - letoh64() fallback
*/

typedef unsigned char PY_UINT8_T;

/* byte swap little endian to host endian
 * Endian conversion not only ensures that the hash function returns the same
 * value on all platforms. It is also required to for a good dispersion of
 * the hash values' least significant bits.
 */
#if PY_LITTLE_ENDIAN
#  define _le64toh(x) ((PY_UINT64_T)(x))
#elif defined(__APPLE__)
#  define _le64toh(x) OSSwapLittleToHostInt64(x)
#elif defined(HAVE_LETOH64)
#  define _le64toh(x) le64toh(x)
#else
#  define _le64toh(x) (((PY_UINT64_T)(x) << 56) | \
                      (((PY_UINT64_T)(x) << 40) & 0xff000000000000ULL) | \
                      (((PY_UINT64_T)(x) << 24) & 0xff0000000000ULL) | \
                      (((PY_UINT64_T)(x) << 8)  & 0xff00000000ULL) | \
                      (((PY_UINT64_T)(x) >> 8)  & 0xff000000ULL) | \
                      (((PY_UINT64_T)(x) >> 24) & 0xff0000ULL) | \
                      (((PY_UINT64_T)(x) >> 40) & 0xff00ULL) | \
                      ((PY_UINT64_T)(x)  >> 56))
#endif


#ifdef _MSC_VER
#  define ROTATE(x, b)  _rotl64(x, b)
#else
#  define ROTATE(x, b) (PY_UINT64_T)( ((x) << (b)) | ( (x) >> (64 - (b))) )
#endif

#define HALF_ROUND(a,b,c,d,s,t)         \
    a += b; c += d;             \
    b = ROTATE(b, s) ^ a;           \
    d = ROTATE(d, t) ^ c;           \
    a = ROTATE(a, 32);

#define DOUBLE_ROUND(v0,v1,v2,v3)       \
    HALF_ROUND(v0,v1,v2,v3,13,16);      \
    HALF_ROUND(v2,v1,v0,v3,17,21);      \
    HALF_ROUND(v0,v1,v2,v3,13,16);      \
    HALF_ROUND(v2,v1,v0,v3,17,21);


static Py_hash_t
siphash24(const void *src, Py_ssize_t src_sz) {
    PY_UINT64_T k0 = _le64toh(_Py_HashSecret.siphash.k0);
    PY_UINT64_T k1 = _le64toh(_Py_HashSecret.siphash.k1);
    PY_UINT64_T b = (PY_UINT64_T)src_sz << 56;
    const PY_UINT64_T *in = (PY_UINT64_T*)src;

    PY_UINT64_T v0 = k0 ^ 0x736f6d6570736575ULL;
    PY_UINT64_T v1 = k1 ^ 0x646f72616e646f6dULL;
    PY_UINT64_T v2 = k0 ^ 0x6c7967656e657261ULL;
    PY_UINT64_T v3 = k1 ^ 0x7465646279746573ULL;

    PY_UINT64_T t;
    PY_UINT8_T *pt;
    PY_UINT8_T *m;

    while (src_sz >= 8) {
        PY_UINT64_T mi = _le64toh(*in);
        in += 1;
        src_sz -= 8;
        v3 ^= mi;
        DOUBLE_ROUND(v0,v1,v2,v3);
        v0 ^= mi;
    }

    t = 0;
    pt = (PY_UINT8_T *)&t;
    m = (PY_UINT8_T *)in;
    switch (src_sz) {
        case 7: pt[6] = m[6];
        case 6: pt[5] = m[5];
        case 5: pt[4] = m[4];
        case 4: Py_MEMCPY(pt, m, sizeof(PY_UINT32_T)); break;
        case 3: pt[2] = m[2];
        case 2: pt[1] = m[1];
        case 1: pt[0] = m[0];
    }
    b |= _le64toh(t);

    v3 ^= b;
    DOUBLE_ROUND(v0,v1,v2,v3);
    v0 ^= b;
    v2 ^= 0xff;
    DOUBLE_ROUND(v0,v1,v2,v3);
    DOUBLE_ROUND(v0,v1,v2,v3);

    /* modified */
    t = (v0 ^ v1) ^ (v2 ^ v3);
    return (Py_hash_t)t;
}

static PyHash_FuncDef PyHash_Func = {siphash24, "siphash24", 64, 128};

#endif /* Py_HASH_ALGORITHM == Py_HASH_SIPHASH24 */

#ifdef __cplusplus
}
#endif
