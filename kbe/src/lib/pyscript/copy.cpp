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


#include "copy.h"
namespace KBEngine{ 
namespace script{
PyObject* Copy::copyMethod_ = NULL;
PyObject* Copy::deepcopyMethod_ = NULL;
bool Copy::isInit = false;


//-------------------------------------------------------------------------------------
bool Copy::initialize(void)
{
	if(isInit)
		return true;
	
	PyObject* pyModule = PyImport_ImportModule("copy");

	if(pyModule)
	{
		copyMethod_ = PyObject_GetAttrString(pyModule, "copy");
		if (!copyMethod_)
		{
			ERROR_MSG("Copy::initialize:get copy is error!\n");
			PyErr_PrintEx(0);
		}

		deepcopyMethod_ = PyObject_GetAttrString(pyModule, "deepcopy");
		if(!deepcopyMethod_)
		{
			ERROR_MSG("Copy::init: get deepcopy is error!\n");
			PyErr_PrintEx(0);
		}

		Py_DECREF(pyModule);
	}
	else
	{
		ERROR_MSG("can't import copy!\n");
		PyErr_PrintEx(0);
	}
	
	isInit = copyMethod_ && deepcopyMethod_;
	return isInit;
}

//-------------------------------------------------------------------------------------
void Copy::finalise(void)
{
	Py_XDECREF(copyMethod_);
	Py_XDECREF(deepcopyMethod_);
	
	copyMethod_ = NULL;
	deepcopyMethod_ = NULL;	
}

//-------------------------------------------------------------------------------------
PyObject* Copy::copy(PyObject* pyobj)
{
	PyObject* pyRet = PyObject_CallFunction(copyMethod_, 
		const_cast<char*>("(O)"), pyobj);
	
	if(!pyRet)
	{
		SCRIPT_ERROR_CHECK();
		Py_RETURN_NONE;
	}
	
	return pyRet;
}

//-------------------------------------------------------------------------------------
PyObject* Copy::deepcopy(PyObject* pyobj)
{
	PyObject* pyRet = PyObject_CallFunction(deepcopyMethod_, 
		const_cast<char*>("(O)"), pyobj);
	
	if(!pyRet)
	{
		SCRIPT_ERROR_CHECK();
		Py_RETURN_NONE;
	}
	
	return pyRet;
}


//-------------------------------------------------------------------------------------

}
}
