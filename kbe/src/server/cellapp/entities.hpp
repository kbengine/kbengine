/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __ENTITIES_H__
#define __ENTITIES_H__
	
// common include	
#include "entity.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "pyscript/scriptobject.hpp"
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

typedef std::tr1::unordered_map<ENTITY_ID, Entity*> ENTITYS_MAP;

class Entities : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(Entities, ScriptObject)	
public:
	Entities():
	ScriptObject(getScriptType(), false)
	{			
	}

	~Entities()
	{
	}	

	/** 暴露一些字典方法给python */
	static PyObject* _has_key(PyObject* self, PyObject *args);
	static PyObject* _keys(PyObject* self, PyObject *args);
	static PyObject* _values(PyObject* self, PyObject *args);
	static PyObject* _items(PyObject* self, PyObject *args);

	/** map操作函数相关 */
	static PyObject *	mp_subscript(PyObject * self, PyObject * key);
	static int mp_length(PyObject * self);

	ENTITYS_MAP& getEntities(void){ return _entities; }
	void add(ENTITY_ID id, Entity* entity);
	void clear(void);
	bool destroy(ENTITY_ID id);
	Entity* find(ENTITY_ID id);
private:
	ENTITYS_MAP _entities;
};

}
#endif
