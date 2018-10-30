// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "navigation_tile_handle.h"	
#include "navigation/navigation.h"
#include "resmgr/resmgr.h"
#include "thread/threadguard.h"
#include "math/math.h"

namespace KBEngine{	
NavTileHandle* NavTileHandle::pCurrNavTileHandle = NULL;
int NavTileHandle::currentLayer = 0;
NavTileHandle::MapSearchNode NavTileHandle::nodeGoal;
NavTileHandle::MapSearchNode NavTileHandle::nodeStart;
AStarSearch<NavTileHandle::MapSearchNode> NavTileHandle::astarsearch;

#define DEBUG_LISTS 0
#define DEBUG_LIST_LENGTHS_ONLY 0

// Returns a random number [0..1)
static float frand()
{
//	return ((float)(rand() & 0xffff)/(float)0xffff);
	return (float)rand()/(float)RAND_MAX;
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
	DEBUG_MSG(fmt::format("NavTileHandle::~NavTileHandle({1:p}, pTilemap={2:p}): ({0}) is destroyed!\n", 
		resPath, (void*)this, (void*)pTilemap));
	
	SAFE_RELEASE(pTilemap);
}

//-------------------------------------------------------------------------------------
int NavTileHandle::findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths)
{
	setMapLayer(layer);
	pCurrNavTileHandle = this;

	if(pCurrNavTileHandle->pTilemap->GetNumLayers() < layer + 1)
	{
		ERROR_MSG(fmt::format("NavTileHandle::findStraightPath: not found layer({})\n", layer));
		return NAV_ERROR;
	}

	// Create a start state
	nodeStart.x = int(start.x / pTilemap->GetTileWidth());
	nodeStart.y = int(start.z / pTilemap->GetTileHeight()); 

	// Define the goal state
	nodeGoal.x = int(end.x / pTilemap->GetTileWidth());				
	nodeGoal.y = int(end.z / pTilemap->GetTileHeight()); 

	//DEBUG_MSG(fmt::format("NavTileHandle::findStraightPath: start({}, {}), end({}, {})\n", 
	//	nodeStart.x, nodeStart.y, nodeGoal.x, nodeGoal.y));

	// Set Start and goal states
	astarsearch.SetStartAndGoalStates(nodeStart, nodeGoal);

	unsigned int SearchState;
	unsigned int SearchSteps = 0;

	do
	{
		SearchState = astarsearch.SearchStep();

		SearchSteps++;

#if DEBUG_LISTS

		DEBUG_MSG(fmt::format("NavTileHandle::findStraightPath: Steps: {}\n", SearchSteps));

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
		
		DEBUG_MSG(fmt::format("NavTileHandle::findStraightPath: Open list has {} nodes\n", len));

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

		DEBUG_MSG(fmt::format("NavTileHandle::findStraightPath: Closed list has {} nodes\n", len));
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

		// DEBUG_MSG(fmt::format("NavTileHandle::findStraightPath: Solution steps {}\n", steps));
		// Once you're done with the solution you can free the nodes up
		astarsearch.FreeSolutionNodes();
	}
	else if( SearchState == AStarSearch<MapSearchNode>::SEARCH_STATE_FAILED ) 
	{
		ERROR_MSG("NavTileHandle::findStraightPath: Search terminated. Did not find goal state\n");
	}

	// Display the number of loops the search went through
	// DEBUG_MSG(fmt::format("NavTileHandle::findStraightPath: SearchSteps: {}\n", SearchSteps));
	astarsearch.EnsureMemoryFreed();

	return 0;
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
		ERROR_MSG(fmt::format("NavTileHandle::raycast: not found layer({})\n",  layer));
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
int NavTileHandle::findRandomPointAroundCircle(int layer, const Position3D& centerPos,
	std::vector<Position3D>& points, uint32 max_points, float maxRadius)
{
	setMapLayer(layer);
	pCurrNavTileHandle = this;

	if(pCurrNavTileHandle->pTilemap->GetNumLayers() < layer + 1)
	{
		ERROR_MSG(fmt::format("NavTileHandle::findRandomPointAroundCircle: not found layer({})\n", layer));
		return NAV_ERROR;
	}
	
	Position3D currpos;
	
	for (uint32 i = 0; i < max_points; i++)
	{
		float rnd = frand();
		float a = maxRadius * rnd;						// 半径在maxRadius米内
		float b = 360.0f * rnd;							// 随机一个角度
		currpos.x = centerPos.x + (a * cos(b)); 		// 半径 * 正余玄
		currpos.z = centerPos.z + (a * sin(b));
		points.push_back(currpos);
	}

	return (int)points.size();
}

//-------------------------------------------------------------------------------------
NavigationHandle* NavTileHandle::create(std::string resPath, const std::map< int, std::string >& params)
{
	if(resPath == "")
		return NULL;
	
	std::string path;
	
	if(params.size() == 0)
	{
		path = resPath;
		path = Resmgr::getSingleton().matchPath(path);
		wchar_t* wpath = strutil::char2wchar(path.c_str());
		std::wstring wspath = wpath;
		free(wpath);
			
		std::vector<std::wstring> results;
		Resmgr::getSingleton().listPathRes(wspath, L"tmx", results);

		if(results.size() == 0)
		{
			ERROR_MSG(fmt::format("NavTileHandle::create: path({}) not found tmx.!\n", 
				Resmgr::getSingleton().matchRes(path)));

			return NULL;
		}
					
		char* cpath = strutil::wchar2char(results[0].c_str());
		path = cpath;
		free(cpath);
	}
	else
	{
		path = Resmgr::getSingleton().matchRes(params.begin()->second);
	}
	
	return _create(path);
}

//-------------------------------------------------------------------------------------
NavTileHandle* NavTileHandle::_create(const std::string& res)
{
	Tmx::Map *map = new Tmx::Map();
	map->ParseFile(res.c_str());

	if (map->HasError()) 
	{
		ERROR_MSG(fmt::format("NavTileHandle::create: open({}) error!\n", res));
		delete map;
		return NULL;
	}
	
	bool mapdir = map->GetProperties().HasProperty("direction8");

	DEBUG_MSG(fmt::format("NavTileHandle::create: ({})\n", res));
	DEBUG_MSG(fmt::format("\t==> map Width : {}\n", map->GetWidth()));
	DEBUG_MSG(fmt::format("\t==> map Height : {}\n", map->GetHeight()));
	DEBUG_MSG(fmt::format("\t==> tile Width : {} px\n", map->GetTileWidth()));
	DEBUG_MSG(fmt::format("\t==> tile Height : {} px\n", map->GetTileHeight()));
	DEBUG_MSG(fmt::format("\t==> findpath direction : {}\n", (mapdir ? 8 : 4)));

	// Iterate through the tilesets.
	for (int i = 0; i < map->GetNumTilesets(); ++i) {

		DEBUG_MSG(fmt::format("\t==> tileset {:02d}\n", i));

		// Get a tileset.
		const Tmx::Tileset *tileset = map->GetTileset(i);

		// Print tileset information.
		DEBUG_MSG(fmt::format("\t==> name : {}\n", tileset->GetName()));
		DEBUG_MSG(fmt::format("\t==> margin : {}\n", tileset->GetMargin()));
		DEBUG_MSG(fmt::format("\t==> spacing : {}\n", tileset->GetSpacing()));
		DEBUG_MSG(fmt::format("\t==> image Width : {}\n", tileset->GetImage()->GetWidth()));
		DEBUG_MSG(fmt::format("\t==> image Height : {}\n", tileset->GetImage()->GetHeight()));
		DEBUG_MSG(fmt::format("\t==> image Source : {}\n", tileset->GetImage()->GetSource().c_str()));
		DEBUG_MSG(fmt::format("\t==> transparent Color (hex) : {}\n", tileset->GetImage()->GetTransparentColor()));
		DEBUG_MSG(fmt::format("\t==> tiles Size : {}\n", tileset->GetTiles().size()));
		
		if (tileset->GetTiles().size() > 0) 
		{
			// Get a tile from the tileset.
			const Tmx::Tile *tile = *(tileset->GetTiles().begin());

			// Print the properties of a tile.
			std::map< std::string, std::string > list = tile->GetProperties().GetList();
			std::map< std::string, std::string >::iterator iter;
			for (iter = list.begin(); iter != list.end(); ++iter) {
				DEBUG_MSG(fmt::format("\t==> property: {} : {}\n", iter->first.c_str(), iter->second.c_str()));
			}
		}
	}
	
	NavTileHandle* pNavTileHandle = new NavTileHandle(mapdir);
	pNavTileHandle->pTilemap = map;
	return pNavTileHandle;
}

//-------------------------------------------------------------------------------------
bool NavTileHandle::validTile(int x, int y) const
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
	
	return (int)mapTile.id;
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
	sprintf( str, "NavTileHandle::MapSearchNode::printNodeInfo(): Node position : (%d,%d)\n", 
		x, y);
	
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

