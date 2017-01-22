/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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


namespace KBEngine {
namespace script{

template <class T>
INLINE T PyWatcherObject<T>::getValue()
{
	T v;

	PyObject* pyObj1 = PyObject_CallFunction(pyCallable_, const_cast<char*>(""));
	if(!pyObj1)
	{
		PyErr_Format(PyExc_Exception, "PyWatcherObject::addToStream: callFunction is error! path=%s name=%s.\n",
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

