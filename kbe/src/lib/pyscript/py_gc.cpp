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


#include "py_gc.hpp"
#include "helper/profile.hpp"

namespace KBEngine{ namespace script {

PyObject* PyGC::collectMethod_ = NULL;
PyObject* PyGC::set_debugMethod_ = NULL;

uint32 PyGC::DEBUG_STATS = 0;
uint32 PyGC::DEBUG_COLLECTABLE = 0;
uint32 PyGC::DEBUG_UNCOLLECTABLE = 0;
uint32 PyGC::DEBUG_SAVEALL = 0;
uint32 PyGC::DEBUG_LEAK = 0;
	
bool PyGC::isInit = false;


//-------------------------------------------------------------------------------------
bool PyGC::initialize(void)
{
	if(isInit)
		return true;
	
	PyObject* gcModule = PyImport_ImportModule("gc");

	if(gcModule)
	{
		collectMethod_ = PyObject_GetAttrString(gcModule, "collect");
		if (!collectMethod_)
		{
			ERROR_MSG("PyGC::initialize: get collect is error!\n");
			PyErr_PrintEx(0);
		}

		set_debugMethod_ = PyObject_GetAttrString(gcModule, "set_debug");
		if(!set_debugMethod_)
		{
			ERROR_MSG("PyGC::init: get set_debug is error!\n");
			PyErr_PrintEx(0);
		}

		PyObject* flag = NULL;
		
		flag = PyObject_GetAttrString(gcModule, "DEBUG_STATS");
		if(!flag)
		{
			ERROR_MSG("PyGC::init: get DEBUG_STATS is error!\n");
			PyErr_PrintEx(0);
		}
		else
		{
			DEBUG_STATS = PyLong_AsLong(flag);
			Py_DECREF(flag);
			flag = NULL;
		}
		
		flag = PyObject_GetAttrString(gcModule, "DEBUG_COLLECTABLE");
		if(!flag)
		{
			ERROR_MSG("PyGC::init: get DEBUG_COLLECTABLE is error!\n");
			PyErr_PrintEx(0);
		}
		else
		{
			DEBUG_COLLECTABLE = PyLong_AsLong(flag);
			Py_DECREF(flag);
			flag = NULL;
		}
		
		flag = PyObject_GetAttrString(gcModule, "DEBUG_UNCOLLECTABLE");
		if(!flag)
		{
			ERROR_MSG("PyGC::init: get DEBUG_UNCOLLECTABLE is error!\n");
			PyErr_PrintEx(0);
		}
		else
		{
			DEBUG_UNCOLLECTABLE = PyLong_AsLong(flag);
			Py_DECREF(flag);
			flag = NULL;
		}
		
		flag = PyObject_GetAttrString(gcModule, "DEBUG_SAVEALL");
		if(!flag)
		{
			ERROR_MSG("PyGC::init: get DEBUG_SAVEALL is error!\n");
			PyErr_PrintEx(0);
		}
		else
		{
			DEBUG_SAVEALL = PyLong_AsLong(flag);
			Py_DECREF(flag);
			flag = NULL;
		}

		flag = PyObject_GetAttrString(gcModule, "DEBUG_LEAK");
		if(!flag)
		{
			ERROR_MSG("PyGC::init: get DEBUG_LEAK is error!\n");
			PyErr_PrintEx(0);
		}
		else
		{
			DEBUG_LEAK = PyLong_AsLong(flag);
			Py_DECREF(flag);
			flag = NULL;
		}

		Py_DECREF(gcModule);
	}
	else
	{
		ERROR_MSG("PyGC::initialize: can't import gc!\n");
		PyErr_PrintEx(0);
	}
	
	isInit = collectMethod_ && set_debugMethod_;
	return isInit;
}

//-------------------------------------------------------------------------------------
void PyGC::finalise(void)
{
	Py_XDECREF(collectMethod_);
	Py_XDECREF(set_debugMethod_);
	
	collectMethod_ = NULL;
	set_debugMethod_ = NULL;	
}

//-------------------------------------------------------------------------------------
void PyGC::collect(int8 generations)
{
	PyObject* pyRet = NULL;
		
	if(generations != -1)
	{
		pyRet = PyObject_CallFunction(collectMethod_, 
			const_cast<char*>("(i)"), generations);
	}
	else
	{
		pyRet = PyObject_CallFunction(collectMethod_, 
			const_cast<char*>(""));
	}
	
	SCRIPT_ERROR_CHECK();
	
	if(pyRet)
	{
		S_RELEASE(pyRet);
	}
}

//-------------------------------------------------------------------------------------
void PyGC::set_debug(uint32 flsgs)
{
	PyObject* pyRet = PyObject_CallFunction(collectMethod_, 
			const_cast<char*>("(i)"), flsgs);
	
	SCRIPT_ERROR_CHECK();
	
	if(pyRet)
	{
		S_RELEASE(pyRet);
	}
}

//-------------------------------------------------------------------------------------

}
}
