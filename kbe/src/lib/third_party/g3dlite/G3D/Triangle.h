/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef G3D_TRIANGLE_H
#define G3D_TRIANGLE_H

#include "platform.h"
#include "g3dmath.h"
#include "Vector3.h"
#include "Plane.h"
#include <string>

namespace G3D {

/**
  A generic triangle representation.  This should not be used
  as the underlying triangle for creating models; it is intended
  for providing fast property queries but requires a lot of
  storage and is mostly immutable.
  */
class Triangle {
private:
    friend class CollisionDetection;
    friend class Ray;

    Vector3                     _vertex[3];

    /** edgeDirection[i] is the normalized vector v[i+1] - v[i] */
    Vector3                     edgeDirection[3];
    double                      edgeMagnitude[3];
    Plane                       _plane;
    Vector3::Axis               _primaryAxis;

    /** vertex[1] - vertex[0] */
    Vector3                     edge01;
    /** vertex[2] - vertex[0] */
    Vector3                     edge02;

    float                       _area;

    void init(const Vector3& v0, const Vector3& v1, const Vector3& v2);

public:
	
    Triangle();
    
    Triangle(const Vector3& v0, const Vector3& v1, const Vector3& v2);
    
    ~Triangle();

    /** 0, 1, or 2 */
    inline const Vector3& vertex(int n) const {
        debugAssert((n >= 0) && (n < 3));
        return _vertex[n];
    }

    double area() const;

    Vector3::Axis primaryAxis() const {
        return _primaryAxis;
    }

    const Vector3& normal() const;

    /** Barycenter */
    Vector3 center() const;

    const Plane& plane() const;

    /** Returns a random point in the triangle. */
    Vector3 randomPoint() const;

    /**
     For two triangles to be equal they must have
     the same vertices <I>in the same order</I>.
     That is, vertex[0] == vertex[0], etc.
     */
    inline bool operator==(const Triangle& other) {
        for (int i = 0; i < 3; ++i) {
            if (_vertex[i] != other._vertex[i]) {
                return false;
            }
        }

        return true;
    }

    inline size_t hashCode() const {
        return
            _vertex[0].hashCode() +
            (_vertex[1].hashCode() >> 2) +
            _vertex[2].hashCode();
    }

    void getBounds(class AABox&) const;

};

} // namespace

inline size_t hashCode(const G3D::Triangle& t) {
	return t.hashCode();
}

#endif
