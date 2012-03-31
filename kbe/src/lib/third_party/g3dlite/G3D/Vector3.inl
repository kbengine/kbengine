/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

//----------------------------------------------------------------------------
#ifdef SSE
	// If you receive an error on this line, it is because you do not have the file
	// xmmintrin.h needed for MMX & SSE extensions.  Download and install
    //
    // http://download.microsoft.com/download/vstudio60ent/SP5/Wideband-Full/WIN98Me/EN-US/vs6sp5.exe
	// and
    // http://download.microsoft.com/download/vb60ent/Update/6/W9X2KXP/EN-US/vcpp5.exe
    //
    // to get this file.
#   include <xmmintrin.h>
#endif

inline size_t hashCode(const G3D::Vector3& v) {
     return v.hashCode();
}

namespace G3D {

//----------------------------------------------------------------------------
inline Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f) {
}

//----------------------------------------------------------------------------

inline Vector3::Vector3 (float fX, float fY, float fZ) : x(fX), y(fY), z(fZ) {
}

//----------------------------------------------------------------------------
inline Vector3::Vector3 (float V[3]) : x(V[0]), y(V[1]), z(V[2]){
}
//----------------------------------------------------------------------------
inline Vector3::Vector3 (double V[3]) : x((float)V[0]), y((float)V[1]), z((float)V[2]){
}

//----------------------------------------------------------------------------
inline Vector3::Vector3 (const Vector3& V) : x(V.x), y(V.y), z(V.z) {
}

//----------------------------------------------------------------------------

//inline Vector3::Vector3 (const __m128& m) {
    // Cast from SSE packed floats
//    *this = *(Vector3*)&m;
//}

//----------------------------------------------------------------------------
inline const float& Vector3::operator[] (int i) const {
    return ((float*)this)[i];
}

inline float& Vector3::operator[] (int i) {
    return ((float*)this)[i];
}


//----------------------------------------------------------------------------
inline Vector3& Vector3::operator= (const Vector3& rkVector) {
    x = rkVector.x;
    y = rkVector.y;
    z = rkVector.z;
    return *this;
}

//----------------------------------------------------------------------------

inline bool Vector3::fuzzyEq(const Vector3& other) const {
    return G3D::fuzzyEq((*this - other).squaredMagnitude(), 0);
}

//----------------------------------------------------------------------------

inline bool Vector3::fuzzyNe(const Vector3& other) const {
    return G3D::fuzzyNe((*this - other).squaredMagnitude(), 0);
}

//----------------------------------------------------------------------------

inline bool Vector3::isFinite() const {
    return G3D::isFinite(x) && G3D::isFinite(y) && G3D::isFinite(z);
}

//----------------------------------------------------------------------------
inline bool Vector3::operator== (const Vector3& rkVector) const {
    return ( x == rkVector.x && y == rkVector.y && z == rkVector.z );
}

//----------------------------------------------------------------------------
inline bool Vector3::operator!= (const Vector3& rkVector) const {
    return ( x != rkVector.x || y != rkVector.y || z != rkVector.z );
}

//----------------------------------------------------------------------------
inline Vector3 Vector3::operator+ (const Vector3& rkVector) const {
    return Vector3(x + rkVector.x, y + rkVector.y, z + rkVector.z);
}

//----------------------------------------------------------------------------
inline Vector3 Vector3::operator- (const Vector3& rkVector) const {
    return Vector3(x - rkVector.x, y - rkVector.y, z - rkVector.z);
}

//----------------------------------------------------------------------------
inline Vector3 Vector3::operator* (const Vector3& rkVector) const {
    return Vector3(x * rkVector.x, y * rkVector.y, z * rkVector.z);
}

inline Vector3 Vector3::operator*(float f) const {
    return Vector3(x * f, y * f, z * f);
}

//----------------------------------------------------------------------------
inline Vector3 Vector3::operator/ (const Vector3& rkVector) const {
    return Vector3(x / rkVector.x, y / rkVector.y, z / rkVector.z);
}

//----------------------------------------------------------------------------
inline Vector3 Vector3::operator- () const {
    return Vector3(-x, -y, -z);
}

//----------------------------------------------------------------------------
inline Vector3& Vector3::operator+= (const Vector3& rkVector) {
    x += rkVector.x;
    y += rkVector.y;
    z += rkVector.z;
    return *this;
}

//----------------------------------------------------------------------------
inline Vector3& Vector3::operator-= (const Vector3& rkVector) {
    x -= rkVector.x;
    y -= rkVector.y;
    z -= rkVector.z;
    return *this;
}

//----------------------------------------------------------------------------
inline Vector3& Vector3::operator*= (float fScalar) {
    x *= fScalar;
    y *= fScalar;
    z *= fScalar;
    return *this;
}

//----------------------------------------------------------------------------
inline Vector3& Vector3::operator*= (const Vector3& rkVector) {
    x *= rkVector.x;
    y *= rkVector.y;
    z *= rkVector.z;
    return *this;
}

//----------------------------------------------------------------------------
inline Vector3& Vector3::operator/= (const Vector3& rkVector) {
    x /= rkVector.x;
    y /= rkVector.y;
    z /= rkVector.z;
    return *this;
}

//----------------------------------------------------------------------------
inline float Vector3::squaredMagnitude () const {
    return x*x + y*y + z*z;
}

//----------------------------------------------------------------------------
inline float Vector3::squaredLength () const {
    return squaredMagnitude();
}

//----------------------------------------------------------------------------
inline float Vector3::magnitude() const {
    return sqrtf(x*x + y*y + z*z);
}

//----------------------------------------------------------------------------
inline float Vector3::length() const {
    return magnitude();
}

//----------------------------------------------------------------------------
inline Vector3 Vector3::direction () const {
    float lenSquared = squaredMagnitude();
    float invSqrt = 1.0f / sqrtf(lenSquared);
    return Vector3(x * invSqrt, y * invSqrt, z * invSqrt);
}

//----------------------------------------------------------------------------

inline Vector3 Vector3::fastDirection () const {
    float lenSquared = x * x + y * y + z * z;
    float invSqrt = rsq(lenSquared);
    return Vector3(x * invSqrt, y * invSqrt, z * invSqrt);
}

//----------------------------------------------------------------------------
inline float Vector3::dot (const Vector3& rkVector) const {
    return x*rkVector.x + y*rkVector.y + z*rkVector.z;
}

//----------------------------------------------------------------------------
inline Vector3 Vector3::cross (const Vector3& rkVector) const {
    return Vector3(y*rkVector.z - z*rkVector.y, z*rkVector.x - x*rkVector.z,
                   x*rkVector.y - y*rkVector.x);
}

//----------------------------------------------------------------------------
inline Vector3 Vector3::unitCross (const Vector3& rkVector) const {
    Vector3 kCross(y*rkVector.z - z*rkVector.y, z*rkVector.x - x*rkVector.z,
                   x*rkVector.y - y*rkVector.x);
    kCross.unitize();
    return kCross;
}

//----------------------------------------------------------------------------
inline Vector3 Vector3::min(const Vector3 &v) const {
    return Vector3(std::min(v.x, x), std::min(v.y, y), std::min(v.z, z));
}

//----------------------------------------------------------------------------
inline Vector3 Vector3::max(const Vector3 &v) const {
    return Vector3(std::max(v.x, x), std::max(v.y, y), std::max(v.z, z));
}

//----------------------------------------------------------------------------
inline bool Vector3::isZero() const {
    return G3D::fuzzyEq(squaredMagnitude(), 0.0f);
}

//----------------------------------------------------------------------------

inline bool Vector3::isUnit() const {
    return G3D::fuzzyEq(squaredMagnitude(), 1.0f);
}

} // namespace
