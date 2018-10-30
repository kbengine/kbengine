// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "pickler.h"
#include "helper/profile.h"

namespace KBEngine{ namespace script {

PyObject* Pickler::picklerMethod_ = NULL;
PyObject* Pickler::unPicklerMethod_ = NULL;
PyObject* Pickler::pyPickleFuncTableModule_ = NULL;
bool Pickler::isInit = false;


//-------------------------------------------------------------------------------------
bool Pickler::initialize(void)
{
	if(isInit)
		return true;
	
	PyObject* cPickleModule = PyImport_ImportModule("pickle");

	if(cPickleModule)
	{
		picklerMethod_ = PyObject_GetAttrString(cPickleModule, "dumps");
		if (!picklerMethod_)
		{
			ERROR_MSG("Pickler::initialize: get dumps error!\n");
			PyErr_PrintEx(0);
		}

		unPicklerMethod_ = PyObject_GetAttrString(cPickleModule, "loads");
		if(!unPicklerMethod_)
		{
			ERROR_MSG("Pickler::init: get loads error!\n");
			PyErr_PrintEx(0);
		}

		Py_DECREF(cPickleModule);
	}
	else
	{
		ERROR_MSG("PyGC::initialize: can't import pickle!\n");
		PyErr_PrintEx(0);
	}
	
	isInit = picklerMethod_ && unPicklerMethod_;
	
	if(isInit)
	{
		// 初始化一个unpickle函数表模块， 所有自定义类的unpickle函数都需要在此注册
		pyPickleFuncTableModule_ = PyImport_AddModule("_upf");

		static struct PyModuleDef moduleDesc =   
		{  
			 PyModuleDef_HEAD_INIT,  
			 "_upf",  
			 "This module is created by KBEngine!",  
			 -1,  
			 NULL  
		};  

		PyObject* m = PyModule_Create(&moduleDesc);	
		if(m == NULL)
		{
			isInit = false;
		}
	}

	return isInit;
}

//-------------------------------------------------------------------------------------
void Pickler::finalise(void)
{
	Py_XDECREF(picklerMethod_);
	Py_XDECREF(unPicklerMethod_);
	
	picklerMethod_ = NULL;
	unPicklerMethod_ = NULL;	
	pyPickleFuncTableModule_ = NULL;
}

//-------------------------------------------------------------------------------------
PyObject* Pickler::getUnpickleFunc(const char* funcName)
{ 
	PyObject* pyfunc = PyObject_GetAttrString(pyPickleFuncTableModule_, funcName); 
	if(pyfunc == NULL)
	{
		SCRIPT_ERROR_CHECK();
	}

	return pyfunc;
}

//-------------------------------------------------------------------------------------
std::string Pickler::pickle(PyObject* pyobj)
{
	AUTO_SCOPED_PROFILE("pickle");

	PyObject* pyRet = PyObject_CallFunction(picklerMethod_, 
		const_cast<char*>("(O)"), pyobj);
	
	SCRIPT_ERROR_CHECK();
	
	if(pyRet)
	{
		std::string str;
		str.assign(PyBytes_AsString(pyRet), PyBytes_Size(pyRet));
		S_RELEASE(pyRet);
		return str;
	}
	
	return "";
}

//-------------------------------------------------------------------------------------
std::string Pickler::pickle(PyObject* pyobj, int8 protocol)
{
	AUTO_SCOPED_PROFILE("pickleEx");

	PyObject* pyRet = PyObject_CallFunction(picklerMethod_, 
		const_cast<char*>("(Oi)"), pyobj, protocol);
	
	SCRIPT_ERROR_CHECK();
	
	if(pyRet)
	{
		std::string str;
		str.assign(PyBytes_AsString(pyRet), PyBytes_Size(pyRet));
		S_RELEASE(pyRet);
		return str;
	}
	
	return "";
}

//-------------------------------------------------------------------------------------
PyObject* Pickler::unpickle(const std::string& str)
{
	AUTO_SCOPED_PROFILE("unpickle");

	PyObject* pyRet = PyObject_CallFunction(unPicklerMethod_, 
			const_cast<char*>("(y#)"), str.data(), str.length());
	
	if (!pyRet)
	{
		std::string buff;
		for (size_t i = 0; i < str.length(); ++i)
		{
			if ((uchar)str[i] >= 32 && (uchar)str[i] <= 126)
				buff += str[i];
			else
				buff += fmt::format("\\x{:02x}", (uchar)str[i]);
		}

		ERROR_MSG(fmt::format("Pickler::unpickle: failed to unpickle[{}] len={}.\n",
			buff, str.length()));
	}
	
	SCRIPT_ERROR_CHECK();
	return pyRet;	
}

//-------------------------------------------------------------------------------------
void Pickler::registerUnpickleFunc(PyObject* pyFunc, const char* funcName)
{
	if(PyObject_SetAttrString(pyPickleFuncTableModule_, funcName, pyFunc) == -1)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}
}

//-------------------------------------------------------------------------------------

}
}
