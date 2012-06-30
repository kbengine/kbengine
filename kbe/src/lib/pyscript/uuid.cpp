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


#include "uuid.hpp"
namespace KBEngine{ 
namespace script{
PyObject* Uuid::uuidMethod_ = NULL;
bool Uuid::isInit = false;


//-------------------------------------------------------------------------------------
bool Uuid::initialize(void)
{
	if(isInit)
		return true;
	
	PyObject* cPyModule = PyImport_ImportModule("uuid");

	if(cPyModule)
	{
		uuidMethod_ = PyObject_GetAttrString(cPyModule, "uuid4");
		if (!uuidMethod_)
		{
			ERROR_MSG("Uuid::initialize:get uuid4 is error!\n");
			PyErr_PrintEx(0);
		}

		Py_DECREF(cPyModule);
	}
	else
	{
		ERROR_MSG("can't import uuid!\n");
		PyErr_PrintEx(0);
	}
	
	isInit = uuidMethod_ != NULL;
	return isInit;
}

//-------------------------------------------------------------------------------------
void Uuid::finalise(void)
{
	Py_XDECREF(uuidMethod_);
	uuidMethod_ = NULL;
}

//-------------------------------------------------------------------------------------
uint64 Uuid::uuid()
{
	PyObject* pyRet = PyObject_CallFunction(uuidMethod_, 
		const_cast<char*>("()"));
	
	SCRIPT_ERROR_CHECK();
	
	if(pyRet)
	{
		S_RELEASE(pyRet);

		PyObject* pyRet1 = PyObject_GetAttrString(pyRet, const_cast<char*>("int"));
		if(pyRet1 == NULL)
		{
			KBE_ASSERT(false && "Uuid::uuid() is error!\n");
			return 0;
		}

		uint64 ruid = PyLong_AsLong(pyRet1);
		S_RELEASE(pyRet1);
		return ruid;
	}
	
	KBE_ASSERT(false && "Uuid::uuid() is error!\n");
	return 0;
}

//-------------------------------------------------------------------------------------

}
}
