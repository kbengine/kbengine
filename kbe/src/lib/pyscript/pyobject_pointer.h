// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_PYOBJECT_POINTER_H
#define KBE_PYOBJECT_POINTER_H

#include "common/smartpointer.h"

namespace KBEngine { 

typedef SmartPointer<PyObject> PyObjectPtr;

template<>
inline void incrementReferenceCount(const PyObject& obj)
{
	Py_INCREF( const_cast<PyObject*>( &obj ) );
};

template<>
inline void decrementReferenceCount(const PyObject& obj)
{
	Py_DECREF( const_cast<PyObject*>( &obj ) );
};

}
#endif // KBE_PYOBJECT_POINTER_H
