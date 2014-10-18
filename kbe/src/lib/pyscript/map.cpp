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


#include "map.hpp"
namespace KBEngine{ namespace script{

/** python map操作所需要的方法表 */
PyMappingMethods Map::mappingMethods =
{
	(lenfunc)Map::mp_length,					// mp_length
	(binaryfunc)Map::mp_subscript,				// mp_subscript
	(objobjargproc)Map::mp_ass_subscript		// mp_ass_subscript
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
SCRIPT_INIT(Map, 0, 0, &Map::mappingMethods, 0, 0)	
	
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
	std::string skey = script::Pickler::pickle(key, 0);
	std::string sval = "";
	
	if (value == NULL)
	{
		lpScriptData->onDataChanged(skey, sval, true);
		return PyDict_DelItem(lpScriptData->pyDict_, key);
	}
	
	sval = script::Pickler::pickle(value, 0);

	lpScriptData->onDataChanged(skey, sval);
	return PyDict_SetItem(lpScriptData->pyDict_, key, value);
}

//-------------------------------------------------------------------------------------
void Map::onDataChanged(std::string& key, std::string& value, bool isDelete)
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
PyObject* Map::__py_has_key(PyObject* self, PyObject* args)
{
	PyObject* pyObj = PyObject_CallMethod(static_cast<Map*>(self)->pyDict_, 
		const_cast<char*>("get"), const_cast<char*>("O"), args);

	if (!pyObj)
	{
		PyErr_SetObject(PyExc_KeyError, args);
		Py_RETURN_FALSE;
	}
	else
	{
		if(pyObj != Py_None)
		{
			Py_DECREF(pyObj);
			Py_RETURN_TRUE; 
		}
	}

	Py_DECREF(pyObj);
	Py_RETURN_FALSE;
}

//-------------------------------------------------------------------------------------
PyObject* Map::__py_get(PyObject* self, PyObject* args)
{
	return PyObject_CallMethod(static_cast<Map*>(self)->pyDict_, 
		const_cast<char*>("get"), const_cast<char*>("O"), args);
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
	PyDict_Update(static_cast<Map*>(self)->pyDict_, args);
	S_Return; 
}

//-------------------------------------------------------------------------------------

}
}
