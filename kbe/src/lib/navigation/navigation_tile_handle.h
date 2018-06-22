// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NAVIGATETILEHANDLE_H
#define KBE_NAVIGATETILEHANDLE_H

#include "navigation/navigation_handle.h"

#include "stlastar.h"
#include "tmxparser/Tmx.h"

namespace KBEngine{

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
	static AStarSearch<NavTileHandle::MapSearchNode> astarsearch;

public:
	NavTileHandle(bool dir);
	NavTileHandle(const KBEngine::NavTileHandle & navTileHandle);

	virtual ~NavTileHandle();

	int findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths);

	virtual int findRandomPointAroundCircle(int layer, const Position3D& centerPos,
		std::vector<Position3D>& points, uint32 max_points, float maxRadius);

	int raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPointVec);

	virtual NavigationHandle::NAV_TYPE type() const{ return NAV_TILE; }

	static NavigationHandle* create(std::string resPath, const std::map< int, std::string >& params);
	static NavTileHandle* _create(const std::string& res);
	
	int getMap(int x, int y);

	void bresenhamLine(const MapSearchNode& p0, const MapSearchNode& p1, std::vector<MapSearchNode>& results);
	void bresenhamLine(int x0, int y0, int x1, int y1, std::vector<MapSearchNode>& results);

	bool direction8() const{ return direction8_; }
	
	bool validTile(int x, int y) const;

public:
	Tmx::Map *pTilemap;
	bool direction8_;
	std::map< int, std::string > params_;
};

}
#endif // KBE_NAVIGATETILEHANDLE_H

