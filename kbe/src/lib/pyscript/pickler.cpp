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


#include "pickler.hpp"
namespace KBEngine{ 
namespace script{
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
			ERROR_MSG("Pickler::initialize:get dumps is error!\n");
			PyErr_PrintEx(0);
		}

		unPicklerMethod_ = PyObject_GetAttrString(cPickleModule, "loads");
		if(!unPicklerMethod_)
		{
			ERROR_MSG("Pickler::init: get loads is error!\n");
			PyErr_PrintEx(0);
		}

		Py_DECREF(cPickleModule);
	}
	else
	{
		ERROR_MSG("can't import pickle!\n");
		PyErr_PrintEx(0);
	}
	
	isInit = picklerMethod_ && unPicklerMethod_;
	
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

	PyModule_Create(&moduleDesc);	
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
std::string Pickler::pickle(PyObject* pyobj)
{
	return pickle(pyobj, 2);
}

//-------------------------------------------------------------------------------------
std::string Pickler::pickle(PyObject* pyobj, int8 protocol)
{
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
	PyObject* pyRet = PyObject_CallFunction(unPicklerMethod_, 
			const_cast<char*>("(s#)"), str.data(), str.length());
	
	if (!pyRet)
	{
		ERROR_MSG("Pickler::unpickle: failed to unpickle[%s] len=%d.\n", str.c_str(), str.length());
		PyErr_Print();
	}
	
	return pyRet;	
}

//-------------------------------------------------------------------------------------
void Pickler::registerUnpickleFunc(PyObject* pyFunc, const char* funcName)
{
	if(PyObject_SetAttrString(pyPickleFuncTableModule_, funcName, pyFunc) == -1){
		PyErr_PrintEx(0);
		return;
	}

	// 这是一段看起来比较奇怪, dir函数会寻找对象的__dict__，当发现没有__dict则会建立, 那么后面的__module__才可以设置成功
	PyObject_Dir(pyFunc);
	
	// 研究cPickle代码发现必须设置这个函数对象的__module__名称为当前模块名称， 否则无法pickle
	PyObject* pyupf = PyBytes_FromString("_upf");
	if(PyObject_SetAttrString(pyFunc, "__module__", pyupf) == -1)
	{
		ERROR_MSG("Pickler::registerUnpickleFunc: set __module__ from unpickleFunc[%s] is error!\n", funcName);
		SCRIPT_ERROR_CHECK();
		Py_DECREF(pyupf);
		return;
	}
	
	Py_DECREF(pyupf);
}

//-------------------------------------------------------------------------------------

}
}
