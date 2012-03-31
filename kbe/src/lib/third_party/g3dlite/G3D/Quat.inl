/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

namespace G3D {

inline float& Quat::operator[] (int i) {
    debugAssert(i >= 0);
    debugAssert(i < 4);
    return ((float*)this)[i];
}

inline const float& Quat::operator[] (int i) const {
    debugAssert(i >= 0);
    debugAssert(i < 4);
    return ((float*)this)[i];
}



inline Quat Quat::operator-(const Quat& other) const {
    return Quat(x - other.x, y - other.y, z - other.z, w - other.w);
}

inline Quat Quat::operator+(const Quat& other) const {
    return Quat(x + other.x, y + other.y, z + other.z, w + other.w);
}

}

