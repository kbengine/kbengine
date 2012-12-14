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

#include "script.hpp"
#include "pywatcher.hpp"

namespace KBEngine{ namespace script{

template <class TYPE> 
inline WatcherObject* _addWatcher(std::string path, PyObject* pyobj)
{
	path = std::string("root/scripts/") + path;
	PyWatcherObject<TYPE>* pwo = new PyWatcherObject<TYPE>(path, pyobj);
	WatcherPaths::root().addWatcher(path, pwo);
	return pwo;
};
//-------------------------------------------------------------------------------------
static PyObject* addWatcher(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 3)
	{
		PyErr_Format(PyExc_Exception, "KBEngine::addWatcher: args is error! "
			"arg(watcherName, deftype[UINT32|STRING...], pyCallable).\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyName = NULL;
	PyObject* pyType = NULL;
	PyObject* pyObj = NULL;
	
	if(PyArg_ParseTuple(args, "O|O|O", &pyName, &pyType, &pyObj) == -1)
	{
		PyErr_Format(PyExc_Exception, "KBEngine::addWatcher: args is error! "
			"arg(watcherPath, deftype[UINT32|STRING...], pyCallable).\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(!PyUnicode_Check(pyName))
	{
		PyErr_Format(PyExc_Exception, "KBEngine::addWatcher: args1 is error! "
			"arg=watcherPath\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(!PyUnicode_Check(pyType))
	{
		PyErr_Format(PyExc_Exception, "KBEngine::addWatcher: args2 is error! "
			"arg=deftype[UINT32|STRING...]\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	wchar_t* wstr = PyUnicode_AsWideCharString(pyName, NULL);					
	char* pwatchername = strutil::wchar2char(wstr);	
	std::string watchername = pwatchername;
	PyMem_Free(wstr);	
	free(pwatchername);

	wstr = PyUnicode_AsWideCharString(pyType, NULL);					
	pwatchername = strutil::wchar2char(wstr);	
	std::string type = pwatchername;
	PyMem_Free(wstr);	
	free(pwatchername);

	PyObject* pyObj1 = NULL;

	if(!PyCallable_Check(pyObj))
	{
		PyErr_Format(PyExc_Exception, "Baseapp::createBase: args3 is error! "
			"arg=pyCallable.\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	pyObj1 = PyObject_CallFunction(pyObj, const_cast<char*>(""));
	if(!pyObj1)
	{
		PyErr_Clear();
		PyErr_Format(PyExc_Exception, "Baseapp::createBase: return is error for args3! "
			"arg=pyCallable.\n");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	Py_INCREF(pyObj);

	if(strcmp("UINT8", type.c_str()) == 0)
	{
		_addWatcher<uint8>(pwatchername, pyObj);
	}
	else if(strcmp("UINT16", type.c_str()) == 0)
	{
		_addWatcher<uint16>(pwatchername, pyObj);
	}
	else if(strcmp("UINT32", type.c_str()) == 0)
	{
		_addWatcher<uint32>(pwatchername, pyObj);
	}
	else if(strcmp("UINT64", type.c_str()) == 0)
	{
		_addWatcher<uint64>(pwatchername, pyObj);
	}
	else if(strcmp("INT8", type.c_str()) == 0)
	{
		_addWatcher<int8>(pwatchername, pyObj);
	}
	else if(strcmp("INT16", type.c_str()) == 0)
	{
		_addWatcher<int16>(pwatchername, pyObj);
	}
	else if(strcmp("INT32", type.c_str()) == 0)
	{
		_addWatcher<int32>(pwatchername, pyObj);
	}
	else if(strcmp("INT64", type.c_str()) == 0)
	{
		_addWatcher<int64>(pwatchername, pyObj);
	}
	else if(strcmp("FLOAT", type.c_str()) == 0)
	{
		_addWatcher<float>(pwatchername, pyObj);
	}
	else if(strcmp("DOUBLE", type.c_str()) == 0)
	{
		_addWatcher<double>(pwatchername, pyObj);
	}
	else if(strcmp("BOOL", type.c_str()) == 0)
	{
		_addWatcher<uint8>(pwatchername, pyObj);
	}
	else if(strcmp("STRING", type.c_str()) == 0)
	{
		_addWatcher<std::string>(pwatchername, pyObj);
	}

	Py_DECREF(pyObj1);
	S_Return;
}

//-------------------------------------------------------------------------------------
bool initializePyWatcher(Script* pScript)
{
	// 注册产生uuid方法到py
	APPEND_SCRIPT_MODULE_METHOD(pScript->getModule(),		addWatcher,			addWatcher,					METH_VARARGS,			0);
	return true;
}


}
}
