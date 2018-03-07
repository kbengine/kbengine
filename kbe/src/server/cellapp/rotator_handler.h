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

#ifndef KBE_RotatorHandler_H
#define KBE_RotatorHandler_H

#include "controller.h"
#include "updatable.h"
#include "pyscript/scriptobject.h"	
#include "math/math.h"	


namespace KBEngine{

class RotatorHandler : public Updatable
{
public:
	RotatorHandler(KBEShared_ptr<Controller> pController, const Direction3D& destDir, float velocity, PyObject* userarg);

	RotatorHandler();
	virtual ~RotatorHandler();

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

	virtual bool update();

	virtual const Direction3D& destDir();
	virtual bool requestTurnOver();
	void pController(KBEShared_ptr<Controller> pController){ pController_ = pController; }

	float velocity() const {
		return velocity_;
	}

	void velocity(float v) {
		velocity_ = v;
	}

protected:
	Direction3D destDir_;
	float velocity_;
	PyObject* pyuserarg_;
	KBEShared_ptr<Controller> pController_;
};
 
}
#endif // KBE_MOVETOENTITYHANDLER_H

