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

#include "baseapp.h"
#include "space.h"
#include "profile.h"

#ifndef CODE_INLINE
#include "space.inl"
#endif

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(Space)
//SCRIPT_METHOD_DECLARE("createCellEntityInNewSpace", createCellEntityInNewSpace, METH_VARARGS, 0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Space)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Space)
SCRIPT_GETSET_DECLARE("createToCellappIndex", pyGetCreateToCellappIndex, pySetCreateToCellappIndex, 0, 0)
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Space, 0, 0, 0, 0, 0)

class CreateSpaceTimerHandler : public TimerHandler
{
public:
	CreateSpaceTimerHandler(Space * entity) : pEntity_(entity)
	{
	}

private:
	virtual void handleTimeout(TimerHandle handle, void * pUser)
	{
		ScriptTimers* scriptTimers = &pEntity_->scriptTimers();
		int id = ScriptTimersUtil::getIDForHandle(scriptTimers, handle);

		KBE_ASSERT(id > 0);

		PyObject* pyobj = PyObject_GetAttrString(pEntity_, "createToCellappIndex");
		PyObject* pyretobj = pEntity_->createCellEntityInNewSpace(pyobj);
		Py_XDECREF(pyretobj);
		Py_XDECREF(pyobj);
	}

	virtual void onRelease(TimerHandle handle, void * /*pUser*/)
	{
		ScriptTimers* scriptTimers = &pEntity_->scriptTimers();
		scriptTimers->releaseTimer(handle);
		delete this;
	}

	Space* pEntity_;
};


//-------------------------------------------------------------------------------------
Space::Space(ENTITY_ID id, const ScriptDefModule* pScriptModule):
Entity(id, pScriptModule, getScriptType(), true),
createToCellappIndex_(0)
{
	CreateSpaceTimerHandler* pHandler = new CreateSpaceTimerHandler(this);
		ScriptTimers * pTimers = &scriptTimers_;

	int rid = ScriptTimersUtil::addTimer(&pTimers,
			0.1f, 0.f,
			0, pHandler);	

	KBE_ASSERT(rid > 0);
}

//-------------------------------------------------------------------------------------
Space::~Space()  
{
}

//-------------------------------------------------------------------------------------
PyObject* Space::pyGetCreateToCellappIndex()
{
	return PyLong_FromUnsignedLong(createToCellappIndex_);
}

//-------------------------------------------------------------------------------------
int Space::pySetCreateToCellappIndex(PyObject *value)
{
	createToCellappIndex_ = PyLong_AsUnsignedLong(value);
	return 0;
}

//-------------------------------------------------------------------------------------
}
