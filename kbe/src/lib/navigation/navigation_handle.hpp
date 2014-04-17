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

#ifndef __KBE_NAVIGATEHANDLE_HPP__
#define __KBE_NAVIGATEHANDLE_HPP__

#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/singleton.hpp"
#include "math/math.hpp"

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

struct NavMeshTileHeader
{
	dtTileRef tileRef;
	int dataSize;
};

class NavigationHandle
{
public:
	enum NAV_TYPE
	{
		NAV_UNKNOWN = 0,
		NAV_MESH = 1,
		NAV_TILE = 2
	};

	NavigationHandle():name()
	{
	}

	virtual ~NavigationHandle()
	{
	}

	virtual NavigationHandle::NAV_TYPE type() const{ return NAV_UNKNOWN; }

	virtual int findStraightPath(const Position3D& start, const Position3D& end, std::vector<Position3D>& paths) = 0;
	virtual int raycast(const Position3D& start, const Position3D& end, float* hitPoint) = 0;

	std::string name;
};

class NavMeshHandle : public NavigationHandle
{
public:
	static const int MAX_POLYS = 256;

	static const int NAV_ERROR = -1;
	static const int NAV_ERROR_NEARESTPOLY = -2;

	static const long RCN_NAVMESH_VERSION = 1;
	static const int INVALID_NAVMESH_POLYREF = 0;
public:
	NavMeshHandle();
	virtual ~NavMeshHandle();

	int findStraightPath(const Position3D& start, const Position3D& end, std::vector<Position3D>& paths);
	int raycast(const Position3D& start, const Position3D& end, float* hitPoint);

	virtual NavigationHandle::NAV_TYPE type() const{ return NAV_MESH; }

	static NavigationHandle* create(std::string name);

	dtNavMesh* navmesh;
	dtNavMeshQuery* navmeshQuery;
};

class NavTileHandle : public NavigationHandle
{
public:
	NavTileHandle();
	virtual ~NavTileHandle();

	int findStraightPath(const Position3D& start, const Position3D& end, std::vector<Position3D>& paths);
	int raycast(const Position3D& start, const Position3D& end, float* hitPoint);

	virtual NavigationHandle::NAV_TYPE type() const{ return NAV_TILE; }

	static NavigationHandle* create(std::string name);
	
	Tmx::Map *pTilemap;
};

}
#endif // __KBE_NAVIGATEHANDLE_HPP__

