/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef G3DMATH_H
#define G3DMATH_H

#include "cstdkbe/cstdkbe.hpp" // for uint32, etc typedefs
using namespace KBEngine;
#ifdef _MSC_VER
// Disable conditional expression is constant, which occurs incorrectly on inlined functions
#   pragma warning (push)
#   pragma warning (disable : 4127)
// disable: "C++ exception handler used"
#   pragma warning (disable : 4530)
#endif

#include "platform.h"
#include <ctype.h>
#include <string>
#include <float.h>
#include <limits>

/*These defines enable functionality introduced with the 1999 ISO C
**standard. They must be defined before the inclusion of math.h to
**engage them. If optimisation is enabled, these functions will be 
**inlined. With optimisation switched off, you have to link in the
**maths library using -lm.
*/

#define _ISOC9X_SOURCE1
#define _ISOC99_SOURCE1
#define __USE_ISOC9X1
#define __USE_ISOC991

#include <math.h>

#include "debug.h"

#undef min
#undef max

namespace G3D {

#ifdef _MSC_VER

//#include <intrin.h>
#include <xmmintrin.h>

/**
   Win32 implementation of the C99 fast rounding routines.
   
   @cite routines are
   Copyright (C) 2001 Erik de Castro Lopo <erikd AT mega-nerd DOT com>
   
   Permission to use, copy, modify, distribute, and sell this file for any 
   purpose is hereby granted without fee, provided that the above copyright 
   and this permission notice appear in all copies.  No representations are
   made about the suitability of this software for any purpose.  It is 
   provided "as is" without express or implied warranty.
*/

#if _MSC_VER >= 1400 && !COMPILER_INTEL
#pragma float_control(push)
#pragma float_control(precise, on)
#endif

__inline long int lrint (double flt) {
#ifndef X64
	int intgr;

    _asm {
        fld flt
        fistp intgr
    };

    return intgr;

#else

	union { int asInt[2]; double asDouble; } n;
	n.asDouble = flt + 6755399441055744.0;
#if USING_BIG_ENDIAN
	return n.asInt [1];
#else
	return n.asInt [0];
#endif

#endif

}

__inline long int lrintf(float flt) {
#ifndef X64
    int intgr;

	_asm {
        fld flt
        fistp intgr
    };

    return intgr;
#else

	union { int asInt[2]; double asDouble; } n;
	n.asDouble = flt + 6755399441055744.0;
#if USING_BIG_ENDIAN
	return n.asInt [1];
#else
	return n.asInt [0];
#endif

#endif
}

#if _MSC_VER >= 1400 && !COMPILER_INTEL
#pragma float_control(pop)
#endif

#endif



const double fuzzyEpsilon = 0.00001;

/** Returns a reference to a static double.
    This value should not be tested against directly, instead
    G3D::isNan() and G3D::isFinite() will return reliable results. */
inline const double& inf() {

// We already have <limits> included but
// not using it in older gcc for safe compilations
#if (__GNUC__ == 2)    
    static const double i = 1.0/sin(0.0);
#else
    // double is a standard type and should have infinity
    static const double i = std::numeric_limits<double>::infinity();
#endif
	return i;
}

/** Returns a reference to a static double.
    This value should not be tested against directly, instead
    G3D::isNan() and G3D::isFinite() will return reliable results. */
inline const double& nan() {

// We already have <limits> included but
// not using it in older gcc for safe compilations
#if (__GNUC__ == 2)
    static const double n = 0.0/sin(0.0);
#else
    // double is a standard type and should have quiet NaN
    static const double n = std::numeric_limits<double>::quiet_NaN();
#endif
	return n;
}

/** Returns a reference to a static double. Use instead of G3D_PI. */
inline const double& pi() {
    static const double p = 3.1415926535898;
    return p;
}

/** Returns a reference to a static double. Use instead of G3D_HALF_PI. */
inline const double& halfPi() {
    static const double p = 1.5707963267949;
    return p;
}

/** Returns a reference to a static double. Use instead of G3D_TWO_PI. */
inline const double& twoPi() {
    static const double p = 6.283185;
    return p;
}

/** @def G3D_PI
    @deprecated Use G3D::pi() instead. */
#define G3D_PI      (3.1415926535898)
/** @def G3D_HALF_PI
    @deprecated Use G3D::halfPi() instead. */
#define G3D_HALF_PI (1.5707963267949)
/** @def G3D_TWO_PI
    @deprecated Use G3D::twoPi() instead. */
#define G3D_TWO_PI  (6.283185)

int iAbs(int iValue);
int iCeil(double fValue);

/**
 Clamps the value to the range [low, hi] (inclusive)
 */
int iClamp(int val, int low, int hi);
double clamp(double val, double low, double hi);
float clamp(float val, float low, float hi);

/**
 Returns a + (b - a) * f;
 */
inline double lerp(double a, double b, double f) {
    return a + (b - a) * f;
}

inline float lerp(float a, float b, float f) {
    return a + (b - a) * f;
}

/**
 Wraps the value to the range [0, hi) (exclusive
 on the high end).  This is like the clock arithmetic
 produced by % (modulo) except the result is guaranteed
 to be positive.
 */
int iWrap(int val, int hi);

int iFloor(double fValue);

int iSign(int iValue);
int iSign(double fValue);

inline int iSign(float f) {
    return iSign((double)f);
}


/** 
    Fast round to integer using the lrint routine.
    Typically 6x faster than casting to integer.
 */
inline int iRound(double fValue) {
    return lrint(fValue);
}

/** 
    Fast round to integer using the lrint routine.
    Typically 6x faster than casting to integer.
 */
inline int iRound(float f) {
    return lrintf(f);
}

/**
 Returns a random number uniformly at random between low and hi
 (inclusive).
 */
int iRandom(int low, int hi);

double abs (double fValue);
double aCos (double fValue);
double aSin (double fValue);
double aTan (double fValue);
double aTan2 (double fY, double fX);
double sign (double fValue);
double square (double fValue);

/**
 Returns true if the argument is a finite real number.
 */
bool isFinite(double x);

/**
 Returns true if the argument is NaN (not a number).
 You can't use x == nan to test this because all
 comparisons against nan return false.
 */
bool isNaN(double x);

/**
 Computes x % 3.
 */
int iMod3(int x);

/** 
 [0, 1]
 @deprecated use uniformRandom()
 */
double unitRandom ();

/**
 Uniform random number between low and hi, inclusive.
 @deprecated use uniformRandom()
 */
double random(double low, double hi);

/**
 [-1, 1]
 @deprecated use uniformRandom()
 */
double symmetricRandom ();

/**
 Uniform random number between low and hi, inclusive. [low, hi]
 */
float uniformRandom(float low = 0.0f, float hi = 1.0f);

/**
 Normally distributed random number. 
 */
float gaussRandom(float mean = 0.0f, float stdev = 1.0f);

int iMin(int x, int y);
int iMax(int x, int y);

double square(double x);
double sumSquares(double x, double y);
double sumSquares(double x, double y, double z);
double distance(double x, double y);
double distance(double x, double y, double z);

/**
  Returnes the 0-based index of the highest 1 bit from
  the left.  -1 means the number was 0.

  @cite Based on code by jukka@liimatta.org
 */ 
int highestBit(uint32 x);

/**
 Note that fuzzyEq(a, b) && fuzzyEq(b, c) does not imply
 fuzzyEq(a, c), although that will be the case on some
 occasions.
 */
bool fuzzyEq(double a, double b);

/** True if a is definitely not equal to b.  
    Guaranteed false if a == b. 
    Possibly false when a != b.*/
bool fuzzyNe(double a, double b);

/** Is a strictly greater than b? (Guaranteed false if a <= b).
    (Possibly false if a > b) */
bool fuzzyGt(double a, double b);

/** Is a near or greater than b? */
bool fuzzyGe(double a, double b);

/** Is a strictly less than b? (Guaranteed false if a >= b)*/
bool fuzzyLt(double a, double b);

/** Is a near or less than b? */
bool fuzzyLe(double a, double b);

/**
 Computes 1 / sqrt(x).
 */
inline float rsq(float x) {
    return 1.0f / sqrtf(x);
}

/**
 Uses SSE to implement rsq.
 @cite Nick nicolas@capens.net
 */
inline float SSErsq(float x) {

    #if defined(SSE) && defined(G3D_WIN32)
	#ifdef X64
		__m128 a, b;
		a = _mm_set_ps1(x);
		b = _mm_rsqrt_ss(a);
		return b.m128_f32[0];
	#else
	    __asm {
           movss xmm0, x
           rsqrtss xmm0, xmm0
           movss x, xmm0
        }
        return x;
	#endif
    #else
        return 1.0f / sqrt(x);
    #endif
}

/**
 Return the next power of 2 higher than the input
 If the input is already a power of 2, the output will be the same 
 as the input.
 */
int ceilPow2(unsigned int in);

/**
 * True if num is a power of two.
 */
bool isPow2(int num);

bool isOdd(int num);
bool isEven(int num);

double toRadians(double deg);
double toDegrees(double rad);

/**
 Returns true if x is not exactly equal to 0.0f.
 */
inline bool any(float x) {
    return x != 0;
}

/**
 Returns true if x is not exactly equal to 0.0f.
 */
inline bool all(float x) {
    return x != 0;
}

/**
 v / v (for DirectX/Cg support)
 */
inline float normalize(float v) {
    return v / v;
}

/**
 a * b (for DirectX/Cg support)
 */
inline float dot(float a, float b) {
    return a * b;
}


/**
 a * b (for DirectX/Cg support)
 */
inline float mul(float a, float b) {
    return a * b;
}

/**
 2^x
 */
inline double exp2(double x) {
    return pow(2.0, x);
}

inline double rsqrt(double x) {
    return 1.0 / sqrt(x);
}


/**
 sin(x)/x
 */
inline double sinc(double x) {
    double r = sin(x) / x;

    if (isNaN(r)) {
        return 1.0;
    } else {
        return r;
    }
}

/**
 Computes a floating point modulo; the result is t wrapped to the range [lo, hi).
 */
inline double wrap(double t, double lo, double hi) {
    if ((t >= lo) && (t < hi)) {
        return t;
    }

    debugAssert(hi > lo);

    double interval = hi - lo;

    return t - interval * iFloor((t - lo) / interval);

}

inline double wrap(double t, double hi) {
    return wrap(t, 0, hi);
}


} // namespace

#ifdef _MSC_VER
#   pragma warning (pop)
#endif

#include "g3dmath.inl"

#endif

