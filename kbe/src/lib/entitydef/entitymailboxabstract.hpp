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


#ifndef __ENTITY_MAILBOX_BASE_H__
#define __ENTITY_MAILBOX_BASE_H__
	
// common include	
#include "cstdkbe/cstdkbe.hpp"
//#include "network/channel.hpp"
#include "pyscript/scriptobject.hpp"
#include "entitydef/common.hpp"
#include "network/address.hpp"

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

namespace Mercury
{
class Channel;
class Bundle;
}

class EntityMailboxAbstract : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(EntityMailboxAbstract, ScriptObject)
public:
	EntityMailboxAbstract(PyTypeObject* scriptType, const Mercury::Address* pAddr, COMPONENT_ID componentID, 
	ENTITY_ID eid, uint16 utype, ENTITY_MAILBOX_TYPE type);
	~EntityMailboxAbstract();

	/** 获取entityID */
	ENTITY_ID getID()const{ return id_; }
	void setID(int id){ id_ = id; }
	DECLARE_PY_GET_MOTHOD(pyGetID);

	/** 获得组件ID */
	COMPONENT_ID getComponentID(void)const{ return componentID_; }

	/** 获得utype */
	uint16 getUType(void)const{ return utype_; }

	/** 获得type */
	ENTITY_MAILBOX_TYPE getType(void)const{ return type_; }

	/** 支持pickler 方法 */
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol);
	
	Mercury::Channel* getChannel(void);

	bool postMail(Mercury::Bundle& bundle);
	void newMail(Mercury::Bundle& bundle);

	const Mercury::Address& addr()const{ return addr_; }
	void addr(const Mercury::Address& saddr){ addr_ = saddr; }
protected:
	COMPONENT_ID							componentID_;			// 远端机器组件的ID
	Mercury::Address						addr_;					// 频道地址
	ENTITY_MAILBOX_TYPE						type_;					// 该mailbox的类型
	ENTITY_ID								id_;					// entityID
	uint16									utype_;					// entity的utype  按照entities.xml中的定义顺序
};

}
#endif
