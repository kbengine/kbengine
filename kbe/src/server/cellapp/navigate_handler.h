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

#ifndef KBE_NAVIGATEHANDLER_H
#define KBE_NAVIGATEHANDLER_H

#include "move_controller.h"	
#include "math/math.h"
#include "navigation/navigation_handle.h"

namespace KBEngine{

class NavigateHandler : public MoveToPointHandler
{
public:
	NavigateHandler(KBEShared_ptr<Controller>& pController, const Position3D& destPos, float velocity, float distance, bool faceMovement, 
		float maxMoveDistance, VECTOR_POS3D_PTR paths_ptr,
		PyObject* userarg);

	NavigateHandler();
	virtual ~NavigateHandler();
	
	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

	virtual bool requestMoveOver(const Position3D& oldPos);

	virtual bool isOnGround(){ return true; }

	virtual MoveType type() const { return MOVE_TYPE_NAV; }

protected:
	int destPosIdx_;
	VECTOR_POS3D_PTR paths_;

	float maxMoveDistance_;
};
 
}
#endif // KBE_NAVIGATEHANDLER_H

