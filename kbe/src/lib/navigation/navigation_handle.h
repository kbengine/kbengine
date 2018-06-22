// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NAVIGATEHANDLE_H
#define KBE_NAVIGATEHANDLE_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "common/smartpointer.h"
#include "common/singleton.h"
#include "math/math.h"

namespace KBEngine{


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

	NavigationHandle():
	resPath()
	{
	}

	virtual ~NavigationHandle()
	{
	}

	virtual NavigationHandle::NAV_TYPE type() const{ return NAV_UNKNOWN; }

	virtual int findStraightPath(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& paths) = 0;

	virtual int findRandomPointAroundCircle(int layer, const Position3D& centerPos,
		std::vector<Position3D>& points, uint32 max_points, float maxRadius) = 0;

	virtual int raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPointVec) = 0;

	std::string resPath;
};

typedef SmartPointer<NavigationHandle> NavigationHandlePtr;

}
#endif // KBE_NAVIGATEHANDLE_H

