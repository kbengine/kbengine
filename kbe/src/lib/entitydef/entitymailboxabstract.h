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


#ifndef KBE_ENTITY_MAILBOX_BASE_H
#define KBE_ENTITY_MAILBOX_BASE_H
	
#include "common/common.h"
//#include "network/channel.h"
#include "pyscript/scriptobject.h"
#include "entitydef/common.h"
#include "network/address.h"
	
namespace KBEngine{

namespace Network
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
		const Network::Address* pAddr, 
		COMPONENT_ID componentID, 
		ENTITY_ID eid, 
		uint16 utype, 
		ENTITY_MAILBOX_TYPE type);
	
	virtual ~EntityMailboxAbstract();

	/** 
		��ȡentityID 
	*/
	INLINE ENTITY_ID id() const;

	INLINE void id(int v);

	DECLARE_PY_GET_MOTHOD(pyGetID);

	/** 
		������ID 
	*/
	INLINE COMPONENT_ID componentID(void) const;

	/** 
		���������ID 
	*/
	INLINE void componentID(COMPONENT_ID cid);

	/** 
		���utype 
	*/
	INLINE ENTITY_SCRIPT_UID utype(void) const;

	/** 
		���type 
	*/
	INLINE ENTITY_MAILBOX_TYPE type(void) const;

	/** 
		֧��pickler ���� 
	*/
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol);
	
	virtual Network::Channel* getChannel(void) = 0;

	virtual bool postMail(Network::Bundle* pBundle);

	virtual void newMail(Network::Bundle& bundle);
	
	const Network::Address& addr() const{ return addr_; }
	void addr(const Network::Address& saddr){ addr_ = saddr; }

	INLINE bool isClient() const;
	INLINE bool isCell() const;
	INLINE bool isCellReal() const;
	INLINE bool isCellViaBase() const;
	INLINE bool isBase() const;
	INLINE bool isBaseReal() const;
	INLINE bool isBaseViaCell() const;
	
protected:
	COMPONENT_ID							componentID_;			// Զ�˻��������ID
	Network::Address						addr_;					// Ƶ����ַ
	ENTITY_MAILBOX_TYPE						type_;					// ��mailbox������
	ENTITY_ID								id_;					// entityID
	ENTITY_SCRIPT_UID						utype_;					// entity��utype  ����entities.xml�еĶ���˳��
};

}

#ifdef CODE_INLINE
#include "entitymailboxabstract.inl"
#endif
#endif // KBE_ENTITY_MAILBOX_BASE_H
