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

#include "navigation_mesh_handle.h"	
#include "navigation/navigation.h"
#include "resmgr/resmgr.h"
#include "thread/threadguard.h"
#include "math/math.h"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
NavMeshHandle::NavMeshHandle():
NavigationHandle()
{
}

//-------------------------------------------------------------------------------------
NavMeshHandle::~NavMeshHandle()
{
	std::vector<dtNavMesh*>::iterator iter = navmesh_layers.begin();
	for(; iter != navmesh_layers.end(); iter++)
		dtFreeNavMesh((*iter));

	std::vector<dtNavMeshQuery*>::iterator iter1 = navmeshQuery_layers.begin();
	for(; iter1 != navmeshQuery_layers.end(); iter1++)
		dtFreeNavMeshQuery((*iter1));
	
	DEBUG_MSG(fmt::format("NavMeshHandle::~NavMeshHandle(): ({}) is destroyed!\n", name));
}

//-------------------------------------------------------------------------------------
int NavMeshHandle::findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths)
{
	if(layer >= (int)navmeshQuery_layers.size())
	{
		ERROR_MSG(fmt::format("NavMeshHandle::findStraightPath: not found layer({})\n",  layer));
		return NAV_ERROR;
	}

	dtNavMeshQuery* navmeshQuery = navmeshQuery_layers[layer];
	// dtNavMesh* 

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
	navmeshQuery->findNearestPoly(spos, extents, &filter, &startRef, nearestPt);
	navmeshQuery->findNearestPoly(epos, extents, &filter, &endRef, nearestPt);

	if (!startRef || !endRef)
	{
		ERROR_MSG(fmt::format("NavMeshHandle::findStraightPath({2}): Could not find any nearby poly's ({0}, {1})\n", startRef, endRef, name));
		return NAV_ERROR_NEARESTPOLY;
	}

	dtPolyRef polys[MAX_POLYS];
	int npolys;
	float straightPath[MAX_POLYS * 3];
	unsigned char straightPathFlags[MAX_POLYS];
	dtPolyRef straightPathPolys[MAX_POLYS];
	int nstraightPath;
	int pos = 0;

	navmeshQuery->findPath(startRef, endRef, spos, epos, &filter, polys, &npolys, MAX_POLYS);
	nstraightPath = 0;

	if (npolys)
	{
		float epos1[3];
		dtVcopy(epos1, epos);
				
		if (polys[npolys-1] != endRef)
			navmeshQuery->closestPointOnPoly(polys[npolys-1], epos, epos1);
				
		navmeshQuery->findStraightPath(spos, epos, polys, npolys, straightPath, straightPathFlags, straightPathPolys, &nstraightPath, MAX_POLYS);
		for(int i = 0; i < nstraightPath * 3; )
		{
			Position3D currpos;
			currpos.x = straightPath[i++];
			currpos.y = straightPath[i++];
			currpos.z = straightPath[i++];
			paths.push_back(currpos);
			pos++; 
			
			//DEBUG_MSG(fmt::format("NavMeshHandle::findStraightPath: {}->{}, {}, {}\n", pos, currpos.x, currpos.y, currpos.z));
		}
	}

	return pos;
}

//-------------------------------------------------------------------------------------
int NavMeshHandle::raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPointVec)
{
	if(layer >= (int)navmeshQuery_layers.size())
	{
		ERROR_MSG(fmt::format("NavMeshHandle::findStraightPath: not found layer({})\n",  layer));
		return NAV_ERROR;
	}

	dtNavMeshQuery* navmeshQuery = navmeshQuery_layers[layer];

	float hitPoint[3];

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
	
	hitPointVec.push_back(Position3D(hitPoint[0], hitPoint[1], hitPoint[2]));
	return 1;
}

//-------------------------------------------------------------------------------------
NavigationHandle* NavMeshHandle::create(std::string name)
{
	if(name == "")
		return NULL;
	
	NavMeshHandle* pNavMeshHandle = NULL;

	std::string path = "spaces/" + name;
	path = Resmgr::getSingleton().matchPath(path);
	wchar_t* wpath = strutil::char2wchar(path.c_str());
	std::wstring wspath = wpath;
	free(wpath);

	std::vector<std::wstring> results;
	Resmgr::getSingleton().listPathRes(wspath, L"navmesh", results);

	if(results.size() == 0)
	{
		ERROR_MSG(fmt::format("NavMeshHandle::create: path({}) not found navmesh.!\n", 
			Resmgr::getSingleton().matchRes(path)));

		return NULL;
	}

	pNavMeshHandle = new NavMeshHandle();
	std::vector<std::wstring>::iterator iter = results.begin();

	for(; iter != results.end(); iter++)
	{
		char* cpath = strutil::wchar2char((*iter).c_str());
		path = cpath;
		free(cpath);
	
		FILE* fp = fopen(path.c_str(), "rb");
		if (!fp)
		{
			ERROR_MSG(fmt::format("NavMeshHandle::create: open({}) is error!\n", 
				Resmgr::getSingleton().matchRes(path)));

			break;
		}
		
		DEBUG_MSG(fmt::format("NavMeshHandle::create: ({}), layer={}\n", 
			name, (pNavMeshHandle->navmeshQuery_layers.size())));

		bool safeStorage = true;
		int pos = 0;
		int size = sizeof(NavMeshSetHeader);
		
		fseek(fp, 0, SEEK_END); 
		size_t flen = ftell(fp); 
		fseek(fp, 0, SEEK_SET); 

		uint8* data = new uint8[flen];
		if(data == NULL)
		{
			ERROR_MSG(fmt::format("NavMeshHandle::create: open({}), memory(size={}) error!\n", 
				Resmgr::getSingleton().matchRes(path), flen));

			fclose(fp);
			SAFE_RELEASE_ARRAY(data);
			break;
		}

		size_t readsize = fread(data, 1, flen, fp);
		if(readsize != flen)
		{
			ERROR_MSG(fmt::format("NavMeshHandle::create: open({}), read(size={} != {}) error!\n", 
				Resmgr::getSingleton().matchRes(path), readsize, flen));

			fclose(fp);
			SAFE_RELEASE_ARRAY(data);
			break;
		}

		if (readsize < sizeof(NavMeshSetHeader))
		{
			ERROR_MSG(fmt::format("NavMeshHandle::create: open({}), NavMeshSetHeader is error!\n", 
				Resmgr::getSingleton().matchRes(path)));

			fclose(fp);
			SAFE_RELEASE_ARRAY(data);
			break;
		}

		NavMeshSetHeader header;
		memcpy(&header, data, size);

		pos += size;

		if (header.version != NavMeshHandle::RCN_NAVMESH_VERSION)
		{
			ERROR_MSG(fmt::format("NavMeshHandle::create: navmesh version({}) is not match({})!\n", 
				header.version, ((int)NavMeshHandle::RCN_NAVMESH_VERSION)));

			fclose(fp);
			SAFE_RELEASE_ARRAY(data);
			break;
		}

		dtNavMesh* mesh = dtAllocNavMesh();
		if (!mesh)
		{
			ERROR_MSG("NavMeshHandle::create: dtAllocNavMesh is failed!\n");
			fclose(fp);
			SAFE_RELEASE_ARRAY(data);
			break;
		}

		dtStatus status = mesh->init(&header.params);
		if (dtStatusFailed(status))
		{
			ERROR_MSG(fmt::format("NavMeshHandle::create: mesh init is error({})!\n", status));
			fclose(fp);
			SAFE_RELEASE_ARRAY(data);
			break;
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
			ERROR_MSG(fmt::format("NavMeshHandle::create:  error({})!\n", status));
			dtFreeNavMesh(mesh);
			break;
		}

		pNavMeshHandle->navmesh_layers.push_back(mesh);
		dtNavMeshQuery* pMavmeshQuery = new dtNavMeshQuery();
		pNavMeshHandle->navmeshQuery_layers.push_back(pMavmeshQuery);
		pMavmeshQuery->init(mesh, 1024);
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

			// DEBUG_MSG(fmt::format("NavMeshHandle::create: verts({}, {}, {})\n", tile->verts[0], tile->verts[1], tile->verts[2]));
		}

		DEBUG_MSG(fmt::format("\t==> tiles loaded: {}\n", tileCount));
		DEBUG_MSG(fmt::format("\t==> BVTree nodes: {}\n", nodeCount));
		DEBUG_MSG(fmt::format("\t==> {} polygons ({} vertices)\n", polyCount, vertCount));
		DEBUG_MSG(fmt::format("\t==> {} triangles ({} vertices)\n", triCount, triVertCount));
		DEBUG_MSG(fmt::format("\t==> {:.2f} MB of data (not including pointers)\n", (((float)dataSize / sizeof(unsigned char)) / 1048576)));
	}

	return pNavMeshHandle;
}

//-------------------------------------------------------------------------------------
}

