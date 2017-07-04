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


#include "map.h"

#ifndef CODE_INLINE
#include "map.inl"
#endif

namespace KBEngine{ namespace script{

/** python map操作所需要的方法表 */
PyMappingMethods Map::mappingMethods =
{
	(lenfunc)Map::mp_length,					// mp_length
	(binaryfunc)Map::mp_subscript,				// mp_subscript
	(objobjargproc)Map::mp_ass_subscript		// mp_ass_subscript
};

// 参考 objects/dictobject.c
// Hack to implement "key in dict"
PySequenceMethods Map::mappingSequenceMethods = 
{
    0,											/* sq_length */
    0,											/* sq_concat */
    0,											/* sq_repeat */
    0,											/* sq_item */
    0,											/* sq_slice */
    0,											/* sq_ass_item */
    0,											/* sq_ass_slice */
	PyMapping_HasKey,							/* sq_contains */
    0,											/* sq_inplace_concat */
    0,											/* sq_inplace_repeat */
};

SCRIPT_METHOD_DECLARE_BEGIN(Map)
SCRIPT_METHOD_DECLARE("has_key",			has_key,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("keys",				keys,				METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("values",				values,				METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("items",				items,				METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("update",				update,				METH_VARARGS,		0)	
SCRIPT_METHOD_DECLARE("get",				get,				METH_VARARGS,		0)	
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Map)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Map)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(Map, 0, &Map::mappingSequenceMethods, &Map::mappingMethods, 0, 0)	
	
//-------------------------------------------------------------------------------------
Map::Map(PyTypeObject* pyType, bool isInitialised):
ScriptObject(pyType, isInitialised)
{
	pyDict_ = PyDict_New();
}

//-------------------------------------------------------------------------------------
Map::~Map()
{
	Py_DECREF(pyDict_);
}

//-------------------------------------------------------------------------------------
int Map::mp_length(PyObject* self)
{
	return PyDict_Size(static_cast<Map*>(self)->pyDict_);
}

//-------------------------------------------------------------------------------------
int Map::mp_ass_subscript(PyObject* self, PyObject* key, PyObject* value)
{
	Map* lpScriptData = static_cast<Map*>(self);

	if (value == NULL)
	{
		lpScriptData->onDataChanged(key, value, true);
		return PyDict_DelItem(lpScriptData->pyDict_, key);
	}
	
	lpScriptData->onDataChanged(key, value);
	return PyDict_SetItem(lpScriptData->pyDict_, key, value);
}

//-------------------------------------------------------------------------------------
void Map::onDataChanged(PyObject* key, PyObject* value, bool isDelete)
{
}
	
//-------------------------------------------------------------------------------------
PyObject* Map::mp_subscript(PyObject* self, PyObject* key)
{
	Map* lpScriptData = static_cast<Map*>(self);

	PyObject* pyObj = PyDict_GetItem(lpScriptData->pyDict_, key);
	if (!pyObj)
		PyErr_SetObject(PyExc_KeyError, key);
	else
		Py_INCREF(pyObj);

	return pyObj;
}


//-------------------------------------------------------------------------------------
int Map::seq_contains(PyObject* self, PyObject* value)
{
	return PyDict_Contains(static_cast<Map*>(self)->pyDict_, value);
}

//-------------------------------------------------------------------------------------
PyObject* Map::__py_has_key(PyObject* self, PyObject* args)
{
	PyObject * pyVal = PySequence_GetItem(args, 0);
	if (!pyVal)
	{
		PyErr_SetObject(PyExc_KeyError, args);
		return NULL;
	}

	int ret = PyDict_Contains(static_cast<Map*>(self)->pyDict_, pyVal);

	Py_DECREF(pyVal);

	if (ret > 0)
	{
		Py_RETURN_TRUE;
	}
	else if (ret == -1)
	{
		PyErr_SetObject(PyExc_KeyError, args);
		return NULL;
	}

	Py_RETURN_FALSE;
}

//-------------------------------------------------------------------------------------
PyObject* Map::__py_get(PyObject* self, PyObject* args)
{
	PyObject * pyVal = PySequence_GetItem(args, 0);

	if (!pyVal)
	{
		PyErr_SetObject(PyExc_KeyError, args);
		return NULL;
	}

	PyObject* pyObj = PyDict_GetItem(static_cast<Map*>(self)->pyDict_, pyVal);

	Py_DECREF(pyVal);

	if (!pyObj)
	{
		if (PySequence_Size(args) > 1)
			return PySequence_GetItem(args, 1);
		else
			PyErr_SetObject(PyExc_KeyError, args);
	}
	else
	{
		Py_INCREF(pyObj);
	}

	return pyObj;
}

//-------------------------------------------------------------------------------------
PyObject* Map::__py_keys(PyObject* self, PyObject* args)
{
	return PyDict_Keys(static_cast<Map*>(self)->pyDict_);
}

//-------------------------------------------------------------------------------------
PyObject* Map::__py_values(PyObject* self, PyObject* args)
{
	return PyDict_Values(static_cast<Map*>(self)->pyDict_);
}

//-------------------------------------------------------------------------------------
PyObject* Map::__py_items(PyObject* self, PyObject* args)
{
	return PyDict_Items(static_cast<Map*>(self)->pyDict_);
}

//-------------------------------------------------------------------------------------
PyObject* Map::__py_update(PyObject* self, PyObject* args)
{
	PyObject * pyVal = PySequence_GetItem(args, 0);
	if (!pyVal)
	{
		PyErr_SetObject(PyExc_KeyError, args);
		return NULL;
	}

	PyDict_Update(static_cast<Map*>(self)->pyDict_, pyVal);

	Py_DECREF(pyVal);
	S_Return; 
}

//-------------------------------------------------------------------------------------

}
}
