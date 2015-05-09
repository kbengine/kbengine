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


#ifndef KBE_CLIENT_ENTITY_H
#define KBE_CLIENT_ENTITY_H
	
// common include	
#include "common/common.h"
//#include "network/channel.h"
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

class ClientEntity : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(ClientEntity, ScriptObject)
public:
	ClientEntity(ENTITY_ID srcEntityID, ENTITY_ID clientEntityID);
	
	~ClientEntity();
	
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

protected:
	ENTITY_ID								srcEntityID_;						// srcEntityID_

	ENTITY_ID								clientEntityID_;					// clientEntityID_
};

}

#endif // KBE_CLIENT_ENTITY_H
