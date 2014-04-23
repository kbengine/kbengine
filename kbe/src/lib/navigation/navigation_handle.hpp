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

#include "stlastar.h"
#include "tmxparser/Tmx.h"

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

class NavigationHandle : public RefCountable
{
public:
	static const int NAV_ERROR = -1;

	enum NAV_TYPE
	{
		NAV_UNKNOWN = 0,
		NAV_MESH = 1,
		NAV_TILE = 2
	};

	enum NAV_OBJECT_STATE
	{
		NAV_OBJECT_STATE_MOVING = 1,	// 移动中
		NAV_OBJECT_STATE_MOVEOVER = 2,	// 移动已经结束了
	};

	NavigationHandle():name()
	{
	}

	virtual ~NavigationHandle()
	{
	}

	virtual NavigationHandle::NAV_TYPE type() const{ return NAV_UNKNOWN; }

	virtual int findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths) = 0;
	virtual int raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPointVec) = 0;
	virtual void onPassedNode(int layer, ENTITY_ID entityID, const Position3D& oldPos, const Position3D& newPos, NavigationHandle::NAV_OBJECT_STATE state) = 0;

	virtual void onEnterObject(int layer, ENTITY_ID entityID, const Position3D& currPos) = 0;
	virtual void onLeaveObject(int layer, ENTITY_ID entityID, const Position3D& currPos) = 0;

	std::string name;
};

typedef SmartPointer<NavigationHandle> NavigationHandlePtr;

class NavMeshHandle : public NavigationHandle
{
public:
	static const int MAX_POLYS = 256;
	static const int NAV_ERROR_NEARESTPOLY = -2;

	static const long RCN_NAVMESH_VERSION = 1;
	static const int INVALID_NAVMESH_POLYREF = 0;
public:
	NavMeshHandle();
	virtual ~NavMeshHandle();

	int findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths);
	int raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPointVec);

	virtual NavigationHandle::NAV_TYPE type() const{ return NAV_MESH; }

	static NavigationHandle* create(std::string name);

	void onPassedNode(int layer, ENTITY_ID entityID, const Position3D& oldPos, const Position3D& newPos, NavigationHandle::NAV_OBJECT_STATE state);

	virtual void onEnterObject(int layer, ENTITY_ID entityID, const Position3D& currPos);
	virtual void onLeaveObject(int layer, ENTITY_ID entityID, const Position3D& currPos);

	dtNavMesh* navmesh;
	dtNavMeshQuery* navmeshQuery;
};

class NavTileHandle : public NavigationHandle
{
public:
	static NavTileHandle* pCurrNavTileHandle;
	static int currentLayer;

	static void setMapLayer(int layer)
	{ 
		currentLayer = layer; 
	}

	enum TILE_STATE
	{
		TILE_STATE_OPENED_COST0 = 0,	// 打开状态, 允许通过
		TILE_STATE_OPENED_COST1 = 1,	// 打开状态, 允许通过
		TILE_STATE_OPENED_COST2 = 2,	// 打开状态, 允许通过
		TILE_STATE_OPENED_COST3 = 3,	// 打开状态, 允许通过
		TILE_STATE_OPENED_COST4 = 4,	// 打开状态, 允许通过
		TILE_STATE_OPENED_COST5 = 5,	// 打开状态, 允许通过
		TILE_STATE_CLOSED = 9			// 关闭状态
	};

	class MapSearchNode
	{
	public:
		int x;	 // the (x,y) positions of the node
		int y;	
		

		MapSearchNode() { x = y = 0; }
		MapSearchNode(int px, int py) {x = px; y = py; }

		float GoalDistanceEstimate( MapSearchNode &nodeGoal );
		bool IsGoal( MapSearchNode &nodeGoal );
		bool GetSuccessors( AStarSearch<MapSearchNode> *astarsearch, MapSearchNode *parent_node );
		float GetCost( MapSearchNode &successor );
		bool IsSameState( MapSearchNode &rhs );

		void PrintNodeInfo(); 
	};
	
	static MapSearchNode nodeGoal, nodeStart;
public:
	NavTileHandle(bool dir);
	NavTileHandle(const KBEngine::NavTileHandle & navTileHandle);

	virtual ~NavTileHandle();

	int findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths);
	int raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPointVec);

	virtual NavigationHandle::NAV_TYPE type() const{ return NAV_TILE; }

	static NavigationHandle* create(std::string name);
	
	int getMap(int x, int y);
	bool hasMapObj(int x, int y);

	void bresenhamLine(const MapSearchNode& p0, const MapSearchNode& p1, std::vector<MapSearchNode>& results);
	void bresenhamLine(int x0, int y0, int x1, int y1, std::vector<MapSearchNode>& results);

	void onPassedNode(int layer, ENTITY_ID entityID, const Position3D& oldPos, const Position3D& newPos, NavigationHandle::NAV_OBJECT_STATE state);

	virtual void onEnterObject(int layer, ENTITY_ID entityID, const Position3D& currPos);
	virtual void onLeaveObject(int layer, ENTITY_ID entityID, const Position3D& currPos);

	bool direction8()const{ return direction8_; }
	
	bool validTile(int x, int y)const;
public:
	Tmx::Map *pTilemap;
	bool direction8_;
};

}
#endif // __KBE_NAVIGATEHANDLE_HPP__

