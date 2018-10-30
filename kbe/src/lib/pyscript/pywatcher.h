// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBENGINE_PY_WATCHER_H
#define KBENGINE_PY_WATCHER_H

#include "scriptobject.h"
#include "helper/watcher.h"

namespace KBEngine{ namespace script{

class Script;

/*
	使watcher能够监视到py脚本中的数据
*/
template <class T>
class PyWatcherObject : public WatcherObject
{
public:
	PyWatcherObject(std::string path, PyObject* pyCallable):
	  WatcherObject(path),
	  pyCallable_(pyCallable)
	{
	}

	virtual ~PyWatcherObject()
	{
		Py_DECREF(pyCallable_);
	}

	void addToInitStream(MemoryStream* s){
		(*s) << path() << name() << id_ << type<T>() << getValue();
	};

	void addToStream(MemoryStream* s){
		(*s) << id_ << getValue();
	};

	INLINE T getValue();
	WATCHER_VALUE_TYPE getType(){ return type<T>(); }

	void readVal(PyObject* pyVal, T& v)
	{
	}

private:
	PyObject* pyCallable_;
};

template <>
inline void PyWatcherObject<uint8>::readVal(PyObject* pyVal, uint8& v)
{
	v = (uint8)PyLong_AsLong(pyVal);
}

template <>
inline void PyWatcherObject<uint16>::readVal(PyObject* pyVal, uint16& v)
{
	v = (uint16)PyLong_AsLong(pyVal);
}

template <>
inline void PyWatcherObject<uint32>::readVal(PyObject* pyVal, uint32& v)
{
	v = (uint32)PyLong_AsLong(pyVal);
}

template <>
inline void PyWatcherObject<uint64>::readVal(PyObject* pyVal, uint64& v)
{
	v = (uint64)PyLong_AsLong(pyVal);
}

template <>
inline void PyWatcherObject<int8>::readVal(PyObject* pyVal, int8& v)
{
	v = (int8)PyLong_AsLong(pyVal);
}

template <>
inline void PyWatcherObject<int16>::readVal(PyObject* pyVal, int16& v)
{
	v = (int16)PyLong_AsLong(pyVal);
}

template <>
inline void PyWatcherObject<int32>::readVal(PyObject* pyVal, int32& v)
{
	v = (int32)PyLong_AsLong(pyVal);
}

template <>
inline void PyWatcherObject<int64>::readVal(PyObject* pyVal, int64& v)
{
	v = (int64)PyLong_AsLong(pyVal);
}

template <>
inline void PyWatcherObject<bool>::readVal(PyObject* pyVal, bool& v)
{
	v = PyLong_AsLong(pyVal) > 0;
}

template <>
inline void PyWatcherObject<float>::readVal(PyObject* pyVal, float& v)
{
	v = (float)PyFloat_AsDouble(pyVal);
}

template <>
inline void PyWatcherObject<double>::readVal(PyObject* pyVal, double& v)
{
	v = PyFloat_AsDouble(pyVal);
}

template <>
inline void PyWatcherObject<std::string>::readVal(PyObject* pyVal, std::string& v)
{
	v = PyUnicode_AsUTF8AndSize(pyVal, NULL);
}

bool initializePyWatcher(Script* pScript);

}
}

#ifdef CODE_INLINE
#include "pywatcher.inl"
#endif

#endif // KBENGINE_PY_WATCHER_H
