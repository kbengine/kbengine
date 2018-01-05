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

#ifndef KBE_TURNENTITYCONTROLLERBASE_H
#define KBE_TURNENTITYCONTROLLERBASE_H

#include "controller.h"
#include "updatable.h"
#include "rotator_handler.h"
#include "pyscript/scriptobject.h"	

namespace KBEngine{

class TurnController : public Controller
{
public:
	TurnController(Entity* pEntity, RotatorHandler* pRotatorHandler = NULL, uint32 id = 0);
	virtual ~TurnController();
	
	void pRotatorHandler(RotatorHandler* pRotatorHandler)
	{
		pRotatorHandler_ = pRotatorHandler;
	}
	
	virtual void destroy();
	virtual void addToStream(KBEngine::MemoryStream& s);
	virtual void createFromStream(KBEngine::MemoryStream& s);

	float velocity() const {
		return pRotatorHandler_->velocity();
	}

	void velocity(float v) {
		pRotatorHandler_->velocity(v);
	}

protected:
	RotatorHandler* pRotatorHandler_;
};

}
#endif // KBE_TURNENTITYCONTROLLERBASE_H

