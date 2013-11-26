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
#ifndef __KBE_NAVMESHEX_HPP__
#define __KBE_NAVMESHEX_HPP__

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

class NavMeshHandle
{
public:
	static const int NAV_ERROR = -1;
	static const int NAV_ERROR_NEARESTPOLY = -2;
public:
	int findStraightPath(Position3D start, Position3D end, std::vector<Position3D>& paths);

	dtNavMesh* navmesh;
	dtNavMeshQuery* navmeshQuery;
	std::string name;
};

/*
	navmesh
*/
class NavMeshEx : public Singleton<NavMeshEx>
{
public:
	NavMeshEx();
	virtual ~NavMeshEx();
	
	bool loadNavmesh(std::string respath);

	bool hasNavmesh(std::string name);

	bool removeNavmesh(std::string name);

	NavMeshHandle* findNavmesh(std::string name);
private:
	KBEUnordered_map<std::string, NavMeshHandle*> navmeshs_;
};

}
#endif
