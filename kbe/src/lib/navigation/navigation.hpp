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
#ifndef __KBE_NAVIGATION_HPP__
#define __KBE_NAVIGATION_HPP__

#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/singleton.hpp"
#include "math/math.hpp"

#include "DetourNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "DetourCommon.h"
#include "DetourNavMesh.h"

namespace KBEngine
{

class NavigationHandle
{
public:
	enum NAV_TYPE
	{
		NAV_UNKNOWN = 0,
		NAV_MESH = 1,
		NAV_TILE_BASED = 2
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
public:
	NavMeshHandle();
	virtual ~NavMeshHandle();

	int findStraightPath(const Position3D& start, const Position3D& end, std::vector<Position3D>& paths);
	int raycast(const Position3D& start, const Position3D& end, float* hitPoint);

	virtual NavigationHandle::NAV_TYPE type() const{ return NAV_MESH; }

	dtNavMesh* navmesh;
	dtNavMeshQuery* navmeshQuery;
};

/*
	navmesh
*/
class Navigation : public Singleton<Navigation>
{
public:
	Navigation();
	virtual ~Navigation();
	
	NavigationHandle* loadNavigation(std::string name);
	NavMeshHandle* loadNavmesh(std::string name);

	bool hasNavigation(std::string name);

	bool removeNavigation(std::string name);

	NavigationHandle* findNavigation(std::string name);
private:
	KBEUnordered_map<std::string, NavigationHandle*> navhandles_;
	KBEngine::thread::ThreadMutex mutex_;
};

}
#endif
