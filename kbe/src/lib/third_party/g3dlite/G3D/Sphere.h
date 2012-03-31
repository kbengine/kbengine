/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef G3D_SPHERE_H
#define G3D_SPHERE_H

#include "platform.h"
#include "Vector3.h"
#include "Array.h"
#include "Sphere.h"

namespace G3D {

/**
 Sphere.
 */
class Sphere {
private:

    static int32     dummy;

public:
    Vector3          center;
    float            radius;

    Sphere() {
        center = Vector3::zero();
        radius = 0;
    }

    Sphere(
        const Vector3&  center,
        float           radius) {

        this->center = center;
        this->radius = radius;
    }

    virtual ~Sphere() {}

    bool operator==(const Sphere& other) const {
        return (center == other.center) && (radius == other.radius);
    }

    bool operator!=(const Sphere& other) const {
        return !((center == other.center) && (radius == other.radius));
    }

    /**
     Returns true if point is less than or equal to radius away from
     the center.
     */
    bool contains(const Vector3& point) const;

/**
	 @deprecated Use culledBy(Array<Plane>&)
     */
    bool culledBy(
        const class Plane*  plane,
        int                 numPlanes,
		int32&				cullingPlaneIndex,
		const uint32  		testMask,
        uint32&             childMask) const;

    /**
	 @deprecated Use culledBy(Array<Plane>&)
     */
    bool culledBy(
        const class Plane*  plane,
        int                 numPlanes,
		int32&				cullingPlaneIndex = dummy,
		const uint32  		testMask = -1) const;

	/**
      See AABox::culledBy
	 */
	bool culledBy(
		const Array<Plane>&		plane,
		int32&					cullingPlaneIndex,
		const uint32  			testMask,
        uint32&                 childMask) const;

    /**
     Conservative culling test that does not produce a mask for children.
     */
	bool culledBy(
		const Array<Plane>&		plane,
		int32&					cullingPlaneIndex = dummy,
		const uint32  			testMask		  = -1) const;
    virtual std::string toString() const;

    float volume() const;

    /** @deprecated */
    float surfaceArea() const;

    inline float area() const {
        return surfaceArea();
    }

    /**
     Uniformly distributed on the surface.
     */
    Vector3 randomSurfacePoint() const;

    /**
     Uniformly distributed on the interior (includes surface)
     */
    Vector3 randomInteriorPoint() const;

    void getBounds(class AABox& out) const;
};

} // namespace

inline size_t hashCode(const G3D::Sphere& sphere) {
    return (size_t)(hashCode(sphere.center) + (sphere.radius * 13));
}

#endif
