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


#ifndef KBE_ALL_CLIENTS_H
#define KBE_ALL_CLIENTS_H

#include "common/common.h"
#include "pyscript/scriptobject.h"
#include "entitydef/common.h"
#include "network/address.h"

//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <map>	
#include <vector>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#include <time.h> 
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

namespace Network
{
class Channel;
class Bundle;
}

class ScriptDefModule;

class AllClients : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(AllClients, ScriptObject)
public:
	AllClients(const ScriptDefModule* pScriptModule, 
		ENTITY_ID eid, 
		bool otherClients);
	
	~AllClients();
	
	/** 
		脚本请求获取属性或者方法 
	*/
	PyObject* onScriptGetAttribute(PyObject* attr);						
			
	/** 
		获得对象的描述 
	*/
	PyObject* tp_repr();
	PyObject* tp_str();
	
	void c_str(char* s, size_t size);
	
	/** 
		获取entityID 
	*/
	ENTITY_ID id() const{ return id_; }
	void setID(int id){ id_ = id; }
	DECLARE_PY_GET_MOTHOD(pyGetID);

	void setScriptModule(const ScriptDefModule*	pScriptModule){ 
		pScriptModule_ = pScriptModule; 
	}

protected:
	const ScriptDefModule*					pScriptModule_;			// 该entity所使用的脚本模块对象

	ENTITY_ID								id_;					// entityID

	bool									otherClients_;			// 是否只是其他客户端， 不包括自己
};

}

#endif // KBE_ALL_CLIENTS_H
