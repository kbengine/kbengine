/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __PYOBJECT_POINTER__
#define __PYOBJECT_POINTER__
namespace KBEngine { 

typedef SmartPointer<PyObject> PyObjectPtr;

template<>
inline void incrementReferenceCount(const PyObject* obj)
{
	Py_INCREF( const_cast<PyObject*>( obj ) );
};

template<>
inline void decrementReferenceCount(const PyObject* obj)
{
	Py_DECREF( const_cast<PyObject*>( obj ) );
};

}
#endif // __PYOBJECT_POINT__