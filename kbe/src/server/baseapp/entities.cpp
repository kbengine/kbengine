#include "entities.hpp"
namespace KBEngine{

/** python Entities操作所需要的方法表 */
PyMappingMethods g_mappingMethods =
{
	(lenfunc)Entities::mp_length,					// mp_length
	(binaryfunc)Entities::mp_subscript,				// mp_subscript
	NULL											// mp_ass_subscript
};


SCRIPT_METHOD_DECLARE_BEGIN(Entities)
SCRIPT_METHOD_DECLARE("has_key",			_has_key,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("keys",				_keys,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("values",				_values,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE("items",				_items,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(Entities)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Entities)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(Entities, 0, 0, &g_mappingMethods, 0, 0)	

//-------------------------------------------------------------------------------------
int Entities::mp_length(PyObject * self)
{
	Entities* lpEntities = static_cast<Entities*>(self);
	ENTITYS_MAP& entities = lpEntities->getEntities();
	return entities.size();
}
	
//-------------------------------------------------------------------------------------
PyObject * Entities::mp_subscript(PyObject* self, PyObject* key /*entityID*/)
{
	Entities* lpEntities = static_cast<Entities*>(self);
	long entityID = PyLong_AsLong(key);
	if (PyErr_Occurred())
		return NULL;

	PyObject * pyEntity = NULL;

	ENTITYS_MAP& entities = lpEntities->getEntities();
	ENTITYS_MAP::const_iterator iter = entities.find(entityID);
	if (iter != entities.end())
		pyEntity = iter->second;

	if(pyEntity == NULL)
	{
		PyErr_Format(PyExc_KeyError, "%ld", entityID);
		return NULL;
	}

	Py_INCREF(pyEntity);
	return pyEntity;
}

//-------------------------------------------------------------------------------------
PyObject* Entities::_has_key(PyObject* self, PyObject *args)
{
	Entities* lpEntities = static_cast<Entities*>(self);

	long entityID;
	if (!PyArg_ParseTuple(args, "i", &entityID))
		return NULL;

	ENTITYS_MAP& entities = lpEntities->getEntities();
	return PyLong_FromLong((entities.find(entityID) != entities.end()));
}

//-------------------------------------------------------------------------------------
PyObject* Entities::_keys(PyObject* self, PyObject *args)
{
	Entities* lpEntities = static_cast<Entities*>(self);
	ENTITYS_MAP& entities = lpEntities->getEntities();
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
PyObject* Entities::_values(PyObject* self, PyObject *args)
{
	Entities* lpEntities = static_cast<Entities*>(self);
	ENTITYS_MAP& entities = lpEntities->getEntities();
	PyObject* pyList = PyList_New(entities.size());
	int i = 0;

	ENTITYS_MAP::const_iterator iter = entities.begin();
	while (iter != entities.end())
	{
		Py_INCREF(iter->second);							// PyObject Entity* 增加一个引用
		PyList_SET_ITEM(pyList, i, iter->second);

		i++;
		iter++;
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
PyObject* Entities::_items(PyObject* self, PyObject *args)
{
	Entities* lpEntities = static_cast<Entities*>(self);
	ENTITYS_MAP& entities = lpEntities->getEntities();
	PyObject* pyList = PyList_New(entities.size());
	int i = 0;

	ENTITYS_MAP::const_iterator iter = entities.begin();
	while (iter != entities.end())
	{
		PyObject * pTuple = PyTuple_New(2);
		PyObject* entityID = PyLong_FromLong(iter->first);
		Py_INCREF(iter->second);							// PyObject Entity* 增加一个引用

		PyTuple_SET_ITEM(pTuple, 0, entityID);
		PyTuple_SET_ITEM(pTuple, 1, iter->second);
		PyList_SET_ITEM(pyList, i, pTuple);
		i++;
		iter++;
	}

	return pyList;
}

//-------------------------------------------------------------------------------------
void Entities::add(ENTITY_ID id, Entity* entity)
{ 
	ENTITYS_MAP::const_iterator iter = _entities.find(id);
	if(iter != _entities.end())
	{
		ERROR_MSG("Bases::add::exist the entityID:%d", id);
		return;
	}

	_entities[id] = entity; 
}

//-------------------------------------------------------------------------------------
void Entities::clear(void)
{
	ENTITYS_MAP::const_iterator iter = _entities.begin();
	while (iter != _entities.end())
	{
		Entity* entity = iter->second;
		entity->destroy();
		iter++;
	}

	_entities.clear();
}

//-------------------------------------------------------------------------------------
Entity* Entities::find(ENTITY_ID id)
{
	ENTITYS_MAP::const_iterator iter = _entities.find(id);
	if(iter != _entities.end())
	{
		return iter->second;
	}
	
	return NULL;
}

//-------------------------------------------------------------------------------------
bool Entities::destroy(ENTITY_ID id)
{
	ENTITYS_MAP::iterator iter = _entities.find(id);
	if(iter != _entities.end())
	{
		Entity* entity = iter->second;
		_entities.erase(iter);
		entity->destroy();
		return true;
	}
	
	return false;
}

//-------------------------------------------------------------------------------------

}
