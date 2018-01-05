/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

