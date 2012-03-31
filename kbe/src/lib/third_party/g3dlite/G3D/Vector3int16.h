/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef VECTOR3INT16_H
#define VECTOR3INT16_H

#include "platform.h"
#include "g3dmath.h"

namespace G3D {

/**
 A Vector3 that packs its fields into uint16s.
 */
#ifdef G3D_WIN32
    // Switch to tight alignment
    #pragma pack(push, 2)
#endif

class Vector3int16 {
private:
    // Hidden operators
    bool operator<(const Vector3int16&) const;
    bool operator>(const Vector3int16&) const;
    bool operator<=(const Vector3int16&) const;
    bool operator>=(const Vector3int16&) const;

public:
    int16              x;
    int16              y;
    int16              z;

    Vector3int16() : x(0), y(0), z(0) {}
    Vector3int16(int16 _x, int16 _y, int16 _z) : x(_x), y(_y), z(_z) {}
    Vector3int16(const class Vector3& v);
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
