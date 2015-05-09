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

#ifndef KBE_MOVETOENTITYHANDLER_H
#define KBE_MOVETOENTITYHANDLER_H

#include "moveto_point_handler.h"	


namespace KBEngine{

class MoveToEntityHandler : public MoveToPointHandler
{
public:
	MoveToEntityHandler(Controller* pController, ENTITY_ID pTargetID, float velocity, float range, bool faceMovement, 
		bool moveVertically, PyObject* userarg);

	MoveToEntityHandler();
	virtual ~MoveToEntityHandler();
	
	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

	virtual bool update();

	virtual const Position3D& destPos();

	virtual MoveType type() const{ return MOVE_TYPE_ENTITY; }

protected:
	ENTITY_ID pTargetID_;
};
 
}
#endif // KBE_MOVETOENTITYHANDLER_H

