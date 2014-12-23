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

#ifndef KBE_PYOBJECT_CALL_TEMPLATE_H
#define KBE_PYOBJECT_CALL_TEMPLATE_H

namespace KBEngine { 

#define SCRIPT_OBJECT_CALL_ARGS0(OBJ, METHOT_NAME)														\
{																										\
	if(PyObject_HasAttrString(OBJ, METHOT_NAME))														\
	{																									\
		PyObject* pyResult = PyObject_CallMethod((OBJ), (METHOT_NAME), 									\
															const_cast<char*>(""));						\
		if(pyResult != NULL)																			\
			Py_DECREF(pyResult);																		\
		else																							\
			PyErr_PrintEx(0);																			\
	}																									\
}																										\
			
			
#define SCRIPT_OBJECT_CALL_ARGS1(OBJ, METHOT_NAME, FORMAT, ARG1)										\
{																										\
	if(PyObject_HasAttrString(OBJ, METHOT_NAME))														\
	{																									\
		PyObject* pyResult = PyObject_CallMethod((OBJ), 												\
												(METHOT_NAME), 											\
												(FORMAT),												\
												(ARG1)													\
													);													\
		if(pyResult != NULL)																			\
			Py_DECREF(pyResult);																		\
		else																							\
			PyErr_PrintEx(0);																			\
	}																									\
}																										\
			
			
#define SCRIPT_OBJECT_CALL_ARGS2(OBJ, METHOT_NAME, FORMAT, ARG1, ARG2)									\
{																										\
	if(PyObject_HasAttrString(OBJ, METHOT_NAME))														\
	{																									\
		PyObject* pyResult = PyObject_CallMethod((OBJ), 												\
												(METHOT_NAME), 											\
												(FORMAT),												\
												(ARG1),													\
												(ARG2)													\
													);													\
		if(pyResult != NULL)																			\
			Py_DECREF(pyResult);																		\
		else																							\
			PyErr_PrintEx(0);																			\
	}																									\
}																										\


#define SCRIPT_OBJECT_CALL_ARGS3(OBJ, METHOT_NAME, FORMAT, ARG1, ARG2, ARG3)							\
{																										\
	if(PyObject_HasAttrString(OBJ, METHOT_NAME))														\
	{																									\
		PyObject* pyResult = PyObject_CallMethod((OBJ), 												\
												(METHOT_NAME), 											\
												(FORMAT),												\
												(ARG1),													\
												(ARG2),													\
												(ARG3)													\
													);													\
		if(pyResult != NULL)																			\
			Py_DECREF(pyResult);																		\
		else																							\
			PyErr_PrintEx(0);																			\
	}																									\
}																										\


#define SCRIPT_OBJECT_CALL_ARGS4(OBJ, METHOT_NAME, FORMAT, ARG1, ARG2, ARG3, ARG4)						\
{																										\
	if(PyObject_HasAttrString(OBJ, METHOT_NAME))														\
	{																									\
		PyObject* pyResult = PyObject_CallMethod((OBJ), 												\
												(METHOT_NAME), 											\
												(FORMAT),												\
												(ARG1),													\
												(ARG2),													\
												(ARG3),													\
												(ARG4)													\
													);													\
		if(pyResult != NULL)																			\
			Py_DECREF(pyResult);																		\
		else																							\
			PyErr_PrintEx(0);																			\
	}																									\
}																										\


#define SCRIPT_OBJECT_CALL_ARGS5(OBJ, METHOT_NAME, FORMAT, ARG1, ARG2, ARG3, ARG4, ARG5)				\
{																										\
	if(PyObject_HasAttrString(OBJ, METHOT_NAME))														\
	{																									\
		PyObject* pyResult = PyObject_CallMethod((OBJ), 												\
												(METHOT_NAME), 											\
												(FORMAT),												\
												(ARG1),													\
												(ARG2),													\
												(ARG3),													\
												(ARG4),													\
												(ARG5)													\
													);													\
		if(pyResult != NULL)																			\
			Py_DECREF(pyResult);																		\
		else																							\
			PyErr_PrintEx(0);																			\
	}																									\
}																										\


}
#endif // KBE_PYOBJECT_CALL_TEMPLATE_H
