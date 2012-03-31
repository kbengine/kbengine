/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef G3D_CRYPTO_H
#define G3D_CRYPTO_H

#include "platform.h"
#include "g3dmath.h"
#include <string>

namespace G3D {

/** Cryptography and hashing helper functions */
class Crypto {
public:

    /**
     Computes the CRC32 value of a byte array.  CRC32 is designed to be a hash
     function that produces different values for similar strings.

     This implementation is compatible with PKZIP and GZIP.

     Based on http://www.gamedev.net/reference/programming/features/crc32/
    */
    static uint32 crc32(const void* bytes, size_t numBytes);

    /**
     Returns the nth prime less than 2000 in constant time.  The first prime has index
     0 and is the number 2.
     */
    static int smallPrime(int n);

    /** Returns 1 + the largest value that can be passed to smallPrime. */
    static int numSmallPrimes();
};

}

#endif
