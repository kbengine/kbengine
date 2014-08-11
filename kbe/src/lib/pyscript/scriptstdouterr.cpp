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


#include "scriptstdouterr.hpp"
#include "helper/script_loglevel.hpp"

#ifndef CODE_INLINE
#include "scriptstdouterr.ipp"
#endif

namespace KBEngine{ namespace script{

SCRIPT_METHOD_DECLARE_BEGIN(ScriptStdOutErr)
SCRIPT_METHOD_DECLARE("write",				write,				METH_VARARGS,			0)	
SCRIPT_METHOD_DECLARE("flush",				flush,				METH_VARARGS,			0)	
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(ScriptStdOutErr)
SCRIPT_MEMBER_DECLARE("softspace",			softspace_,			T_CHAR,					0,			0)	
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ScriptStdOutErr)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ScriptStdOutErr, 0, 0, 0, 0, 0)									

//-------------------------------------------------------------------------------------
ScriptStdOutErr::ScriptStdOutErr():
ScriptObject(getScriptType(), false),
softspace_(0),
sysModule_(NULL),
prevStderr_(NULL),
prevStdout_(NULL),
isInstall_(false),
sbuffer_()
{
}

//-------------------------------------------------------------------------------------
ScriptStdOutErr::~ScriptStdOutErr()
{
}

//-------------------------------------------------------------------------------------
PyObject* ScriptStdOutErr::__py_write(PyObject* self, PyObject *args)
{
	PyObject* obj = NULL;

	if (!PyArg_ParseTuple(args, "O", &obj))
	{
		ERROR_MSG("ScriptStdOutErr::write: Bad args\n");
		return NULL;
	}
		
	Py_ssize_t size = 0;
	wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(obj, &size);
	static_cast<ScriptStdOutErr*>(self)->onPrint(PyUnicode_AsWideCharStringRet0, size);
	PyMem_Free(PyUnicode_AsWideCharStringRet0);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptStdOutErr::__py_flush(PyObject* self, PyObject *args)
{
	S_Return;
}

//-------------------------------------------------------------------------------------
void ScriptStdOutErr::onPrint(const wchar_t* msg, uint32 msglen)
{
	std::wstring str;
	str.assign(msg, msglen);
	sbuffer_ += str;

	if(msg[0] == L'\n')
	{
		std::string out;
		strutil::wchar2utf8(sbuffer_, out);

		const char* tracebackflag = "Traceback (most recent call last)";
		size_t len = strlen(tracebackflag);
		
		if(out.size() > len)
		{
			if(out.substr(0, len) == tracebackflag)
				DebugHelper::getSingleton().setScriptMsgType(log4cxx::ScriptLevel::SCRIPT_ERR);
		}

		SCRIPT_MSG(out);
		sbuffer_ = L"";
	}
}

//-------------------------------------------------------------------------------------
bool ScriptStdOutErr::install(void)
{
	sysModule_ = PyImport_ImportModule("sys");
	if (!sysModule_)
	{
		ERROR_MSG("ScriptStdOut: Failed to import sys module\n");
		return false;
	}

	prevStderr_ = PyObject_GetAttrString(sysModule_, "stderr");
	prevStdout_ = PyObject_GetAttrString(sysModule_, "stdout");

	PyObject_SetAttrString(sysModule_, "stdout", (PyObject *)this);
	PyObject_SetAttrString(sysModule_, "stderr", (PyObject *)this);
	isInstall_ = true;
	return true;	
}

//-------------------------------------------------------------------------------------
bool ScriptStdOutErr::uninstall(void)
{
	if (prevStderr_)
	{
		PyObject_SetAttrString(sysModule_, "stderr", prevStderr_);
		Py_DECREF(prevStderr_);
		prevStderr_ = NULL;
	}

	if (prevStdout_)
	{
		PyObject_SetAttrString(sysModule_, "stdout", prevStdout_);
		Py_DECREF(prevStdout_);
		prevStdout_ = NULL;
	}

	Py_DECREF(sysModule_);
	sysModule_ = NULL;
	isInstall_ = false;
	return true;	
}

//-------------------------------------------------------------------------------------

}
}
