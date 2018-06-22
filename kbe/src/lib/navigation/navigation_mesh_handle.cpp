// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "navigation_mesh_handle.h"	
#include "navigation/navigation.h"
#include "resmgr/resmgr.h"
#include "thread/threadguard.h"
#include "math/math.h"

namespace KBEngine{	

// Returns a random number [0..1)
static float frand()
{
//	return ((float)(rand() & 0xffff)/(float)0xffff);
	return (float)rand()/(float)RAND_MAX;
}

//-------------------------------------------------------------------------------------
NavMeshHandle::NavMeshHandle():
NavigationHandle(),
navmeshLayer()
{
}

//-------------------------------------------------------------------------------------
NavMeshHandle::~NavMeshHandle()
{
	std::map<int, NavmeshLayer>::iterator iter = navmeshLayer.begin();
	for(; iter != navmeshLayer.end(); ++iter)
	{
		dtFreeNavMesh(iter->second.pNavmesh);
		dtFreeNavMeshQuery(iter->second.pNavmeshQuery);
	}
	
	DEBUG_MSG(fmt::format("NavMeshHandle::~NavMeshHandle(): ({}) is destroyed!\n", resPath));
}

//-------------------------------------------------------------------------------------
int NavMeshHandle::findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths)
{
	std::map<int, NavmeshLayer>::iterator iter = navmeshLayer.find(layer);
	if(iter == navmeshLayer.end())
	{
		ERROR_MSG(fmt::format("NavMeshHandle::findStraightPath: not found layer({})\n",  layer));
		return NAV_ERROR;
	}

	dtNavMeshQuery* navmeshQuery = iter->second.pNavmeshQuery;
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

	float startNearestPt[3];
	float endNearestPt[3];
	navmeshQuery->findNearestPoly(spos, extents, &filter, &startRef, startNearestPt);
	navmeshQuery->findNearestPoly(epos, extents, &filter, &endRef, endNearestPt);

	if (!startRef || !endRef)
	{
		ERROR_MSG(fmt::format("NavMeshHandle::findStraightPath({2}): Could not find any nearby poly's ({0}, {1})\n", startRef, endRef, resPath));
		return NAV_ERROR_NEARESTPOLY;
	}

	dtPolyRef polys[MAX_POLYS];
	int npolys;
	float straightPath[MAX_POLYS * 3];
	unsigned char straightPathFlags[MAX_POLYS];
	dtPolyRef straightPathPolys[MAX_POLYS];
	int nstraightPath;
	int pos = 0;

	navmeshQuery->findPath(startRef, endRef, startNearestPt, endNearestPt, &filter, polys, &npolys, MAX_POLYS);
	nstraightPath = 0;

	if (npolys)
	{
		float epos1[3];
		dtVcopy(epos1, endNearestPt);
				
		if (polys[npolys-1] != endRef)
			navmeshQuery->closestPointOnPoly(polys[npolys-1], endNearestPt, epos1, 0);
				
		navmeshQuery->findStraightPath(startNearestPt, endNearestPt, polys, npolys, straightPath, straightPathFlags, straightPathPolys, &nstraightPath, MAX_POLYS);

		Position3D currpos;
		for(int i = 0; i < nstraightPath * 3; )
		{
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
int NavMeshHandle::findRandomPointAroundCircle(int layer, const Position3D& centerPos, 
	std::vector<Position3D>& points, uint32 max_points, float maxRadius)
{
	std::map<int, NavmeshLayer>::iterator iter = navmeshLayer.find(layer);
	if(iter == navmeshLayer.end())
	{
		ERROR_MSG(fmt::format("NavMeshHandle::findRandomPointAroundCircle: not found layer({})\n",  layer));
		return NAV_ERROR;
	}

	dtNavMeshQuery* navmeshQuery = iter->second.pNavmeshQuery;
	
	dtQueryFilter filter;
	filter.setIncludeFlags(0xffff);
	filter.setExcludeFlags(0);

	if (maxRadius <= 0.0001f)
	{
		Position3D currpos;
		
		for (uint32 i = 0; i < max_points; i++)
		{
			float pt[3];
			dtPolyRef ref;
			dtStatus status = navmeshQuery->findRandomPoint(&filter, frand, &ref, pt);
			if (dtStatusSucceed(status))
			{
				currpos.x = pt[0];
				currpos.y = pt[1];
				currpos.z = pt[2];

				points.push_back(currpos);
			}
		}

		return (int)points.size();
	}
	
	const float extents[3] = {2.f, 4.f, 2.f};

	dtPolyRef startRef = INVALID_NAVMESH_POLYREF;

	float spos[3];
	spos[0] = centerPos.x;
	spos[1] = centerPos.y;
	spos[2] = centerPos.z;

	float startNearestPt[3];
	navmeshQuery->findNearestPoly(spos, extents, &filter, &startRef, startNearestPt);

	if (!startRef)
	{
		ERROR_MSG(fmt::format("NavMeshHandle::findRandomPointAroundCircle({1}): Could not find any nearby poly's ({0})\n", startRef, resPath));
		return NAV_ERROR_NEARESTPOLY;
	}
	
	Position3D currpos;
	bool done = false;
	int itry = 0;

	while (itry++ < 3 && points.size() == 0)
	{
		max_points -= points.size();

		for (uint32 i = 0; i < max_points; i++)
		{
			float pt[3];
			dtPolyRef ref;
			dtStatus status = navmeshQuery->findRandomPointAroundCircle(startRef, spos, maxRadius, &filter, frand, &ref, pt);

			if (dtStatusSucceed(status))
			{
				done = true;
				currpos.x = pt[0];
				currpos.y = pt[1];
				currpos.z = pt[2];

				Position3D v = centerPos - currpos;
				float dist_len = KBEVec3Length(&v);
				if (dist_len > maxRadius)
					continue;

				points.push_back(currpos);
			}
		}

		if (!done)
			break;
	}

	return (int)points.size();
}

//-------------------------------------------------------------------------------------
int NavMeshHandle::raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPointVec)
{
	std::map<int, NavmeshLayer>::iterator iter = navmeshLayer.find(layer);
	if(iter == navmeshLayer.end())
	{
		ERROR_MSG(fmt::format("NavMeshHandle::raycast: not found layer({})\n",  layer));
		return NAV_ERROR;
	}

	dtNavMeshQuery* navmeshQuery = iter->second.pNavmeshQuery;

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
NavigationHandle* NavMeshHandle::create(std::string resPath, const std::map< int, std::string >& params)
{
	if(resPath == "")
		return NULL;
	
	NavMeshHandle* pNavMeshHandle = NULL;

	std::string path = resPath;
	path = Resmgr::getSingleton().matchPath(path);
	wchar_t* wpath = strutil::char2wchar(path.c_str());
	std::wstring wspath = wpath;
	free(wpath);

	if(params.size() == 0)
	{
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
		int layer = 0;
		
		for(; iter != results.end(); ++iter)
		{
			char* cpath = strutil::wchar2char((*iter).c_str());
			path = cpath;
			free(cpath);
			
			_create(layer++, resPath, path, pNavMeshHandle);
		}
	}
	else
	{
		pNavMeshHandle = new NavMeshHandle();
		std::map< int, std::string >::const_iterator iter = params.begin();

		for(; iter != params.end(); ++iter)
		{
			_create(iter->first, resPath, path + "/" + iter->second, pNavMeshHandle);
		}		
	}
	
	return pNavMeshHandle;
}

//-------------------------------------------------------------------------------------
template<typename NAVMESH_SET_HEADER>
dtNavMesh* tryReadNavmesh(uint8* data, size_t readsize, const std::string& res, bool showlog)
{
	if (readsize < sizeof(NAVMESH_SET_HEADER))
	{
		if(showlog)
		{
			ERROR_MSG(fmt::format("NavMeshHandle::tryReadNavmesh: open({}), NavMeshSetHeader error!\n", 
				Resmgr::getSingleton().matchRes(res)));
		}

		return NULL;
	}
	
	int pos = 0;
	int size = 0;
	bool safeStorage = true;
	
	NAVMESH_SET_HEADER header;
	size = sizeof(NAVMESH_SET_HEADER);
	
	memcpy(&header, data, size);

	if (header.version != NavMeshHandle::RCN_NAVMESH_VERSION)
	{
		if(showlog)
		{
			ERROR_MSG(fmt::format("NavMeshHandle::tryReadNavmesh: navmesh version({}) is not match({})!\n", 
				header.version, ((int)NavMeshHandle::RCN_NAVMESH_VERSION)));
		}
		
		return NULL;
	}

	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh)
	{
		if(showlog)
		{
			ERROR_MSG("NavMeshHandle::tryReadNavmesh: dtAllocNavMesh is failed!\n");
		}
		
		return NULL;
	}

	dtStatus status = mesh->init(&header.params);
	if (dtStatusFailed(status))
	{
		if(showlog)
		{
			ERROR_MSG(fmt::format("NavMeshHandle::tryReadNavmesh: mesh init error({})!\n", status));
		}
		
		dtFreeNavMesh(mesh);
		return NULL;
	}

	// Read tiles.
	bool success = true;
	pos += size;

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
	
	if (!success)
	{
		if(showlog)
		{
			ERROR_MSG(fmt::format("NavMeshHandle::tryReadNavmesh:  error({})!\n", status));
		}
		
		dtFreeNavMesh(mesh);
		return NULL;
	}
	
	return mesh;
}

bool NavMeshHandle::_create(int layer, const std::string& resPath, const std::string& res, NavMeshHandle* pNavMeshHandle)
{
	KBE_ASSERT(pNavMeshHandle);
	FILE* fp = fopen(res.c_str(), "rb");
	if (!fp)
	{
		ERROR_MSG(fmt::format("NavMeshHandle::create: open({}) error!\n", 
			Resmgr::getSingleton().matchRes(res)));

		return false;
	}
	
	DEBUG_MSG(fmt::format("NavMeshHandle::create: ({}), layer={}\n", 
		res, layer));

	fseek(fp, 0, SEEK_END); 
	size_t flen = ftell(fp); 
	fseek(fp, 0, SEEK_SET); 

	uint8* data = new uint8[flen];
	if(data == NULL)
	{
		ERROR_MSG(fmt::format("NavMeshHandle::create: open({}), memory(size={}) error!\n", 
			Resmgr::getSingleton().matchRes(res), flen));

		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return false;
	}

	size_t readsize = fread(data, 1, flen, fp);
	if(readsize != flen)
	{
		ERROR_MSG(fmt::format("NavMeshHandle::create: open({}), read(size={} != {}) error!\n", 
			Resmgr::getSingleton().matchRes(res), readsize, flen));

		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return false;
	}

	dtNavMesh* mesh = tryReadNavmesh<NavMeshSetHeader>(data, readsize, res, false);
	
	// 如果加载失败则尝试加载扩展格式
	if(!mesh)
		mesh = tryReadNavmesh<NavMeshSetHeaderEx>(data, readsize, res, true);

	if (!mesh)
	{
		ERROR_MSG("NavMeshHandle::create: dtAllocNavMesh is failed!\n");
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return false;
	}

	fclose(fp);
	SAFE_RELEASE_ARRAY(data);

	dtNavMeshQuery* pMavmeshQuery = new dtNavMeshQuery();

	pMavmeshQuery->init(mesh, 1024);
	pNavMeshHandle->resPath = resPath;
	pNavMeshHandle->navmeshLayer[layer].pNavmeshQuery = pMavmeshQuery;
	pNavMeshHandle->navmeshLayer[layer].pNavmesh = mesh;
	
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
	
	return true;
}

//-------------------------------------------------------------------------------------
}

