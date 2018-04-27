// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_ENTITYGARBAGES_H
#define KBE_ENTITYGARBAGES_H
	
// common include	
#include "helper/debug_helper.h"
#include "common/common.h"
#include "common/smartpointer.h"
#include "pyscript/scriptobject.h"
#include "pyscript/pyobject_pointer.h"
	
namespace KBEngine{

template<typename T>
class EntityGarbages : public script::ScriptObject
{
	/** 
		子类化 将一些py操作填充进派生类 
	*/
	INSTANCE_SCRIPT_HREADER(EntityGarbages, ScriptObject)	
public:
	typedef KBEUnordered_map<ENTITY_ID, PyObject*> ENTITYS_MAP;

	EntityGarbages():
	ScriptObject(getScriptType(), false),
	_entities(),
	_lastTime(0)
	{			
	}

	~EntityGarbages()
	{
		if(size() > 0)
		{
			ERROR_MSG(fmt::format("EntityGarbages::~EntityGarbages(): leaked, size={}.\n", 
				size()));

			int i = 0;

			ENTITYS_MAP::iterator iter = _entities.begin();
			for(; iter != _entities.end(); ++iter)
			{
				if(i++ >= 256)
					break;

				ERROR_MSG(fmt::format("\t--> leaked: {}({}).\n", 
					iter->second->ob_type->tp_name, iter->first));
			}
		}

		finalise();
	}	

	void finalise()
	{
		clear();
	}

	/** 
		暴露一些字典方法给python 
	*/
	DECLARE_PY_MOTHOD_ARG1(pyHas_key, ENTITY_ID);
	DECLARE_PY_MOTHOD_ARG0(pyKeys);
	DECLARE_PY_MOTHOD_ARG0(pyValues);
	DECLARE_PY_MOTHOD_ARG0(pyItems);
	
	static PyObject* __py_pyGet(PyObject * self, 
		PyObject * args, PyObject* kwds);

	/** 
		map操作函数相关 
	*/
	static PyObject* mp_subscript(PyObject* self, PyObject* key);

	static int mp_length(PyObject* self);

	static PyMappingMethods mappingMethods;
	static PySequenceMethods mappingSequenceMethods;

	ENTITYS_MAP& getEntities(void){ return _entities; }

	void add(ENTITY_ID id, T* entity);
	void clear();
	void erase(ENTITY_ID id);

	T* find(ENTITY_ID id);

	size_t size() const { return _entities.size(); }
		
private:
	ENTITYS_MAP _entities;
	uint64 _lastTime;
};

/** 
	Python EntityGarbages操作所需要的方法表 
*/
template<typename T>
PyMappingMethods EntityGarbages<T>::mappingMethods =
{
	(lenfunc)mp_length,								// mp_length
	(binaryfunc)mp_subscript,						// mp_subscript
	NULL											// mp_ass_subscript
};

// 参考 objects/dictobject.c
// Hack to implement "key in dict"
template<typename T>
PySequenceMethods EntityGarbages<T>::mappingSequenceMethods = 
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

TEMPLATE_SCRIPT_METHOD_DECLARE_BEGIN(template<typename T>, EntityGarbages<T>, EntityGarbages)
SCRIPT_METHOD_DECLARE("has_key",			pyHas_key,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("keys",				pyKeys,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("values",				pyValues,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("items",				pyItems,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("get",				pyGet,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE_END()

TEMPLATE_SCRIPT_MEMBER_DECLARE_BEGIN(template<typename T>, EntityGarbages<T>, EntityGarbages)
SCRIPT_MEMBER_DECLARE_END()

TEMPLATE_SCRIPT_GETSET_DECLARE_BEGIN(template<typename T>, EntityGarbages<T>, EntityGarbages)
SCRIPT_GETSET_DECLARE_END()
TEMPLATE_SCRIPT_INIT(template<typename T>, EntityGarbages<T>, EntityGarbages, 0, &EntityGarbages<T>::mappingSequenceMethods, &EntityGarbages<T>::mappingMethods, 0, 0)	

//-------------------------------------------------------------------------------------
template<typename T>
int EntityGarbages<T>::mp_length(PyObject * self)
{
	return (int)static_cast<EntityGarbages*>(self)->getEntities().size();
}
	
//-------------------------------------------------------------------------------------
template<typename T>
PyObject* EntityGarbages<T>::mp_subscript(PyObject* self, PyObject* key /*entityID*/)
{
	EntityGarbages* lpEntities = static_cast<EntityGarbages*>(self);
	ENTITY_ID entityID = PyLong_AsLong(key);
	if (PyErr_Occurred())
		return NULL;

	PyObject * pyEntity = NULL;

	ENTITYS_MAP& entities = lpEntities->getEntities();
	ENTITYS_MAP::const_iterator iter = entities.find(entityID);
	if (iter != entities.end())
		pyEntity = iter->second;

	if(pyEntity == NULL)
	{
		PyErr_Format(PyExc_KeyError, "%d", entityID);
		//PyErr_PrintEx(0);
		return NULL;
	}

	Py_INCREF(pyEntity);
	return pyEntity;
}

//-------------------------------------------------------------------------------------
template<typename T>
PyObject* EntityGarbages<T>::pyHas_key(ENTITY_ID entityID)
{
	ENTITYS_MAP& entities = getEntities();
	return PyLong_FromLong((entities.find(entityID) != entities.end()));
}

//-------------------------------------------------------------------------------------
template<typename T>
PyObject* EntityGarbages<T>::pyKeys()
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
PyObject* EntityGarbages<T>::pyValues()
{
	ENTITYS_MAP& entities = getEntities();
	PyObject* pyList = PyList_New(entities.size());
	int i = 0;

	ENTITYS_MAP::const_iterator iter = entities.begin();
	while (iter != entities.end())
	{
		Py_INCREF(iter->second);
		PyList_SET_ITEM(pyList, i, iter->second);

		i++;
		iter++;
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
template<typename T>
PyObject* EntityGarbages<T>::pyItems()
{
	ENTITYS_MAP& entities = getEntities();
	PyObject* pyList = PyList_New(entities.size());
	int i = 0;

	ENTITYS_MAP::const_iterator iter = entities.begin();
	while (iter != entities.end())
	{
		PyObject * pTuple = PyTuple_New(2);
		PyObject* entityID = PyLong_FromLong(iter->first);
		Py_INCREF(iter->second);

		PyTuple_SET_ITEM(pTuple, 0, entityID);
		PyTuple_SET_ITEM(pTuple, 1, iter->second);
		PyList_SET_ITEM(pyList, i, pTuple);
		i++;
		iter++;
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
template<typename T>
PyObject* EntityGarbages<T>::__py_pyGet(PyObject* self, PyObject * args, PyObject* kwds)
{
	EntityGarbages* lpEntities = static_cast<EntityGarbages*>(self);
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
void EntityGarbages<T>::add(ENTITY_ID id, T* entity)
{ 
	ENTITYS_MAP::const_iterator iter = _entities.find(id);
	if(iter != _entities.end())
	{
		ERROR_MSG(fmt::format("EntityGarbages::add: entityID:{} has exist\n.", id));
		return;
	}

	if(_entities.size() == 0)
	{
		_lastTime = timestamp();
	}
	else
	{
		// X秒内没有清空过garbages则错误警告
		if(_lastTime > 0 && timestamp() - _lastTime > uint64(stampsPerSecond()) * 3600)
		{
			// 再未清空情况下，下次不提示了
			_lastTime = 0;
			
			ERROR_MSG(fmt::format("For a long time(3600s) not to empty the garbages, there may be a leak of the entitys(size:{}), "
				"please use the \"KBEngine.entities.garbages.items()\" command query!\n", 
				size()));
		}
	}
	
	_entities[id] = entity; 
}

//-------------------------------------------------------------------------------------
template<typename T>
void EntityGarbages<T>::clear()
{
	_entities.clear();
	_lastTime = 0;
}

//-------------------------------------------------------------------------------------
template<typename T>
T* EntityGarbages<T>::find(ENTITY_ID id)
{
	ENTITYS_MAP::const_iterator iter = _entities.find(id);
	if(iter != _entities.end())
	{
		return static_cast<T*>(iter->second);
	}
	
	return NULL;
}

//-------------------------------------------------------------------------------------
template<typename T>
void EntityGarbages<T>::erase(ENTITY_ID id)
{
	_entities.erase(id);
	
	if(_entities.size() == 0)
		_lastTime = 0;
}

}
#endif // KBE_ENTITYGARBAGES_H

