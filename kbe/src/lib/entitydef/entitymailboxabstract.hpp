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


#ifndef KBE_ENTITY_MAILBOX_BASE_HPP
#define KBE_ENTITY_MAILBOX_BASE_HPP
	
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
	/** ���໯ ��һЩpy�������������� */
	INSTANCE_SCRIPT_HREADER(EntityMailboxAbstract, ScriptObject)
public:
	EntityMailboxAbstract(PyTypeObject* scriptType, 
		const Mercury::Address* pAddr, 
		COMPONENT_ID componentID, 
		ENTITY_ID eid, 
		uint16 utype, 
		ENTITY_MAILBOX_TYPE type);
	
	virtual ~EntityMailboxAbstract();

	/** 
		��ȡentityID 
	*/
	INLINE ENTITY_ID id()const;

	INLINE void id(int v);

	DECLARE_PY_GET_MOTHOD(pyGetID);

	/** 
		������ID 
	*/
	INLINE COMPONENT_ID componentID(void)const;

	/** 
		���������ID 
	*/
	INLINE void componentID(COMPONENT_ID cid);

	/** 
		���utype 
	*/
	INLINE ENTITY_SCRIPT_UID utype(void)const;

	/** 
		���type 
	*/
	INLINE ENTITY_MAILBOX_TYPE type(void)const;

	/** 
		֧��pickler ���� 
	*/
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol);
	
	virtual Mercury::Channel* getChannel(void) = 0;

	virtual bool postMail(Mercury::Bundle& bundle);

	virtual void newMail(Mercury::Bundle& bundle);
	
	const Mercury::Address& addr()const{ return addr_; }
	void addr(const Mercury::Address& saddr){ addr_ = saddr; }

	INLINE bool isClient()const;
	INLINE bool isCell()const;
	INLINE bool isCellReal()const;
	INLINE bool isCellViaBase()const;
	INLINE bool isBase()const;
	INLINE bool isBaseReal()const;
	INLINE bool isBaseViaCell()const;
protected:
	COMPONENT_ID							componentID_;			// Զ�˻��������ID
	Mercury::Address						addr_;					// Ƶ����ַ
	ENTITY_MAILBOX_TYPE						type_;					// ��mailbox������
	ENTITY_ID								id_;					// entityID
	ENTITY_SCRIPT_UID						utype_;					// entity��utype  ����entities.xml�еĶ���˳��
};

}

#ifdef CODE_INLINE
#include "entitymailboxabstract.ipp"
#endif
#endif // KBE_ENTITY_MAILBOX_BASE_HPP
