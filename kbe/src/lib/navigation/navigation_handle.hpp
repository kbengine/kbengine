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

}
#endif // __KBE_NAVIGATEHANDLE_HPP__

