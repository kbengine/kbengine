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


#ifndef __ENTITIES_H__
#define __ENTITIES_H__
	
// common include	
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "pyscript/scriptobject.hpp"
#include "../../server/cellapp/entity.hpp"
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

class Entities : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(Entities, ScriptObject)	
public:
	typedef std::tr1::unordered_map<ENTITY_ID, Entity*> ENTITYS_MAP;

	Entities():
	ScriptObject(getScriptType(), false)
	{			
	}

	~Entities()
	{
	}	

	/** 暴露一些字典方法给python */
	DECLARE_PY_MOTHOD_ARG1(pyHas_key, ENTITY_ID);
	DECLARE_PY_MOTHOD_ARG0(pyKeys);
	DECLARE_PY_MOTHOD_ARG0(pyValues);
	DECLARE_PY_MOTHOD_ARG0(pyItems);

	/** map操作函数相关 */
	static PyObject* mp_subscript(PyObject * self, PyObject * key);
	static int mp_length(PyObject * self);
	static PyMappingMethods mappingMethods;

	ENTITYS_MAP& getEntities(void){ return _entities; }
	void add(ENTITY_ID id, Entity* entity);
	void clear(void);
	Entity* erase(ENTITY_ID id);
	Entity* find(ENTITY_ID id);
private:
	ENTITYS_MAP _entities;
};

}
#endif
