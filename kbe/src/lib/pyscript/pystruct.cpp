// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "pystruct.h"
namespace KBEngine{ 
namespace script{

PyObject* PyStruct::pack_ = NULL;
PyObject* PyStruct::unpack_ = NULL;
bool PyStruct::isInit = false;

//-------------------------------------------------------------------------------------
bool PyStruct::initialize(void)
{
	if(isInit)
		return true;
	
	PyObject* cModule = PyImport_ImportModule("struct");

	if(cModule)
	{
		pack_ = PyObject_GetAttrString(cModule, "pack");
		if (!pack_)
		{
			ERROR_MSG("PyStruct::initialize:get pack error!\n");
			PyErr_PrintEx(0);
		}

		unpack_ = PyObject_GetAttrString(cModule, "unpack");
		if(!unpack_)
		{
			ERROR_MSG("PyStruct::init: get unpack error!\n");
			PyErr_PrintEx(0);
		}

		Py_DECREF(cModule);
	}
	else
	{
		ERROR_MSG("can't import struct!\n");
		PyErr_PrintEx(0);
	}
	
	isInit = pack_ && unpack_;
	return isInit;
}

//-------------------------------------------------------------------------------------
void PyStruct::finalise(void)
{
	Py_XDECREF(pack_);
	Py_XDECREF(unpack_);
	
	pack_ = NULL;
	unpack_ = NULL;	
}

//-------------------------------------------------------------------------------------
std::string PyStruct::pack(PyObject* fmt, PyObject* args)
{
	PyObject* pyRet = PyObject_CallFunction(pack_, 
			const_cast<char*>("(OO)"), fmt, args);
	
	std::string datas;

	if (!pyRet)
	{
		ERROR_MSG("PyStruct::pack: is failed.\n");
		SCRIPT_ERROR_CHECK();
	}
	else
	{
		char *buffer;
		Py_ssize_t length;

		if(PyBytes_AsStringAndSize(pyRet, &buffer, &length) < 0)
		{
			SCRIPT_ERROR_CHECK();
			return datas;
		}

		datas.assign(buffer, length);
	}

	return datas;	
}

//-------------------------------------------------------------------------------------
PyObject* PyStruct::unpack(PyObject* fmt, PyObject* args)
{
	PyObject* pyRet = PyObject_CallFunction(unpack_, 
			const_cast<char*>("(OO)"), fmt, args);
	
	if (!pyRet)
	{
		ERROR_MSG("PyStruct::unpack: is failed.\n");
	}
	
	SCRIPT_ERROR_CHECK();
	return pyRet;	
}

//-------------------------------------------------------------------------------------

}
}
