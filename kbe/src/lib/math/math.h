// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_MATH_H
#define KBE_MATH_H

#include <string>
#include "common/common.h"

// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
//#define USE_D3DX
#if KBE_PLATFORM == PLATFORM_WIN32 && defined(USE_D3DX)
#pragma comment(lib,"d3dx9.lib")
#include <d3dx9math.h>
typedef D3DXMATRIX								Matrix;
typedef D3DXQUATERNION							Quaternion;
typedef D3DXVECTOR2								Vector2;
typedef D3DXVECTOR3								Vector3;
typedef D3DXVECTOR4								Vector4;

#define KBE_PI									D3DX_PI
#define KBE_2PI									KBE_PI * 2
#define KBE_DegreeToRadian						D3DXToRadian
#define KBE_RadianToDegree						D3DXToDegree
	
#define KBEVec2Length							D3DXVec2Length
#define KBEVec2LengthSq							D3DXVec2LengthSq
#define KBEVec2Normalize						D3DXVec2Normalize
#define KBEVec2Dot								D3DXVec2Dot
#define KBEVec3Length							D3DXVec3Length
#define KBEVec3LengthSq							D3DXVec3LengthSq
#define KBEVec3Dot								D3DXVec3Dot
#define KBEVec3Cross							D3DXVec3Cross
#define KBEVec3Normalize						D3DXVec3Normalize
#define KBEVec3Lerp								D3DXVec3Lerp
#define KBEVec3Transform						D3DXVec3Transform
#define KBEVec3TransformCoord					D3DXVec3TransformCoord
#define KBEVec3TransformNormal					D3DXVec3TransformNormal
#define KBEVec4Transform						D3DXVec4Transform
#define KBEVec4Length							D3DXVec4Length
#define KBEVec4LengthSq							D3DXVec4LengthSq
#define KBEVec4Normalize						D3DXVec4Normalize
#define KBEVec4Lerp								D3DXVec4Lerp
#define KBEVec4Dot								D3DXVec4Dot
#define KBEMatrixIdentity						D3DXMatrixIdentity
#define KBEMatrixInverse						D3DXMatrixInverse
#define KBEMatrixRotationQuaternion				D3DXMatrixRotationQuaternion
#define KBEMatrixTranspose						D3DXMatrixTranspose
#define KBEMatrixfDeterminant					D3DXMatrixDeterminant
#define KBEMatrixOrthoLH						D3DXMatrixOrthoLH
#define KBEMatrixLookAtLH						D3DXMatrixLookAtLH
#define KBEMatrixMultiply						D3DXMatrixMultiply
#define KBEMatrixPerspectiveFovLH				D3DXMatrixPerspectiveFovLH
#define KBEMatrixRotationX						D3DXMatrixRotationX
#define KBEMatrixRotationY						D3DXMatrixRotationY
#define KBEMatrixRotationZ						D3DXMatrixRotationZ
#define KBEMatrixScaling						D3DXMatrixScaling
#define KBEMatrixTranslation					D3DXMatrixTranslation
#define KBEMatrixRotationYawPitchRoll			D3DXMatrixRotationYawPitchRoll
#define KBEQuaternionDot						D3DXQuaternionDot
#define KBEQuaternionNormalize					D3DXQuaternionNormalize
#define KBEQuaternionRotationMatrix				D3DXQuaternionRotationMatrix
#define KBEQuaternionSlerp						D3DXQuaternionSlerp
#define KBEQuaternionRotationAxis				D3DXQuaternionRotationAxis
#define KBEQuaternionMultiply					D3DXQuaternionMultiply
#define KBEQuaternionInverse					D3DXQuaternionInverse

template <class TYPE>
inline TYPE KBEClamp(TYPE value, TYPE minValue, TYPE maxValue)
{
	return value < minValue ? minValue :
	( value > maxValue ? maxValue : value);
}

#else
#include "G3D/g3dmath.h"
#include "G3D/Vector2.h"
#include "G3D/Vector3.h"
#include "G3D/Vector4.h"
#include "G3D/Matrix3.h"
#include "G3D/Quat.h"

typedef G3D::Matrix3							Matrix;
typedef G3D::Quat								Quaternion;
typedef G3D::Vector2							Vector2;
typedef G3D::Vector3							Vector3;
typedef G3D::Vector4							Vector4;

#define KBE_PI									3.1415926535898
#define KBE_2PI									6.2831853071796
#define KBE_DegreeToRadian						G3D::toRadians
#define KBE_RadianToDegree						G3D::toRadians
	
#define KBEVec2Length(v)						(v)->length()
#define KBEVec2LengthSq(v)						(v)->squaredLength()
#define KBEVec2Normalize(v, vv)					(v)->normalise()
#define KBEVec2Dot(v, vv)						(v)->dot(static_cast<const G3D::Vector2 &>(*(vv)))
#define KBEVec3Length(v)						(v)->length()
#define KBEVec3LengthSq(v)						(v)->squaredLength()
#define KBEVec3Dot(v, vv)						(v)->dot(static_cast<const G3D::Vector3 &>(*(vv)))
#define KBEVec3Cross(v)							(v)->cross()
#define KBEVec3Normalize(v, vv)					(v)->normalise()
#define KBEVec3Lerp(v)							(v)->lerp()
#define KBEVec3Transform(v)						D3DXVec3Transform
#define KBEVec3TransformCoord(v)				D3DXVec3TransformCoord
#define KBEVec3TransformNormal(v)				D3DXVec3TransformNormal
#define KBEVec4Transform(v)						D3DXVec4Transform
#define KBEVec4Length(v)						(v)->length()
#define KBEVec4LengthSq(v)						(v)->squaredLength()
#define KBEVec4Normalize(v, vv)					(v)->normalise()
#define KBEVec4Lerp(v)							(v)->lerp()
#define KBEVec4Dot(v, vv)						(v)->dot(static_cast<const G3D::Vector4 &>(*(vv)))
#define KBEMatrixIdentity						Matrix3::identity()
#define KBEMatrixInverse(v)						Matrix3::inverse()
#define KBEMatrixRotationQuaternion(v)			D3DXMatrixRotationQuaternion
#define KBEMatrixTranspose(v)					D3DXMatrixTranspose
#define KBEMatrixfDeterminant(v)				D3DXMatrixDeterminant
#define KBEMatrixOrthoLH(v)						D3DXMatrixOrthoLH
#define KBEMatrixLookAtLH(v)					D3DXMatrixLookAtLH
#define KBEMatrixMultiply(v)					D3DXMatrixMultiply
#define KBEMatrixPerspectiveFovLH(v)			D3DXMatrixPerspectiveFovLH
#define KBEMatrixRotationX(v)					D3DXMatrixRotationX
#define KBEMatrixRotationY(v)					D3DXMatrixRotationY
#define KBEMatrixRotationZ(v)					D3DXMatrixRotationZ
#define KBEMatrixScaling(v)						D3DXMatrixScaling
#define KBEMatrixTranslation(v)					D3DXMatrixTranslation
#define KBEMatrixRotationYawPitchRoll(v)		D3DXMatrixRotationYawPitchRoll
#define KBEQuaternionDot(v)						D3DXQuaternionDot
#define KBEQuaternionNormalize(v)				D3DXQuaternionNormalize
#define KBEQuaternionRotationMatrix(v)			D3DXQuaternionRotationMatrix
#define KBEQuaternionSlerp(v)					D3DXQuaternionSlerp
#define KBEQuaternionRotationAxis(v)			D3DXQuaternionRotationAxis
#define KBEQuaternionMultiply(v)				D3DXQuaternionMultiply
#define KBEQuaternionInverse(v)					D3DXQuaternionInverse
#define KBEClamp								G3D::clamp
#endif

// 从2个3d向量忽略y计算出2d长度
inline float KBEVec3CalcVec2Length(const Vector3& v1, const Vector3& v2)
{
	float x = v1.x - v2.x;
	float z = v1.z - v2.z;
	return sqrt(x*x + z*z);
}

inline float int82angle(KBEngine::int8 angle, bool half = false)
{
	return float(angle) * float((KBE_PI / (half ? 254.f : 128.f)));
}

inline KBEngine::int8 angle2int8(float v, bool half = false)
{
	KBEngine::int8 angle = 0;
	if(!half)
	{
		angle = (KBEngine::int8)floorf((v * 128.f) / float(KBE_PI) + 0.5f);
	}
	else
	{
		angle = (KBEngine::int8)KBEClamp(floorf( (v * 254.f) / float(KBE_PI) + 0.5f ), -128.f, 127.f );
	}

	return angle;
}

typedef Vector3													Position3D;												// 表示3D位置变量类型	
typedef KBEShared_ptr< std::vector<Position3D> >				VECTOR_POS3D_PTR;										// 指向Position3D数组的智能指针类型声明

struct Direction3D																										// 表示方向位置变量类型
{
	Direction3D():dir(0.f, 0.f, 0.f) {};
	Direction3D(const Vector3 & v):dir(v){}
	Direction3D(float r, float p, float y):dir(r, p, y){}
	Direction3D(const Direction3D & v) :dir(v.dir){}

	Direction3D& operator=(const Direction3D& v)
	{
		dir = v.dir;
		return *this;
	}

	float roll() const{ return dir.x; }		
	float pitch() const{ return dir.y; }		
	float yaw() const{ return dir.z; }		

	void roll(float v){ dir.x = v; }		
	void pitch(float v){ dir.y = v; }		
	void yaw(float v){ dir.z = v; }	

	// roll, pitch, yaw
	Vector3 dir;
};

/** 浮点数比较 */
#define floatEqual(v1, v3) (abs(v1 - v2) < std::numeric_limits<float>::epsilon())
inline bool almostEqual(const float f1, const float f2, const float epsilon = 0.0004f)
{
	return fabsf( f1 - f2 ) < epsilon;
}

inline bool almostEqual(const double d1, const double d2, const double epsilon = 0.0004)
{
	return fabs( d1 - d2 ) < epsilon;
}

inline bool almostZero(const float f, const float epsilon = 0.0004f)
{
	return f < epsilon && f > -epsilon;
}

inline bool almostZero(const double d, const double epsilon = 0.0004)
{
	return d < epsilon && d > -epsilon;
}

template<typename T>
inline bool almostEqual(const T& c1, const T& c2, const float epsilon = 0.0004f)
{
	if( c1.size() != c2.size() )
		return false;
	typename T::const_iterator iter1 = c1.begin();
	typename T::const_iterator iter2 = c2.begin();
	for( ; iter1 != c1.end(); ++iter1, ++iter2 )
		if( !almostEqual( *iter1, *iter2, epsilon ) )
			return false;
	return true;
}

#endif // KBE_MATH_H
