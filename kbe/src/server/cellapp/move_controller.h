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

#ifndef KBE_MOVETOPOINTCONTROLLERBASE_H
#define KBE_MOVETOPOINTCONTROLLERBASE_H

#include "controller.h"
#include "updatable.h"
#include "moveto_point_handler.h"
#include "pyscript/scriptobject.h"	

namespace KBEngine{

class MoveController : public Controller
{
public:
	MoveController(Entity* pEntity, MoveToPointHandler* pMoveToPointHandler = NULL, uint32 id = 0);
	virtual ~MoveController();
	
	void pMoveToPointHandler(MoveToPointHandler* pMoveToPointHandler)
		{ pMoveToPointHandler_ = pMoveToPointHandler; }


	virtual void addToStream(KBEngine::MemoryStream& s);
	virtual void createFromStream(KBEngine::MemoryStream& s);

protected:
	MoveToPointHandler* pMoveToPointHandler_;
};
 
}
#endif // KBE_MOVETOPOINTCONTROLLERBASE_H

