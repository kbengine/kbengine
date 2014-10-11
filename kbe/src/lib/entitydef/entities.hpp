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

#ifndef KBE_ENTITIES_HPP
#define KBE_ENTITIES_HPP
	
// common include	
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "pyscript/scriptobject.hpp"
#include "pyscript/pyobject_pointer.hpp"
//#define NDEBUG
#include <map>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#include <unordered_map>
#else
// linux include
#include <errno.h>
#include <tr1/unordered_map>
#endif
	
namespace KBEngine{

template<typename T>
class Entities : public script::ScriptObject
{
	/** 
		���໯ ��һЩpy�������������� 
	*/
	INSTANCE_SCRIPT_HREADER(Entities, ScriptObject)	
public:
	typedef KBEUnordered_map<ENTITY_ID, PyObjectPtr> ENTITYS_MAP;

	Entities():
	ScriptObject(getScriptType(), false)
	{			
	}

	~Entities()
	{
		finalise();
	}	

	void finalise()
	{
		clear(false);
	}

	/** 
		��¶һЩ�ֵ䷽����python 
	*/
	DECLARE_PY_MOTHOD_ARG1(pyHas_key, ENTITY_ID);
	DECLARE_PY_MOTHOD_ARG0(pyKeys);
	DECLARE_PY_MOTHOD_ARG0(pyValues);
	DECLARE_PY_MOTHOD_ARG0(pyItems);
	
	static PyObject* __py_pyGet(PyObject * self, 
		PyObject * args, PyObject* kwds);

	/** 
		map����������� 
	*/
	static PyObject* mp_subscript(PyObject * self, PyObject * key);

	static int mp_length(PyObject * self);

	static PyMappingMethods mappingMethods;

	ENTITYS_MAP& getEntities(void){ return _entities; }

	void add(ENTITY_ID id, T* entity);
	void clear(bool callScript);
	void clear(bool callScript, std::vector<ENTITY_ID> excludes);
	PyObjectPtr erase(ENTITY_ID id);

	T* find(ENTITY_ID id);

	size_t size()const { return _entities.size(); }
private:
	ENTITYS_MAP _entities;
};

/** 
	python Entities��������Ҫ�ķ����� 
*/
template<typename T>
PyMappingMethods Entities<T>::mappingMethods =
{
	(lenfunc)mp_length,								// mp_length
	(binaryfunc)mp_subscript,						// mp_subscript
	NULL											// mp_ass_subscript
};

TEMPLATE_SCRIPT_METHOD_DECLARE_BEGIN(template<typename T>, Entities<T>, Entities)
SCRIPT_METHOD_DECLARE("has_key",			pyHas_key,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("keys",				pyKeys,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("values",				pyValues,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("items",				pyItems,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("get",				pyGet,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE_END()

TEMPLATE_SCRIPT_MEMBER_DECLARE_BEGIN(template<typename T>, Entities<T>, Entities)
SCRIPT_MEMBER_DECLARE_END()

TEMPLATE_SCRIPT_GETSET_DECLARE_BEGIN(template<typename T>, Entities<T>, Entities)
SCRIPT_GETSET_DECLARE_END()
TEMPLATE_SCRIPT_INIT(template<typename T>, Entities<T>, Entities, 0, 0, &Entities<T>::mappingMethods, 0, 0)	


//-------------------------------------------------------------------------------------
template<typename T>
int Entities<T>::mp_length(PyObject * self)
{
	return static_cast<Entities*>(self)->getEntities().size();
}
	
//-------------------------------------------------------------------------------------
template<typename T>
PyObject * Entities<T>::mp_subscript(PyObject* self, PyObject* key /*entityID*/)
{
	Entities* lpEntities = static_cast<Entities*>(self);
	ENTITY_ID entityID = PyLong_AsLong(key);
	if (PyErr_Occurred())
		return NULL;

	PyObject * pyEntity = NULL;

	ENTITYS_MAP& entities = lpEntities->getEntities();
	ENTITYS_MAP::const_iterator iter = entities.find(entityID);
	if (iter != entities.end())
		pyEntity = iter->second.get();

	if(pyEntity == NULL)
	{
		PyErr_Format(PyExc_KeyError, "%d", entityID);
		PyErr_PrintEx(0);
		return NULL;
	}

	Py_INCREF(pyEntity);
	return pyEntity;
}

//-------------------------------------------------------------------------------------
template<typename T>
PyObject* Entities<T>::pyHas_key(ENTITY_ID entityID)
{
	ENTITYS_MAP& entities = getEntities();
	return PyLong_FromLong((entities.find(entityID) != entities.end()));
}

//-------------------------------------------------------------------------------------
template<typename T>
PyObject* Entities<T>::pyKeys()
{
	ENTITYS_MAP& entities = getEntities();
	PyObject* pyList = PyList_New(entities.size());
	int i = 0;

	ENTITYS_MAP::const_iterator iter = entities.begin();
	while (iter != entities.end())
	{
		PyObject* entityID = PyLong_FromLong(iter->first);
		PyList_SET_ITEM(pyList, i, entityID);

		i++;
		iter++;
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
template<typename T>
PyObject* Entities<T>::pyValues()
{
	ENTITYS_MAP& entities = getEntities();
	PyObject* pyList = PyList_New(entities.size());
	int i = 0;

	ENTITYS_MAP::const_iterator iter = entities.begin();
	while (iter != entities.end())
	{
		Py_INCREF(iter->second.get());							// PyObject Entity* ����һ������
		PyList_SET_ITEM(pyList, i, iter->second.get());

		i++;
		iter++;
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
template<typename T>
PyObject* Entities<T>::pyItems()
{
	ENTITYS_MAP& entities = getEntities();
	PyObject* pyList = PyList_New(entities.size());
	int i = 0;

	ENTITYS_MAP::const_iterator iter = entities.begin();
	while (iter != entities.end())
	{
		PyObject * pTuple = PyTuple_New(2);
		PyObject* entityID = PyLong_FromLong(iter->first);
		Py_INCREF(iter->second.get());							// PyObject Entity* ����һ������

		PyTuple_SET_ITEM(pTuple, 0, entityID);
		PyTuple_SET_ITEM(pTuple, 1, iter->second.get());
		PyList_SET_ITEM(pyList, i, pTuple);
		i++;
		iter++;
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
template<typename T>
PyObject* Entities<T>::__py_pyGet(PyObject* self, PyObject * args, PyObject* kwds)
{
	Entities* lpEntities = static_cast<Entities*>(self);
	PyObject * pDefault = Py_None;
	ENTITY_ID id = 0;
	if (!PyArg_ParseTuple( args, "i|O", &id, &pDefault))
	{
		return NULL;
	}

	PyObject* pEntity = lpEntities->find(id);

	if (!pEntity)
	{
		pEntity = pDefault;
	}

	Py_INCREF(pEntity);
	return pEntity;
}

//-------------------------------------------------------------------------------------
template<typename T>
void Entities<T>::add(ENTITY_ID id, T* entity)
{ 
	ENTITYS_MAP::const_iterator iter = _entities.find(id);
	if(iter != _entities.end())
	{
		ERROR_MSG(fmt::format("Entities::add: entityID:{} has exist\n.", id));
		return;
	}

	_entities[id] = entity; 
}

//-------------------------------------------------------------------------------------
template<typename T>
void Entities<T>::clear(bool callScript)
{
	ENTITYS_MAP::const_iterator iter = _entities.begin();
	while (iter != _entities.end())
	{
		T* entity = (T*)iter->second.get();
		entity->destroy(callScript);
		iter++;
	}

	_entities.clear();
}

//-------------------------------------------------------------------------------------
template<typename T>
void Entities<T>::clear(bool callScript, std::vector<ENTITY_ID> excludes)
{
	ENTITYS_MAP::const_iterator iter = _entities.begin();
	for (;iter != _entities.end();)
	{
		if(std::find(excludes.begin(), excludes.end(), iter->first) != excludes.end())
		{
			iter++;
			continue;
		}

		T* entity = (T*)iter->second.get();
		entity->destroy(callScript);
		_entities.erase(iter++);
	}
	
	// ���ڴ���excludes�������
	// _entities.clear();
}

//-------------------------------------------------------------------------------------
template<typename T>
T* Entities<T>::find(ENTITY_ID id)
{
	ENTITYS_MAP::const_iterator iter = _entities.find(id);
	if(iter != _entities.end())
	{
		return static_cast<T*>(iter->second.get());
	}
	
	return NULL;
}

//-------------------------------------------------------------------------------------
template<typename T>
PyObjectPtr Entities<T>::erase(ENTITY_ID id)
{
	ENTITYS_MAP::iterator iter = _entities.find(id);
	if(iter != _entities.end())
	{
		T* entity = static_cast<T*>(iter->second.get());
		_entities.erase(iter);
		return entity;
	}
	
	return NULL;
}

}
#endif // KBE_ENTITIES_HPP

