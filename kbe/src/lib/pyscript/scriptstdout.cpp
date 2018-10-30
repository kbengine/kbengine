// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "scriptstdout.h"
#include "scriptstdouterr.h"

#ifndef CODE_INLINE
#include "scriptstdout.inl"
#endif

namespace KBEngine{ namespace script{

SCRIPT_METHOD_DECLARE_BEGIN(ScriptStdOut)
SCRIPT_METHOD_DECLARE("write",				write,				METH_VARARGS,			0)	
SCRIPT_METHOD_DECLARE("flush",				flush,				METH_VARARGS,			0)	
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ScriptStdOut)
SCRIPT_MEMBER_DECLARE("softspace",			softspace_,			T_CHAR,					0,			0)	
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ScriptStdOut)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ScriptStdOut, 0, 0, 0, 0, 0)									

//-------------------------------------------------------------------------------------
ScriptStdOut::ScriptStdOut(ScriptStdOutErr* pScriptStdOutErr):
ScriptObject(getScriptType(), false),
softspace_(0),
old_stdobj_(NULL),
isInstall_(false),
pScriptStdOutErr_(pScriptStdOutErr)
{
}

//-------------------------------------------------------------------------------------
ScriptStdOut::~ScriptStdOut()
{
}

//-------------------------------------------------------------------------------------
bool ScriptStdOut::install(void)
{
	PyObject* sysModule = PyImport_ImportModule("sys");
	if (!sysModule)
	{
		ERROR_MSG("ScriptStdOut: Failed to import sys module\n");
		return false;
	}
	

	old_stdobj_ = PyObject_GetAttrString(sysModule, "stdout");

	PyObject_SetAttrString(sysModule, "stdout", (PyObject *)this);
	isInstall_ = true;
	Py_DECREF(sysModule);
	return true;	
}

//-------------------------------------------------------------------------------------
bool ScriptStdOut::uninstall(void)
{
	PyObject* sysModule = PyImport_ImportModule("sys");
	if (!sysModule)
	{
		ERROR_MSG("ScriptStdOut: Failed to import sys module\n");
		return false;
	}

	if (old_stdobj_)
	{
		PyObject_SetAttrString(sysModule, "stdout", old_stdobj_);
		Py_DECREF(old_stdobj_);
		old_stdobj_ = NULL;
	}

	Py_DECREF(sysModule);
	isInstall_ = false;
	return true;	
}

//-------------------------------------------------------------------------------------
PyObject* ScriptStdOut::__py_write(PyObject* self, PyObject *args)
{
	char* sdata = NULL;
	Py_ssize_t size = 0;

	if (!PyArg_ParseTuple(args, "s#", &sdata, &size))
	{
		ERROR_MSG("ScriptStdOut::write: Bad args\n");
		return NULL;
	}

	static_cast<ScriptStdErr*>(self)->pScriptStdOutErr()->info_msg(sdata, size);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptStdOut::__py_flush(PyObject* self, PyObject *args)
{
	S_Return;
}

//-------------------------------------------------------------------------------------

}
}
