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
	if (iter == navmeshLayer.end())
	{
		ERROR_MSG(fmt::format("NavMeshHandle::findRandomPointAroundCircle: not found layer({})\n", layer));
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

	const float extents[3] = { 2.f, 4.f, 2.f };

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

	const float squareSize = (float)maxRadius;
	float squareVerts[12] = {
		spos[0] - squareSize, spos[1], spos[2] + squareSize,\
		spos[0] + squareSize, spos[1], spos[2] + squareSize,\
		spos[0] + squareSize, spos[1], spos[2] - squareSize,\
		spos[0] - squareSize, spos[1], spos[2] - squareSize,\
	};

	static const int maxResult = 32;
	dtPolyRef polyRefs[maxResult];
	dtPolyRef parentPolyRefs[maxResult];
	int polyCount = 0;
	float cost[maxResult];

	navmeshQuery->findPolysAroundShape(startRef, squareVerts, 4, &filter, polyRefs, parentPolyRefs, cost, &polyCount, maxResult);

	if (polyCount == 0)
	{
		return (int)points.size();
	}

	float* allPolyAreas = new float[polyCount];
	Position3D currpos;
	for (uint32 time = 0; time < max_points; time++)
	{
		const dtMeshTile* randomTile = 0;
		const dtPoly* randomPoly = 0;
		dtPolyRef randomPolyRef = 0;
		float areaSum = 0.0f;

		for (int i = 0; i < polyCount; i++)
		{
			float polyArea = 0.0f;

			if (time == 0)
			{
				const dtMeshTile* tile = 0;
				const dtPoly* poly = 0;
				dtPolyRef ref = polyRefs[i];
				navmeshQuery->getAttachedNavMesh()->getTileAndPolyByRefUnsafe(ref, &tile, &poly);

				if (poly->getType() != DT_POLYTYPE_GROUND) continue;

				// Place random locations on on ground.
				if (poly->getType() == DT_POLYTYPE_GROUND)
				{
					// Calc area of the polygon.
					for (int j = 2; j < poly->vertCount; ++j)
					{
						const float* va = &tile->verts[poly->verts[0] * 3];
						const float* vb = &tile->verts[poly->verts[j - 1] * 3];
						const float* vc = &tile->verts[poly->verts[j] * 3];
						polyArea += dtTriArea2D(va, vb, vc);
					}

					allPolyAreas[i] = polyArea;
					areaSum += polyArea;
					const float u = frand();

					if (u*areaSum <= polyArea)
					{
						randomTile = tile;
						randomPoly = poly;
						randomPolyRef = ref;
					}
				}
			}
			else
			{
				// Choose random polygon weighted by area, using reservoi sampling.
				areaSum += allPolyAreas[i];
				const float u = frand();

				if (u*areaSum <= allPolyAreas[i])
				{
					const dtMeshTile* tile = 0;
					const dtPoly* poly = 0;
					dtPolyRef ref = polyRefs[i];
					navmeshQuery->getAttachedNavMesh()->getTileAndPolyByRefUnsafe(ref, &tile, &poly);

					if (poly->getType() != DT_POLYTYPE_GROUND) continue;

					randomTile = tile;
					randomPoly = poly;
					randomPolyRef = ref;
				}
			}
		}

		// Randomly pick point on polygon.
		dtPolyRef randomRef = INVALID_NAVMESH_POLYREF;
		const float* v = &randomTile->verts[randomPoly->verts[0] * 3];
		float verts[3 * DT_VERTS_PER_POLYGON];
		float areas[DT_VERTS_PER_POLYGON + 4];
		dtVcopy(&verts[0 * 3], v);

		for (int j = 1; j < randomPoly->vertCount; ++j)
		{
			v = &randomTile->verts[randomPoly->verts[j] * 3];
			dtVcopy(&verts[j * 3], v);
		}

		float overlapPolyVerts[(DT_VERTS_PER_POLYGON + 4) * 3];
		int nOverlapPolyVerts = 0;

		getOverlapPolyPoly2D(squareVerts, 4, verts, randomPoly->vertCount, overlapPolyVerts, &nOverlapPolyVerts);

		if (nOverlapPolyVerts <= 0)
		{
			delete[] allPolyAreas;
			return (int)points.size();
		}

		const float s = frand();
		const float t = frand();
		float pt[3];
		dtRandomPointInConvexPoly(overlapPolyVerts, nOverlapPolyVerts, areas, s, t, pt);

		float h = 0.0f;
		dtStatus status = navmeshQuery->getPolyHeight(randomPolyRef, pt, &h);

		if (dtStatusFailed(status))
		{
			delete[] allPolyAreas;
			return (int)points.size();
		}

		pt[1] = h;
		randomRef = randomPolyRef;

		if (randomRef)
		{
			currpos.x = pt[0];
			currpos.y = pt[1];
			currpos.z = pt[2];

			float src_len = sqrt(2) * squareSize;
			float xx = centerPos.x - currpos.x;
			float yy = centerPos.y - currpos.y;
			float dist_len = sqrt(xx * xx + yy * yy);

			if (dist_len > src_len)
			{
				ERROR_MSG(fmt::format("NavMeshHandle::findRandomPointAroundCircle::(Out of range)::centerPos({},{},{}), currpos({},{},{}), errLen({}), {}, {}\n", 
					centerPos.x, centerPos.y, centerPos.z, currpos.x, currpos.y, currpos.z, (dist_len - src_len), dist_len, src_len));

				continue;
			}

			points.push_back(currpos);
		}
	}

	delete[] allPolyAreas;

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
inline float calAtan(float* srcPoint, float* point)
{
	return atan2(point[2] - srcPoint[2], point[0] - srcPoint[0]);
}

inline void swapPoint(float* a, float* b)
{
	float tmp[3] = { a[0],a[1],a[2] };
	a[0] = b[0];
	a[1] = b[1];
	a[2] = b[2];
	b[0] = tmp[0];
	b[1] = tmp[1];
	b[2] = tmp[2];
}

void NavMeshHandle::getOverlapPolyPoly2D(const float* polyVertsA, const int nPolyVertsA, const float* polyVertsB, const int nPolyVertsB, float* intsectPt, int* intsectPtCount)
{
	*intsectPtCount = 0;

	///Find polyA's verts which in polyB.
	for (int i = 0; i < nPolyVertsA; ++i)
	{
		const float* va = &polyVertsA[i * 3];
		if (dtPointInPolygon(va, polyVertsB, nPolyVertsB))
		{
			intsectPt[*intsectPtCount * 3] = va[0];
			intsectPt[*intsectPtCount * 3 + 1] = va[1];
			intsectPt[*intsectPtCount * 3 + 2] = va[2];
			*intsectPtCount += 1;
		}
	}

	///Find polyB's verts which in polyA.
	for (int i = 0; i < nPolyVertsB; ++i)
	{
		const float* va = &polyVertsB[i * 3];
		if (dtPointInPolygon(va, polyVertsA, nPolyVertsA))
		{
			intsectPt[*intsectPtCount * 3] = va[0];
			intsectPt[*intsectPtCount * 3 + 1] = va[1];
			intsectPt[*intsectPtCount * 3 + 2] = va[2];
			*intsectPtCount += 1;
		}
	}

	///Find edge intersection of polyA and polyB.
	for (int i = 0; i < nPolyVertsA; ++i)
	{
		const float* p1 = &polyVertsA[i * 3];
		int p2_idx = (i + 1) % nPolyVertsA;
		const float* p2 = &polyVertsA[p2_idx * 3];

		for (int j = 0; j < nPolyVertsB; ++j)
		{
			const float* q1 = &polyVertsB[j * 3];
			int q2_idx = (j + 1) % nPolyVertsB;
			const float* q2 = &polyVertsB[q2_idx * 3];

			if (isSegSegCross2D(p1, p2, q1, q2))				 ///If two segment is cross
			{
				float s, t;
				if (dtIntersectSegSeg2D(p1, p2, q1, q2, s, t))	///Caculate intersection point
				{
					float pt[3];
					dtVlerp(pt, q1, q2, t);
					intsectPt[*intsectPtCount * 3] = pt[0];
					intsectPt[*intsectPtCount * 3 + 1] = pt[1];
					intsectPt[*intsectPtCount * 3 + 2] = pt[2];
					*intsectPtCount += 1;
				}
			}
		}
	}

	///sort intersection to clockwise.
	if (*intsectPtCount > 0)
	{
		clockwiseSortPoints(intsectPt, *intsectPtCount);
	}

}

void NavMeshHandle::clockwiseSortPoints(float* verts, const int nVerts)
{
	float x = 0.0;
	float z = 0.0;
	for (int i = 0; i < nVerts; ++i)
	{
		x += verts[i * 3];
		z += verts[i * 3 + 2];
	}

	//Put most left point in first position.
	for (int i = 0; i < nVerts; i++)
	{
		if (verts[i * 3] < verts[0])
		{
			swapPoint(&verts[i * 3], &verts[0]);
		}
		else if (verts[i * 3] == verts[0] && verts[i * 3 + 2] < verts[2])
		{
			swapPoint(&verts[i * 3], &verts[0]);
		}
	}

	//Sort points by slope.
	for (int i = 1; i < nVerts; i++)
	{
		for (int j = 1; j < nVerts - i; j++)
		{
			int index = j * 3;
			int n_index = (j + 1) * 3;
			float angle = calAtan(&verts[0], &verts[index]);
			float n_angle = calAtan(&verts[0], &verts[n_index]);
			if (angle < n_angle)
			{
				swapPoint(&verts[index], &verts[n_index]);
			}
		}
	}
}

bool NavMeshHandle::isSegSegCross2D(const float* p1, const float *p2, const float* q1, const float* q2)
{
	bool ret = dtMin(p1[0], p2[0]) <= dtMax(q1[0], q2[0]) &&
		dtMin(q1[0], q2[0]) <= dtMax(p1[0], p2[0]) &&
		dtMin(p1[2], p2[2]) <= dtMax(q1[2], q2[2]) &&
		dtMin(q1[2], q2[2]) <= dtMax(p1[2], p2[2]);

	if (!ret)
	{
		return false;
	}

	long line1, line2;
	line1 = (long)(p1[0] * (q1[2] - p2[2]) + p2[0] * (p1[2] - q1[2]) + q1[0] * (p2[2] - p1[2]));
	line2 = (long)(p1[0] * (q2[2] - p2[2]) + p2[0] * (p1[2] - q2[2]) + q2[0] * (p2[2] - p1[2]));
	if (((line1 ^ line2) >= 0) && !(line1 == 0 && line2 == 0))
	{
		return false;
	}

	line1 = (long)(q1[0] * (p1[2] - q2[2]) + q2[0] * (q1[2] - p1[2]) + p1[0] * (q2[2] - q1[2]));
	line2 = (long)(q1[0] * (p2[2] - q2[2]) + q2[0] * (q1[2] - p2[2]) + p2[0] * (q2[2] - q1[2]));
	if (((line1 ^ line2) >= 0) && !(line1 == 0 && line2 == 0))
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------

}


