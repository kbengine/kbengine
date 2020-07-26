// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "script.h"
#include "pywatcher.h"

#ifndef CODE_INLINE
#include "pywatcher.inl"
#endif

namespace KBEngine{ namespace script{

template <class TYPE> 
inline WatcherObject* _addWatcher(std::string path, PyObject* pyobj)
{
	path = std::string("root/scripts/") + path;
	PyWatcherObject<TYPE>* pwo = new PyWatcherObject<TYPE>(path, pyobj);
	WatcherPaths::root().addWatcher(path, pwo);
	return pwo;
};

inline bool _delWatcher(std::string path)
{
	path = std::string("root/scripts/") + path;
	return WatcherPaths::root().delWatcher(path);
};

//-------------------------------------------------------------------------------------
static PyObject* addWatcher(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 3)
	{
		PyErr_Format(PyExc_Exception, "KBEngine::addWatcher: args error! "
			"arg(watcherName, deftype[UINT32|STRING...], pyCallable).\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyName = NULL;
	PyObject* pyType = NULL;
	PyObject* pyObj = NULL;
	
	if(!PyArg_ParseTuple(args, "O|O|O", &pyName, &pyType, &pyObj))
	{
		PyErr_Format(PyExc_Exception, "KBEngine::addWatcher: args error! "
			"arg(watcherPath, deftype[UINT32|STRING...], pyCallable).\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(!PyUnicode_Check(pyName))
	{
		PyErr_Format(PyExc_Exception, "KBEngine::addWatcher: args1 error! "
			"arg=watcherPath\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(!PyUnicode_Check(pyType))
	{
		PyErr_Format(PyExc_Exception, "KBEngine::addWatcher: args2 error! "
			"arg=deftype[UINT32|STRING...]\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	std::string watchername = PyUnicode_AsUTF8AndSize(pyName, NULL);
	std::string type = PyUnicode_AsUTF8AndSize(pyType, NULL);
	
	PyObject* pyObj1 = NULL;

	if(!PyCallable_Check(pyObj))
	{
		PyErr_Format(PyExc_Exception, "Baseapp::addWatcher: args3 error! "
			"arg=pyCallable.\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	pyObj1 = PyObject_CallFunction(pyObj, const_cast<char*>(""));
	if(!pyObj1)
	{
		PyErr_Clear();
		PyErr_Format(PyExc_Exception, "Baseapp::addWatcher: return error for args3! "
			"arg=pyCallable.\n");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	Py_INCREF(pyObj);

	if(strcmp("UINT8", type.c_str()) == 0)
	{
		_addWatcher<uint8>(watchername, pyObj);
	}
	else if(strcmp("UINT16", type.c_str()) == 0)
	{
		_addWatcher<uint16>(watchername, pyObj);
	}
	else if(strcmp("UINT32", type.c_str()) == 0)
	{
		_addWatcher<uint32>(watchername, pyObj);
	}
	else if(strcmp("UINT64", type.c_str()) == 0)
	{
		_addWatcher<uint64>(watchername, pyObj);
	}
	else if(strcmp("INT8", type.c_str()) == 0)
	{
		_addWatcher<int8>(watchername, pyObj);
	}
	else if(strcmp("INT16", type.c_str()) == 0)
	{
		_addWatcher<int16>(watchername, pyObj);
	}
	else if(strcmp("INT32", type.c_str()) == 0)
	{
		_addWatcher<int32>(watchername, pyObj);
	}
	else if(strcmp("INT64", type.c_str()) == 0)
	{
		_addWatcher<int64>(watchername, pyObj);
	}
	else if(strcmp("FLOAT", type.c_str()) == 0)
	{
		_addWatcher<float>(watchername, pyObj);
	}
	else if(strcmp("DOUBLE", type.c_str()) == 0)
	{
		_addWatcher<double>(watchername, pyObj);
	}
	else if(strcmp("BOOL", type.c_str()) == 0)
	{
		_addWatcher<uint8>(watchername, pyObj);
	}
	else if(strcmp("STRING", type.c_str()) == 0)
	{
		_addWatcher<std::string>(watchername, pyObj);
	}

	Py_DECREF(pyObj1);
	S_Return;
}

//-------------------------------------------------------------------------------------
static PyObject* delWatcher(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 1)
	{
		PyErr_Format(PyExc_Exception, "KBEngine::delWatcher: watcherName error!\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyName = NULL;
	
	if(!PyArg_ParseTuple(args, "O", &pyName))
	{
		PyErr_Format(PyExc_Exception, "KBEngine::delWatcher: watcherName error!\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(!PyUnicode_Check(pyName))
	{
		PyErr_Format(PyExc_Exception, "KBEngine::delWatcher: watcherName error!\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	bool ret = _delWatcher(PyUnicode_AsUTF8AndSize(pyName, NULL));
	
	if(!ret)
	{
		Py_RETURN_FALSE;
	}

	Py_RETURN_TRUE;
}

//-------------------------------------------------------------------------------------
bool initializePyWatcher(Script* pScript)
{
	// 注册产生uuid方法到py
	APPEND_SCRIPT_MODULE_METHOD(pScript->getModule(),		addWatcher,			addWatcher,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(pScript->getModule(),		delWatcher,			delWatcher,					METH_VARARGS,			0);
	return true;
}


}
}
