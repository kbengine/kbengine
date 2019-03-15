// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "script.h"
#include "py_gc.h"
#include "scriptstdouterr.h"
#include "py_macros.h"
#include "helper/profile.h"

namespace KBEngine{ namespace script {

PyObject* PyGC::collectMethod_ = NULL;
PyObject* PyGC::set_debugMethod_ = NULL;
KBEUnordered_map<std::string, int> PyGC::tracingCountMap_;

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
			ERROR_MSG("PyGC::initialize: get collect error!\n");
			PyErr_PrintEx(0);
		}

		set_debugMethod_ = PyObject_GetAttrString(gcModule, "set_debug");
		if(!set_debugMethod_)
		{
			ERROR_MSG("PyGC::init: get set_debug error!\n");
			PyErr_PrintEx(0);
		}

		PyObject* flag = NULL;
		
		flag = PyObject_GetAttrString(gcModule, "DEBUG_STATS");
		if(!flag)
		{
			ERROR_MSG("PyGC::init: get DEBUG_STATS error!\n");
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
			ERROR_MSG("PyGC::init: get DEBUG_COLLECTABLE error!\n");
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
			ERROR_MSG("PyGC::init: get DEBUG_UNCOLLECTABLE error!\n");
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
			ERROR_MSG("PyGC::init: get DEBUG_SAVEALL error!\n");
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
			ERROR_MSG("PyGC::init: get DEBUG_LEAK error!\n");
			PyErr_PrintEx(0);
		}
		else
		{
			DEBUG_LEAK = PyLong_AsLong(flag);
			Py_DECREF(flag);
			flag = NULL;
		}

		APPEND_SCRIPT_MODULE_METHOD(gcModule, debugTracing,	__py_debugTracing,	METH_VARARGS, 0);

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
void PyGC::set_debug(uint32 flags)
{
	PyObject* pyRet = PyObject_CallFunction(set_debugMethod_, 
		const_cast<char*>("i"), flags);
	
	SCRIPT_ERROR_CHECK();
	
	if(pyRet)
	{
		S_RELEASE(pyRet);
	}
}

//-------------------------------------------------------------------------------------
void PyGC::incTracing(std::string name)
{
	KBEUnordered_map<std::string, int>::iterator iter = tracingCountMap_.find(name);
	if(iter == tracingCountMap_.end())
	{
		tracingCountMap_[name] = 0;
		iter = tracingCountMap_.find(name);
	}

	iter->second = iter->second + 1;
}

//-------------------------------------------------------------------------------------
void PyGC::decTracing(std::string name)
{
	KBEUnordered_map<std::string, int>::iterator iter = tracingCountMap_.find(name);
	if(iter == tracingCountMap_.end())
	{
		return;
	}

	iter->second = iter->second - 1;
	KBE_ASSERT(iter->second >= 0);
}

//-------------------------------------------------------------------------------------
void PyGC::debugTracing(bool shuttingdown)
{
	KBEUnordered_map<std::string, int>::iterator iter = tracingCountMap_.begin();
	for(; iter != tracingCountMap_.end(); ++iter)
	{
		if(shuttingdown)
		{
			if(iter->second == 0)
				continue;

			ERROR_MSG(fmt::format("PyGC::debugTracing(): {} : leaked({})\n", iter->first, iter->second));
		}
		else
		{
			Script::getSingleton().pyStdouterr()->pyPrint(fmt::format("PyGC::debugTracing(): {} : {}", iter->first, iter->second));
		}
	}
}

//-------------------------------------------------------------------------------------
PyObject* PyGC::__py_debugTracing(PyObject* self, PyObject* args)
{
	debugTracing(false);
	S_Return;
}

//-------------------------------------------------------------------------------------

}
}
