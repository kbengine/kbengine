/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __KBE_MATH__
#define __KBE_MATH__

// common include	
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include "cstdkbe/cstdkbe.hpp"

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

#define KBE_PI									G3D::G3D_PI
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

typedef Vector3													Position3D;												// 表示3D位置变量类型																						// mailbox 所投递的mail类别的类别

struct Direction3D																										// 表示方向位置变量类型
{
	Direction3D():roll(0.0f), pitch(0.0f), yaw(0.0f) {};
	Direction3D(const Vector3 & v):roll(v[0]), pitch(v[1]), yaw(v[2]) {}
	Vector3 asVector3() const { return Vector3(roll, pitch, yaw); }

	float roll;		
	float pitch;	
	float yaw;		
};

/** entity 详情级别类型定义 */
struct DetailLevel
{
	struct Level
	{
		Level():radius(0.0f), hyst(0.0f){};
		float radius;
		float hyst;
	};
	
	DetailLevel()
	{
		level[0] = NULL;
		level[1] = NULL;
		level[2] = NULL;
		level[3] = NULL;
	}

	~DetailLevel()
	{
		SAFE_RELEASE(level[0]);
		SAFE_RELEASE(level[1]);
		SAFE_RELEASE(level[2]);
		SAFE_RELEASE(level[3]);
	}




	Level* level[4];
};

#endif
