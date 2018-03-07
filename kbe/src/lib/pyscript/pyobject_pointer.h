/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_PYOBJECT_POINTER_H
#define KBE_PYOBJECT_POINTER_H

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
