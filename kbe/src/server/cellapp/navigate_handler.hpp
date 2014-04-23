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

#ifndef __KBE_NAVIGATEHANDLER_HPP__
#define __KBE_NAVIGATEHANDLER_HPP__

#include "move_controller.hpp"	

namespace KBEngine{

class NavigateHandler : public MoveToPointHandler
{
public:
	NavigateHandler(Controller* pController, const Position3D& destPos, float velocity, float range, bool faceMovement, 
		float maxMoveDistance, float maxDistance, float girth,
		PyObject* userarg);
	virtual ~NavigateHandler();
	
	virtual bool requestMoveOver(const Position3D& oldPos);

	virtual bool isOnGround(){ return true; }
protected:
	int destPosIdx_;
	std::vector<Position3D> paths_;
	NavigationHandlePtr pNavHandle_;

	float maxMoveDistance_;
	float maxDistance_;
};
 
}
#endif // __KBE_NAVIGATEHANDLER_HPP__

