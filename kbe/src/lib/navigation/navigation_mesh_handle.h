/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_NAVIGATEMESHHANDLE_H
#define KBE_NAVIGATEMESHHANDLE_H

#include "navigation/navigation_handle.h"

#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "DetourCommon.h"
#include "DetourNavMesh.h"

namespace KBEngine{

struct NavMeshSetHeader
{
	int version;
	int tileCount;
	dtNavMeshParams params;
};

struct NavMeshSetHeaderEx
{
	int magic;
	int version;
	int tileCount;
	dtNavMeshParams params;
};

struct NavMeshTileHeader
{
	dtTileRef tileRef;
	int dataSize;
};

class NavMeshHandle : public NavigationHandle
{
public:
	static const int MAX_POLYS = 256;
	static const int NAV_ERROR_NEARESTPOLY = -2;

	static const long RCN_NAVMESH_VERSION = 1;
	static const int INVALID_NAVMESH_POLYREF = 0;
	
	struct NavmeshLayer
	{
		dtNavMesh* pNavmesh;
		dtNavMeshQuery* pNavmeshQuery;
	};

public:
	NavMeshHandle();
	virtual ~NavMeshHandle();

	int findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths);

	int findRandomPointAroundCircle(int layer, const Position3D& centerPos, std::vector<Position3D>& points, 
		uint32 max_points, float maxRadius);

	int raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPointVec);

	virtual NavigationHandle::NAV_TYPE type() const{ return NAV_MESH; }

	static NavigationHandle* create(std::string resPath, const std::map< int, std::string >& params);
	static bool _create(int layer, const std::string& resPath, const std::string& res, NavMeshHandle* pNavMeshHandle);
	
	std::map<int, NavmeshLayer> navmeshLayer;
private:
	/* Derives overlap polygon of two polygon on the xz-plane.
		@param[in]		polyVertsA		Vertices of polygon A.
		@param[in]		nPolyVertsA		Vertices number of polygon A.
		@param[in]		polyVertsB		Vertices of polygon B.
		@param[in]		nPolyVertsB		Vertices number of polygon B.
		@param[out]		intsectPt		Vertices of overlap polygon.
		@param[out]		intsectPtCount	Vertices number of overlap polygon.
	*/
	void getOverlapPolyPoly2D(const float* polyVertsA, const int nPolyVertsA, const float* polyVertsB, const int nPolyVertsB, float* intsectPt, int* intsectPtCount);

	/* Sort vertices to clockwise. */
	void clockwiseSortPoints(float* verts, const int nVerts);

	/* Determines if two segment cross on xz-plane. */
	bool isSegSegCross2D(const float* p1, const float *p2, const float* q1, const float* q2);
};

}

#endif // KBE_NAVIGATEMESHHANDLE_H

