// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine {
namespace script{

template <class T>
INLINE T PyWatcherObject<T>::getValue()
{
	T v;

	PyObject* pyObj1 = PyObject_CallFunction(pyCallable_, const_cast<char*>(""));
	if(!pyObj1)
	{
		PyErr_Format(PyExc_Exception, "PyWatcherObject::addToStream: callFunction error! path=%s name=%s.\n",
			path(), name());
		PyErr_PrintEx(0);
	}
	else
	{
		readVal(pyObj1, v);
		SCRIPT_ERROR_CHECK();
		Py_DECREF(pyObj1);
	}

	return v;
}
	
}
}

