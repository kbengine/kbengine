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
#include "math/math.hpp"

namespace KBEngine{	
NavTileHandle* NavTileHandle::pCurrNavTileHandle = NULL;
int NavTileHandle::currentLayer = 0;
NavTileHandle::MapSearchNode NavTileHandle::nodeGoal;
NavTileHandle::MapSearchNode NavTileHandle::nodeStart;
AStarSearch<NavTileHandle::MapSearchNode> NavTileHandle::astarsearch;

#define DEBUG_LISTS 0
#define DEBUG_LIST_LENGTHS_ONLY 0

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
int NavMeshHandle::findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths)
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
	navmeshQuery->findNearestPoly(spos, extents, &filter, &startRef, nearestPt);
	navmeshQuery->findNearestPoly(epos, extents, &filter, &endRef, nearestPt);

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
			
			//DEBUG_MSG(boost::format("NavMeshHandle::findStraightPath: %1%->%2%, %3%, %4%\n") % pos % currpos.x % currpos.y % currpos.z);
		}
	}

	return pos;
}

//-------------------------------------------------------------------------------------
void NavMeshHandle::onPassedNode(int layer, ENTITY_ID entityID, const Position3D& oldPos, const Position3D& newPos, NavigationHandle::NAV_OBJECT_STATE state)
{
}

//-------------------------------------------------------------------------------------
void NavMeshHandle::onEnterObject(int layer, ENTITY_ID entityID, const Position3D& currPos)
{
}

//-------------------------------------------------------------------------------------
void NavMeshHandle::onLeaveObject(int layer, ENTITY_ID entityID, const Position3D& currPos)
{
}

//-------------------------------------------------------------------------------------
int NavMeshHandle::raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPointVec)
{
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

	std::string path = "spaces/" + name + "/" + name + ".navmesh";

	FILE* fp = Resmgr::getSingleton().openRes(path, "rb");
	if (!fp)
	{
		ERROR_MSG(boost::format("NavMeshHandle::create: open(%1%) is error!\n") % 
			Resmgr::getSingleton().matchRes(path));

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
		ERROR_MSG(boost::format("NavMeshHandle::create: open(%1%), memory(size=%2%) error!\n") % 
			Resmgr::getSingleton().matchRes(path) % flen);

		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return NULL;
	}

	size_t readsize = fread(data, 1, flen, fp);
	if(readsize != flen)
	{
		ERROR_MSG(boost::format("NavMeshHandle::create: open(%1%), read(size=%2% != %3%) error!\n") % 
			Resmgr::getSingleton().matchRes(path) % readsize % flen);

		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return NULL;
	}

    if (readsize < sizeof(NavMeshSetHeader))
	{
		ERROR_MSG(boost::format("NavMeshHandle::create: open(%1%), NavMeshSetHeader is error!\n") % 
			Resmgr::getSingleton().matchRes(path));

		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
		return NULL;
	}

    NavMeshSetHeader header;
	memcpy(&header, data, size);

    pos += size;

	if (header.version != NavMeshHandle::RCN_NAVMESH_VERSION)
    {
		ERROR_MSG(boost::format("NavMeshHandle::create: version(%1%) is not match(%2%)!\n") % 
			header.version % ((int)NavMeshHandle::RCN_NAVMESH_VERSION));

		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
        return NULL;
    }

    dtNavMesh* mesh = dtAllocNavMesh();
    if (!mesh)
    {
		ERROR_MSG("NavMeshHandle::create: dtAllocNavMesh is failed!\n");
		fclose(fp);
		SAFE_RELEASE_ARRAY(data);
        return NULL;
    }

    dtStatus status = mesh->init(&header.params);
    if (dtStatusFailed(status))
    {
		ERROR_MSG(boost::format("NavMeshHandle::create: mesh init is error(%1%)!\n") % status);
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
		ERROR_MSG(boost::format("NavMeshHandle::create:  error(%1%)!\n") % status);
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

		// DEBUG_MSG(boost::format("NavMeshHandle::create: verts(%1%, %2%, %3%)\n") % tile->verts[0] % tile->verts[1] % tile->verts[2]);
    }

	DEBUG_MSG(boost::format("NavMeshHandle::create: (%1%)\n") % name);
	DEBUG_MSG(boost::format("\t==> tiles loaded: %1%\n") % tileCount);
	DEBUG_MSG(boost::format("\t==> BVTree nodes: %1%\n") % nodeCount);
    DEBUG_MSG(boost::format("\t==> %1% polygons (%2% vertices)\n") % polyCount % vertCount);
    DEBUG_MSG(boost::format("\t==> %1% triangles (%2% vertices)\n") % triCount % triVertCount);
    DEBUG_MSG(boost::format("\t==> %.2f MB of data (not including pointers)\n") % (((float)dataSize / sizeof(unsigned char)) / 1048576));
	return pNavMeshHandle;
}

//-------------------------------------------------------------------------------------
NavTileHandle::NavTileHandle(bool dir):
NavigationHandle(),
pTilemap(0),
direction8_(dir)
{
}

//-------------------------------------------------------------------------------------
NavTileHandle::NavTileHandle(const KBEngine::NavTileHandle & navTileHandle):
NavigationHandle(),
pTilemap(0),
direction8_(navTileHandle.direction8_)
{
	pTilemap = new Tmx::Map(*navTileHandle.pTilemap);
}

//-------------------------------------------------------------------------------------
NavTileHandle::~NavTileHandle()
{
	DEBUG_MSG(boost::format("NavTileHandle::~NavTileHandle(%2%, pTilemap=%3%): (%1%) is destroyed!\n") % name % this % pTilemap);
	SAFE_RELEASE(pTilemap);
}

//-------------------------------------------------------------------------------------
int NavTileHandle::findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths)
{
	setMapLayer(layer);
	pCurrNavTileHandle = this;

	if(pCurrNavTileHandle->pTilemap->GetNumLayers() < layer + 1)
	{
		ERROR_MSG(boost::format("NavTileHandle::findStraightPath: not found layer(%1%)\n") %  layer);
		return NAV_ERROR;
	}

	// Create a start state
	nodeStart.x = int(start.x / pTilemap->GetTileWidth());
	nodeStart.y = int(start.z / pTilemap->GetTileHeight()); 

	// Define the goal state
	nodeGoal.x = int(end.x / pTilemap->GetTileWidth());				
	nodeGoal.y = int(end.z / pTilemap->GetTileHeight()); 

	//DEBUG_MSG(boost::format("NavTileHandle::findStraightPath: start(%1%, %2%), end(%3%, %4%)\n") % 
	//	nodeStart.x % nodeStart.y % nodeGoal.x % nodeGoal.y);

	// Set Start and goal states
	astarsearch.SetStartAndGoalStates(nodeStart, nodeGoal);

	unsigned int SearchState;
	unsigned int SearchSteps = 0;

	do
	{
		SearchState = astarsearch.SearchStep();

		SearchSteps++;

#if DEBUG_LISTS

		DEBUG_MSG(boost::format("NavTileHandle::findStraightPath: Steps: %1%\n") % SearchSteps);

		int len = 0;

		DEBUG_MSG("NavTileHandle::findStraightPath: Open:\n");
		MapSearchNode *p = astarsearch.GetOpenListStart();
		while( p )
		{
			len++;
#if !DEBUG_LIST_LENGTHS_ONLY			
			((MapSearchNode *)p)->printNodeInfo();
#endif
			p = astarsearch.GetOpenListNext();
			
		}
		
		DEBUG_MSG(boost::format("NavTileHandle::findStraightPath: Open list has %1% nodes\n") % len);

		len = 0;

		DEBUG_MSG("NavTileHandle::findStraightPath: Closed:\n");
		p = astarsearch.GetClosedListStart();
		while( p )
		{
			len++;
#if !DEBUG_LIST_LENGTHS_ONLY			
			p->printNodeInfo();
#endif			
			p = astarsearch.GetClosedListNext();
		}

		DEBUG_MSG(boost::format("NavTileHandle::findStraightPath: Closed list has %1% nodes\n") % len);
#endif

	}
	while( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SEARCHING );

	if( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_SUCCEEDED )
	{
		//DEBUG_MSG("NavTileHandle::findStraightPath: Search found goal state\n");
		MapSearchNode *node = astarsearch.GetSolutionStart();

		int steps = 0;

		//node->PrintNodeInfo();
		for( ;; )
		{
			node = astarsearch.GetSolutionNext();

			if( !node )
			{
				break;
			}

			//node->PrintNodeInfo();
			steps ++;
			paths.push_back(Position3D((float)node->x * pTilemap->GetTileWidth(), 0, (float)node->y * pTilemap->GetTileWidth()));
		};

		// DEBUG_MSG(boost::format("NavTileHandle::findStraightPath: Solution steps %1%\n") % steps);
		// Once you're done with the solution you can free the nodes up
		astarsearch.FreeSolutionNodes();
	}
	else if( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED ) 
	{
		ERROR_MSG("NavTileHandle::findStraightPath: Search terminated. Did not find goal state\n");
	}

	// Display the number of loops the search went through
	// DEBUG_MSG(boost::format("NavTileHandle::findStraightPath: SearchSteps: %1%\n") % SearchSteps);
	astarsearch.EnsureMemoryFreed();

	return 0;
}

//-------------------------------------------------------------------------------------
void NavTileHandle::onPassedNode(int layer, ENTITY_ID entityID, const Position3D& oldPos, const Position3D& newPos, NavigationHandle::NAV_OBJECT_STATE state)
{
	setMapLayer(layer);
	pCurrNavTileHandle = this;

	if(pCurrNavTileHandle->pTilemap->GetNumLayers() < layer + 1)
	{
		ERROR_MSG(boost::format("NavTileHandle::onPassedNode: not found layer(%1%)\n") %  layer);
		return;
	}

	MapSearchNode nodeOld;
	nodeOld.x = int(oldPos.x / pTilemap->GetTileWidth());
	nodeOld.y = int(oldPos.z / pTilemap->GetTileHeight()); 

	MapSearchNode nodeNew;
	nodeNew.x = int(newPos.x / pTilemap->GetTileWidth());				
	nodeNew.y = int(newPos.z / pTilemap->GetTileHeight()); 

	Tmx::Layer * pLayer = pCurrNavTileHandle->pTilemap->GetLayer(layer);

	
	if(nodeOld.x != nodeNew.x || nodeOld.y != nodeNew.y)
	{
		if(pCurrNavTileHandle->validTile(nodeOld.x, nodeOld.y))
		{
			Tmx::MapTile& oldMapTile = pLayer->GetTile(nodeOld.x, nodeOld.y);
			oldMapTile.delObj(entityID);

			//DEBUG_MSG(boost::format("NavTileHandle::onPassedNode: leave[entity(%1%), x=%2%, y=%3%, layer=%4%, objs=%5%].\n") % 
			//	entityID % nodeOld.x % nodeOld.y % layer % oldMapTile.objs.size());
		}
	}

	if(pCurrNavTileHandle->validTile(nodeNew.x, nodeNew.y))
	{
		Tmx::MapTile& newMapTile = pLayer->GetTile(nodeNew.x, nodeNew.y);
		newMapTile.addObj(entityID, g_kbetime);

		//DEBUG_MSG(boost::format("NavTileHandle::onPassedNode: enter[entity(%1%), x=%2%, y=%3%, layer=%4%, objs=%5%].\n") % 
		//	entityID % nodeNew.x % nodeNew.y % layer % newMapTile.objs.size());
	}
}

//-------------------------------------------------------------------------------------
void NavTileHandle::onEnterObject(int layer, ENTITY_ID entityID, const Position3D& currPos)
{
	setMapLayer(layer);
	pCurrNavTileHandle = this;

	Tmx::Layer * pLayer = pCurrNavTileHandle->pTilemap->GetLayer(layer);

	if(pCurrNavTileHandle->pTilemap->GetNumLayers() < layer + 1)
	{
		ERROR_MSG(boost::format("NavTileHandle::onEnterObject: not found layer(%1%)\n") %  layer);
		return;
	}

	MapSearchNode node;
	node.x = int(currPos.x / pTilemap->GetTileWidth());
	node.y = int(currPos.z / pTilemap->GetTileHeight()); 

	if(pCurrNavTileHandle->validTile(node.x, node.y))
	{
		Tmx::MapTile& mapTile = pLayer->GetTile(node.x, node.y);

		mapTile.addObj(entityID, g_kbetime);

		//DEBUG_MSG(boost::format("NavTileHandle::onEnterObject: entity(%1%), x=%2%, y=%3%, layer=%4%, objs=%5%.\n") % 
		//	entityID % node.x % node.y % layer % mapTile.objs.size());
	}
}

//-------------------------------------------------------------------------------------
void NavTileHandle::onLeaveObject(int layer, ENTITY_ID entityID, const Position3D& currPos)
{
	setMapLayer(layer);
	pCurrNavTileHandle = this;

	Tmx::Layer * pLayer = pCurrNavTileHandle->pTilemap->GetLayer(layer);

	if(pCurrNavTileHandle->pTilemap->GetNumLayers() < layer + 1)
	{
		ERROR_MSG(boost::format("NavTileHandle::onLeaveObject: not found layer(%1%)\n") %  layer);
		return;
	}

	MapSearchNode node;
	node.x = int(currPos.x / pTilemap->GetTileWidth());
	node.y = int(currPos.z / pTilemap->GetTileHeight()); 

	if(pCurrNavTileHandle->validTile(node.x, node.y))
	{
		Tmx::MapTile& mapTile = pLayer->GetTile(node.x, node.y);

		//DEBUG_MSG(boost::format("NavTileHandle::onLeaveObject: entity(%1%), x=%2%, y=%3%, layer=%4%, objs=%5%.\n") % 
		//	entityID % node.x % node.y % layer % mapTile.objs.size());

		mapTile.delObj(entityID);
	}
}

//-------------------------------------------------------------------------------------
void swap(int& a, int& b) 
{
	int c = a;
	a = b;
	b = c;
}

//-------------------------------------------------------------------------------------
void NavTileHandle::bresenhamLine(const MapSearchNode& p0, const MapSearchNode& p1, std::vector<MapSearchNode>& results)
{
	bresenhamLine(p0.x, p0.y, p1.x, p1.y, results);
}

//-------------------------------------------------------------------------------------
void NavTileHandle::bresenhamLine(int x0, int y0, int x1, int y1, std::vector<MapSearchNode>& results)
{
	// Optimization: it would be preferable to calculate in
	// advance the size of "result" and to use a fixed-size array
	// instead of a list.

	bool steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}
	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int deltax = x1 - x0;
	int deltay = abs(y1 - y0);
	int error = 0;
	int ystep;
	int y = y0;

	if (y0 < y1) ystep = 1; 
		else ystep = -1;

	for (int x = x0; x <= x1; x++) 
	{
		if (steep) 
			results.push_back(MapSearchNode(y, x));
		else 
			results.push_back(MapSearchNode(x, y));

		error += deltay;
		if (2 * error >= deltax) {
			y += ystep;
			error -= deltax;
		}
	}
}

//-------------------------------------------------------------------------------------
int NavTileHandle::raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPointVec)
{
	setMapLayer(layer);
	pCurrNavTileHandle = this;

	if(pCurrNavTileHandle->pTilemap->GetNumLayers() < layer + 1)
	{
		ERROR_MSG(boost::format("NavTileHandle::raycast: not found layer(%1%)\n") %  layer);
		return NAV_ERROR;
	}

	// Create a start state
	MapSearchNode nodeStart;
	nodeStart.x = int(start.x / pTilemap->GetTileWidth());
	nodeStart.y = int(start.z / pTilemap->GetTileHeight()); 

	// Define the goal state
	MapSearchNode nodeEnd;
	nodeEnd.x = int(end.x / pTilemap->GetTileWidth());				
	nodeEnd.y = int(end.z / pTilemap->GetTileHeight()); 

	std::vector<MapSearchNode> vec;
	bresenhamLine(nodeStart, nodeEnd, vec);
	
	if(vec.size() > 0)
	{
		vec.erase(vec.begin());
	}

	std::vector<MapSearchNode>::iterator iter = vec.begin();
	for(; iter != vec.end(); iter++)
	{
		if(getMap((*iter).x, (*iter).y) == TILE_STATE_CLOSED)
			break;

		hitPointVec.push_back(Position3D(float((*iter).x * pTilemap->GetTileWidth()), start.y, float((*iter).y * pTilemap->GetTileWidth())));
	}

	return 1;
}

//-------------------------------------------------------------------------------------
NavigationHandle* NavTileHandle::create(std::string name)
{
	if(name == "")
		return NULL;

	std::string path = Resmgr::getSingleton().matchRes("spaces/" + name + "/" + name + ".tmx");

	Tmx::Map *map = new Tmx::Map();
	map->ParseFile(path.c_str());

	if (map->HasError()) 
	{
		ERROR_MSG(boost::format("NavTileHandle::create: open(%1%) is error!\n") % path);
		delete map;
		return NULL;
	}
	
	bool mapdir = map->GetProperties().HasProperty("direction8");

	DEBUG_MSG(boost::format("NavTileHandle::create: (%1%)\n") % name);
	DEBUG_MSG(boost::format("\t==> map Width : %1%\n") % map->GetWidth());
	DEBUG_MSG(boost::format("\t==> map Height : %1%\n") % map->GetHeight());
	DEBUG_MSG(boost::format("\t==> tile Width : %1% px\n") % map->GetTileWidth());
	DEBUG_MSG(boost::format("\t==> tile Height : %1% px\n") % map->GetTileHeight());
	DEBUG_MSG(boost::format("\t==> findpath direction : %1%\n") % (mapdir ? 8 : 4));

	// Iterate through the tilesets.
	for (int i = 0; i < map->GetNumTilesets(); ++i) {

		DEBUG_MSG(boost::format("\t==> tileset %02d\n") % i);

		// Get a tileset.
		const Tmx::Tileset *tileset = map->GetTileset(i);

		// Print tileset information.
		DEBUG_MSG(boost::format("\t==> name : %1%\n") % tileset->GetName());
		DEBUG_MSG(boost::format("\t==> margin : %1%\n") % tileset->GetMargin());
		DEBUG_MSG(boost::format("\t==> spacing : %1%\n") % tileset->GetSpacing());
		DEBUG_MSG(boost::format("\t==> image Width : %1%\n") % tileset->GetImage()->GetWidth());
		DEBUG_MSG(boost::format("\t==> image Height : %1%\n") % tileset->GetImage()->GetHeight());
		DEBUG_MSG(boost::format("\t==> image Source : %1%\n") % tileset->GetImage()->GetSource().c_str());
		DEBUG_MSG(boost::format("\t==> transparent Color (hex) : %1%\n") % tileset->GetImage()->GetTransparentColor());
		DEBUG_MSG(boost::format("\t==> tiles Size : %1%\n") % tileset->GetTiles().size());
		if (tileset->GetTiles().size() > 0) 
		{
			// Get a tile from the tileset.
			const Tmx::Tile *tile = *(tileset->GetTiles().begin());

			// Print the properties of a tile.
			std::map< std::string, std::string > list = tile->GetProperties().GetList();
			std::map< std::string, std::string >::iterator iter;
			for (iter = list.begin(); iter != list.end(); ++iter) {
				DEBUG_MSG(boost::format("\t==> property: %1% : %2%\n") % iter->first.c_str() % iter->second.c_str());
			}
		}
	}

	NavTileHandle* pNavTileHandle = new NavTileHandle(mapdir);
	pNavTileHandle->pTilemap = map;
	return pNavTileHandle;
}

//-------------------------------------------------------------------------------------
bool NavTileHandle::validTile(int x, int y)const
{
	if( x < 0 ||
	    x >= pTilemap->GetWidth() ||
		 y < 0 ||
		 y >= pTilemap->GetHeight()
	  )
	{
		return false;	 
	}

	return true;
}

//-------------------------------------------------------------------------------------
int NavTileHandle::getMap(int x, int y)
{
	if(!validTile(x, y))
		return TILE_STATE_CLOSED;	 

	Tmx::MapTile& mapTile = pTilemap->GetLayer(currentLayer)->GetTile(x, y);
	
	// 如果是起始点或者是目的地上已经有对象占有了， 我们仍然让astar能够起作用
	// 至于是否能够移动可以交给其他层进行判定， 如在移动途中前方tile被占是否要重新寻路还是采用另一种算法绕开
	// 还是停止
	if((x != nodeStart.x || nodeStart.y != y) && (x != nodeGoal.x || nodeGoal.y != y))
	{
		if(mapTile.minTime > 3)
			return TILE_STATE_CLOSED;	
	}

	return (int)mapTile.id;
}

//-------------------------------------------------------------------------------------
bool NavTileHandle::hasMapObj(int x, int y)
{
	if(!validTile(x, y))
		return false;	 

	Tmx::MapTile& mapTile = pTilemap->GetLayer(currentLayer)->GetTile(x, y);
	return mapTile.minTime > 0;
}

//-------------------------------------------------------------------------------------
bool NavTileHandle::MapSearchNode::IsSameState(MapSearchNode &rhs)
{

	// same state in a maze search is simply when (x,y) are the same
	if( (x == rhs.x) &&
		(y == rhs.y) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

//-------------------------------------------------------------------------------------
void NavTileHandle::MapSearchNode::PrintNodeInfo()
{
	char str[100];
	sprintf( str, "NavTileHandle::MapSearchNode::printNodeInfo(): Node position : (%d,%d)\n", x,y );

	DEBUG_MSG(str);
}

//-------------------------------------------------------------------------------------
// Here's the heuristic function that estimates the distance from a Node
// to the Goal. 

float NavTileHandle::MapSearchNode::GoalDistanceEstimate(MapSearchNode &nodeGoal)
{
	float xd = float(((float)x - (float)nodeGoal.x));
	float yd = float(((float)y - (float)nodeGoal.y));

	return xd + yd;
}

//-------------------------------------------------------------------------------------
bool NavTileHandle::MapSearchNode::IsGoal(MapSearchNode &nodeGoal)
{

	if( (x == nodeGoal.x) &&
		(y == nodeGoal.y) )
	{
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
// This generates the successors to the given Node. It uses a helper function called
// AddSuccessor to give the successors to the AStar class. The A* specific initialisation
// is done for each node internally, so here you just set the state information that
// is specific to the application
bool NavTileHandle::MapSearchNode::GetSuccessors(AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node)
{
	int parent_x = -1; 
	int parent_y = -1; 

	if( parent_node )
	{
		parent_x = parent_node->x;
		parent_y = parent_node->y;
	}
	
	MapSearchNode NewNode;

	// push each possible move except allowing the search to go backwards

	if( (NavTileHandle::pCurrNavTileHandle->getMap( x-1, y ) < TILE_STATE_CLOSED) 
		&& !((parent_x == x-1) && (parent_y == y))
	  ) 
	{
		NewNode = MapSearchNode( x-1, y );
		astarsearch->AddSuccessor( NewNode );
	}	

	if( (NavTileHandle::pCurrNavTileHandle->getMap( x, y-1 ) < TILE_STATE_CLOSED) 
		&& !((parent_x == x) && (parent_y == y-1))
	  ) 
	{
		NewNode = MapSearchNode( x, y-1 );
		astarsearch->AddSuccessor( NewNode );
	}	

	if( (NavTileHandle::pCurrNavTileHandle->getMap( x+1, y ) < TILE_STATE_CLOSED)
		&& !((parent_x == x+1) && (parent_y == y))
	  ) 
	{
		NewNode = MapSearchNode( x+1, y );
		astarsearch->AddSuccessor( NewNode );
	}
		
	if( (NavTileHandle::pCurrNavTileHandle->getMap( x, y+1 ) < TILE_STATE_CLOSED) 
		&& !((parent_x == x) && (parent_y == y+1))
		)
	{
		NewNode = MapSearchNode( x, y+1 );
		astarsearch->AddSuccessor( NewNode );
	}	

	// 如果是8方向移动
	if(NavTileHandle::pCurrNavTileHandle->direction8())
	{
		if( (NavTileHandle::pCurrNavTileHandle->getMap( x + 1, y + 1 ) < TILE_STATE_CLOSED) 
			&& !((parent_x == x + 1) && (parent_y == y + 1))
		  ) 
		{
			NewNode = MapSearchNode( x + 1, y + 1 );
			astarsearch->AddSuccessor( NewNode );
		}	

		if( (NavTileHandle::pCurrNavTileHandle->getMap( x + 1, y-1 ) < TILE_STATE_CLOSED) 
			&& !((parent_x == x + 1) && (parent_y == y-1))
		  ) 
		{
			NewNode = MapSearchNode( x + 1, y-1 );
			astarsearch->AddSuccessor( NewNode );
		}	

		if( (NavTileHandle::pCurrNavTileHandle->getMap( x - 1, y + 1) < TILE_STATE_CLOSED)
			&& !((parent_x == x - 1) && (parent_y == y + 1))
		  ) 
		{
			NewNode = MapSearchNode( x - 1, y + 1);
			astarsearch->AddSuccessor( NewNode );
		}	

		if( (NavTileHandle::pCurrNavTileHandle->getMap( x - 1, y - 1 ) < TILE_STATE_CLOSED) 
			&& !((parent_x == x - 1) && (parent_y == y - 1))
			)
		{
			NewNode = MapSearchNode( x - 1, y - 1 );
			astarsearch->AddSuccessor( NewNode );
		}	
	}

	return true;
}

//-------------------------------------------------------------------------------------
// given this node, what does it cost to move to successor. In the case
// of our map the answer is the map terrain value at this node since that is 
// conceptually where we're moving
float NavTileHandle::MapSearchNode::GetCost( MapSearchNode &successor )
{
	/*
		一个tile寻路的性价比
		每个tile都可以定义从0~5的性价比值， 值越大性价比越低
		比如： 前方虽然能够通过但是前方是泥巴路， 行走起来非常费力， 
		或者是前方为高速公路， 行走非常快。
	*/
	
	/*
		计算代价：
		通常用公式表示为：f = g + h.
		g就是从起点到当前点的代价.
		h是当前点到终点的估计代价，是通过估价函数计算出来的.

		对于一个不再边上的节点，他周围会有8个节点，可以看成他到周围8个点的代价都是1。
		精确点，到上下左右4个点的代价是1，到左上左下右上右下的1.414就是“根号2”，这个值就是前面说的g.
		2.8  2.4  2  2.4  2.8
		2.4  1.4  1  1.4  2.4
		2    1    0    1    2
		2.4  1.4  1  1.4  2.4
		2.8  2.4  2  2.4  2.8
	*/
	if(NavTileHandle::pCurrNavTileHandle->direction8())
	{
		if (x != successor.x && y != successor.y) {
			return (float) (NavTileHandle::pCurrNavTileHandle->getMap( x, y ) + 0.41421356/* 本身有至少1的值 */); //sqrt(2)
		}
	}

	return (float) NavTileHandle::pCurrNavTileHandle->getMap( x, y );

}

//-------------------------------------------------------------------------------------
}

