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

#include "navigation_handle.hpp"	
#include "navigation/navigation.hpp"
#include "resmgr/resmgr.hpp"
#include "thread/threadguard.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
NavMeshHandle::NavMeshHandle():
NavigationHandle()
{
}

//-------------------------------------------------------------------------------------
NavMeshHandle::~NavMeshHandle()
{
	dtFreeNavMeshQuery(navmeshQuery);
	dtFreeNavMesh(navmesh);

	DEBUG_MSG(boost::format("NavMeshHandle::~NavMeshHandle(): (%1%) is destroyed!\n") % name);
}

//-------------------------------------------------------------------------------------
int NavMeshHandle::findStraightPath(const Position3D& start, const Position3D& end, std::vector<Position3D>& paths)
{
	float spos[3];
	spos[0] = start.x;
	spos[1] = start.y;
	spos[2] = start.z;

	float epos[3];
	epos[0] = end.x;
	epos[1] = end.y;
	epos[2] = end.z;

	dtQueryFilter filter;
	filter.setIncludeFlags(0xffff);
	filter.setExcludeFlags(0);

	const float extents[3] = {2.f, 4.f, 2.f};

	dtPolyRef startRef = INVALID_NAVMESH_POLYREF;
	dtPolyRef endRef = INVALID_NAVMESH_POLYREF;

	float nearestPt[3];
	dtStatus status = navmeshQuery->findNearestPoly(spos, extents, &filter, &startRef, nearestPt);
	status = navmeshQuery->findNearestPoly(epos, extents, &filter, &endRef, nearestPt);

	if (!startRef || !endRef)
	{
		ERROR_MSG(boost::format("NavMeshHandle::findStraightPath(%3%): Could not find any nearby poly's (%1%, %2%)\n") % startRef % endRef % name);
		return NAV_ERROR_NEARESTPOLY;
	}

	dtPolyRef polys[MAX_POLYS];
	int npolys;
	float straightPath[MAX_POLYS * 3];
	unsigned char straightPathFlags[MAX_POLYS];
	dtPolyRef straightPathPolys[MAX_POLYS];
	int nstraightPath;
	int pos = 0;

	status = navmeshQuery->findPath(startRef, endRef, spos, epos, &filter, polys, &npolys, MAX_POLYS);
	nstraightPath = 0;

	if (npolys)
	{
		float epos1[3];
		dtVcopy(epos1, epos);
				
		if (polys[npolys-1] != endRef)
			navmeshQuery->closestPointOnPoly(polys[npolys-1], epos, epos1);
				
		status = navmeshQuery->findStraightPath(spos, epos, polys, npolys, straightPath, straightPathFlags, straightPathPolys, &nstraightPath, MAX_POLYS);
		for(int i = 0; i < nstraightPath * 3; )
		{
			Position3D currpos;
			currpos.x = straightPath[i++];
			currpos.y = straightPath[i++];
			currpos.z = straightPath[i++];
			paths.push_back(currpos);
			pos++; 
			
			//DEBUG_MSG(boost::format("NavMeshHandle::findStraightPath: %1%->%2%, %3%, %4%\n") % pos % currpos.x % currpos.y % currpos.z);
		}
	}

	return pos;
}

//-------------------------------------------------------------------------------------
int NavMeshHandle::raycast(const Position3D& start, const Position3D& end, float* hitPoint)
{
	float spos[3];
	spos[0] = start.x;
	spos[1] = start.y;
	spos[2] = start.z;

	float epos[3];
	epos[0] = end.x;
	epos[1] = end.y;
	epos[2] = end.z;

	dtQueryFilter filter;
	filter.setIncludeFlags(0xffff);
	filter.setExcludeFlags(0);

	const float extents[3] = {2.f, 4.f, 2.f};

	dtPolyRef startRef = INVALID_NAVMESH_POLYREF;

	float nearestPt[3];
	navmeshQuery->findNearestPoly(spos, extents, &filter, &startRef, nearestPt);

	if (!startRef)
	{
		return NAV_ERROR_NEARESTPOLY;
	}

	float t = 0;
	float hitNormal[3];
	memset(hitNormal, 0, sizeof(hitNormal));

	dtPolyRef polys[MAX_POLYS];
	int npolys;

	navmeshQuery->raycast(startRef, spos, epos, &filter, &t, hitNormal, polys, &npolys, MAX_POLYS);

	if (t > 1)
	{
		// no hit
		return NAV_ERROR;
	}
	else
	{
		// Hit
		hitPoint[0] = spos[0] + (epos[0] - spos[0]) * t;
		hitPoint[1] = spos[1] + (epos[1] - spos[1]) * t;
		hitPoint[2] = spos[2] + (epos[2] - spos[2]) * t;
		if (npolys)
		{
			float h = 0;
			navmeshQuery->getPolyHeight(polys[npolys-1], hitPoint, &h);
			hitPoint[1] = h;
		}
	}
	
	return 1;
}

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------
NavigationHandle* NavMeshHandle::create(std::string name)
{
	if(name == "")
		return NULL;

	std::string path = Resmgr::getSingleton().matchRes("spaces/" + name + "/" + name + ".navmesh");

	FILE* fp = fopen(path.c_str(), "rb");
	if (!fp)
	{
		ERROR_MSG(boost::format("Navigation::loadNavmesh: open(%1%) is error!\n") % path);
		return NULL;
	}

	bool safeStorage = true;
    int pos = 0;
    int size = sizeof(NavMeshSetHeader);
	
	fseek(fp, 0, SEEK_END); 
	size_t flen = ftell(fp); 
	fseek(fp, 0, SEEK_SET); 

	uint8* data = new uint8[flen];
	if(data == NULL)
	{
		ERROR_MSG(boost::format("Navigation::loadNavmesh: open(%1%), memory(size=%2%) error!\n") % path % flen);
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return NULL;
	}

	size_t readsize = fread(data, 1, flen, fp);
	if(readsize != flen)
	{
		ERROR_MSG(boost::format("Navigation::loadNavmesh: open(%1%), read(size=%2% != %3%) error!\n") % path % readsize % flen);
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return NULL;
	}

    if (readsize < sizeof(NavMeshSetHeader))
	{
		ERROR_MSG(boost::format("Navigation::loadNavmesh: open(%1%), NavMeshSetHeader is error!\n") % path);
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return NULL;
	}

    NavMeshSetHeader header;
	memcpy(&header, data, size);

    pos += size;

	if (header.version != NavMeshHandle::RCN_NAVMESH_VERSION)
    {
		ERROR_MSG(boost::format("Navigation::loadNavmesh: version(%1%) is not match(%2%)!\n") % header.version % ((int)NavMeshHandle::RCN_NAVMESH_VERSION));
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
        return NULL;
    }

    dtNavMesh* mesh = dtAllocNavMesh();
    if (!mesh)
    {
		ERROR_MSG("Navigation::loadNavmesh: dtAllocNavMesh is failed!\n");
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
        return NULL;
    }

    dtStatus status = mesh->init(&header.params);
    if (dtStatusFailed(status))
    {
		ERROR_MSG(boost::format("Navigation::loadNavmesh: mesh init is error(%1%)!\n") % status);
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
	    return NULL;
    }

    // Read tiles.
    bool success = true;
    for (int i = 0; i < header.tileCount; ++i)
    {
	    NavMeshTileHeader tileHeader;
        size = sizeof(NavMeshTileHeader);
	    memcpy(&tileHeader, &data[pos], size);
        pos += size;

        size = tileHeader.dataSize;
		if (!tileHeader.tileRef || !tileHeader.dataSize)
        {
            success = false;
            status = DT_FAILURE + DT_INVALID_PARAM;
		    break;
        }
		
	    unsigned char* tileData = 
            (unsigned char*)dtAlloc(size, DT_ALLOC_PERM);
	    if (!tileData)
        {
            success = false;
            status = DT_FAILURE + DT_OUT_OF_MEMORY;
            break;
        }
        memcpy(tileData, &data[pos], size);
        pos += size;

	    status = mesh->addTile(tileData
            , size
			, (safeStorage ? DT_TILE_FREE_DATA : 0)
            , tileHeader.tileRef
            , 0);

        if (dtStatusFailed(status))
        {
            success = false;
            break;
        }
    }

	fclose(fp);
	SAFE_RELEASE_ARRAY(data);

    if (!success)
    {
		ERROR_MSG(boost::format("Navigation::loadNavmesh:  error(%1%)!\n") % status);
        dtFreeNavMesh(mesh);
		return NULL;
    }

	NavMeshHandle* pNavMeshHandle = new NavMeshHandle();
	pNavMeshHandle->navmesh = mesh;
	pNavMeshHandle->navmeshQuery = new dtNavMeshQuery();
	pNavMeshHandle->navmeshQuery->init(mesh, 1024);
	pNavMeshHandle->name = name;

    uint32 tileCount = 0;
    uint32 nodeCount = 0;
    uint32 polyCount = 0;
    uint32 vertCount = 0;
    uint32 triCount = 0;
    uint32 triVertCount = 0;
    uint32 dataSize = 0;

	const dtNavMesh* navmesh = mesh;
    for (int32 i = 0; i < navmesh->getMaxTiles(); ++i)
    {
        const dtMeshTile* tile = navmesh->getTile(i);
        if (!tile || !tile->header)
            continue;

        tileCount ++;
        nodeCount += tile->header->bvNodeCount;
        polyCount += tile->header->polyCount;
        vertCount += tile->header->vertCount;
        triCount += tile->header->detailTriCount;
        triVertCount += tile->header->detailVertCount;
        dataSize += tile->dataSize;

		// DEBUG_MSG(boost::format("Navigation::loadNavmesh: verts(%1%, %2%, %3%)\n") % tile->verts[0] % tile->verts[1] % tile->verts[2]);
    }

	DEBUG_MSG(boost::format("Navigation::loadNavmesh: (%1%)\n") % name);
	DEBUG_MSG(boost::format("\t==> tiles loaded: %1%\n") % tileCount);
	DEBUG_MSG(boost::format("\t==> BVTree nodes: %1%\n") % nodeCount);
    DEBUG_MSG(boost::format("\t==> %1% polygons (%2% vertices)\n") % polyCount % vertCount);
    DEBUG_MSG(boost::format("\t==> %1% triangles (%2% vertices)\n") % triCount % triVertCount);
    DEBUG_MSG(boost::format("\t==> %.2f MB of data (not including pointers)\n") % (((float)dataSize / sizeof(unsigned char)) / 1048576));
	return pNavMeshHandle;
}

//-------------------------------------------------------------------------------------
NavTileHandle::NavTileHandle():
NavigationHandle()
{
}

//-------------------------------------------------------------------------------------
NavTileHandle::~NavTileHandle()
{
	DEBUG_MSG(boost::format("NavTileHandle::~NavTileHandle(): (%1%) is destroyed!\n") % name);
}

//-------------------------------------------------------------------------------------
int NavTileHandle::findStraightPath(const Position3D& start, const Position3D& end, std::vector<Position3D>& paths)
{
	return 0;
}

//-------------------------------------------------------------------------------------
int NavTileHandle::raycast(const Position3D& start, const Position3D& end, float* hitPoint)
{
	return 0;
}

//-------------------------------------------------------------------------------------
NavigationHandle* NavTileHandle::create(std::string name)
{
	return NULL;
}

//-------------------------------------------------------------------------------------
}

