/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef VECTOR2INT16_H
#define VECTOR2INT16_H

#include "platform.h"
#include "g3dmath.h"

namespace G3D {

/**
 A Vector2 that packs its fields into uint16s.
 */
#ifdef G3D_WIN32
    // Switch to tight alignment
    #pragma pack(push, 2)
#endif

class Vector2int16 {
private:
    // Hidden operators
    bool operator<(const Vector2int16&) const;
    bool operator>(const Vector2int16&) const;
    bool operator<=(const Vector2int16&) const;
    bool operator>=(const Vector2int16&) const;

public:
    int16              x;
    int16              y;

    Vector2int16() : x(0), y(0) {}
    Vector2int16(int16 _x, int16 _y) : x(_x), y(_y){}
    Vector2int16(const class Vector2& v);

    inline int16& operator[] (int i) {
        debugAssert(((unsigned int)i) <= 1);
        return ((int16*)this)[i];
    }

    inline const int16& operator[] (int i) const {
        debugAssert(((unsigned int)i) <= 1);
        return ((int16*)this)[i];
    }


    inline bool operator== (const Vector2int16& rkVector) const {
        return ((int32*)this)[0] == ((int32*)&rkVector)[0];
    }

    inline bool operator!= (const Vector2int16& rkVector) const {
        return ((int32*)this)[0] != ((int32*)&rkVector)[0];
    }

}
#if defined(G3D_LINUX) || defined(G3D_OSX)
    __attribute((aligned(1)))
#endif
;

#ifdef G3D_WIN32
    #pragma pack(pop)
#endif

}
#endif
