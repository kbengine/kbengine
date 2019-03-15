// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_ENTITYCALL_BASE_H
#define KBE_ENTITYCALL_BASE_H
	
#include "common/common.h"
#include "pyscript/scriptobject.h"
#include "entitydef/common.h"
#include "network/address.h"

#ifdef KBE_SERVER
#include "server/components.h"
#endif

namespace KBEngine{

class ScriptDefModule;
class RemoteEntityMethod;
class MethodDescription;

namespace Network
{
class Channel;
class Bundle;
}

class EntityCallAbstract : public script::ScriptObject
{
	/** ���໯ ��һЩpy�������������� */
	INSTANCE_SCRIPT_HREADER(EntityCallAbstract, ScriptObject)
public:
	EntityCallAbstract(PyTypeObject* scriptType, 
		const Network::Address* pAddr, 
		COMPONENT_ID componentID, 
		ENTITY_ID eid, 
		uint16 utype, 
		ENTITYCALL_TYPE type);
	
	virtual ~EntityCallAbstract();

	typedef std::tr1::function<RemoteEntityMethod* (MethodDescription* pMethodDescription, EntityCallAbstract* pEntityCall)> EntityCallCallHookFunc;
	typedef std::tr1::function<Network::Channel* (EntityCallAbstract&)> FindChannelFunc;

	enum ENTITYCALL_CLASS
	{
		ENTITYCALL_CLASS_ENTITY,
		ENTITYCALL_CLASS_ENTITY_COMPONENT,
	};

	/**
		����entityCall��__findChannelFunc������ַ
	*/
	static void setFindChannelFunc(FindChannelFunc func) {
		__findChannelFunc = func;
	};

	/**
		����entityCall��__hookCallFunc������ַ
	*/
	static void setEntityCallCallHookFunc(EntityCallCallHookFunc* pFunc) {
		__hookCallFuncPtr = pFunc;
	};

	static void resetCallHooks() {
		__hookCallFuncPtr = NULL;
		__findChannelFunc = FindChannelFunc();
	}

	virtual ENTITYCALL_CLASS entitycallClass() const {
		return ENTITYCALL_CLASS_ENTITY;
	}

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
	INLINE ENTITYCALL_TYPE type(void) const;

	/** 
		֧��pickler ���� 
	*/
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol);
	
	virtual Network::Channel* getChannel(void);

	virtual bool sendCall(Network::Bundle* pBundle);

	virtual void newCall_(Network::Bundle& bundle);
	virtual void newCall(Network::Bundle& bundle);

	const Network::Address& addr() const{ return addr_; }
	void addr(const Network::Address& saddr){ addr_ = saddr; }

	INLINE bool isClient() const;
	INLINE bool isCell() const;
	INLINE bool isCellReal() const;
	INLINE bool isCellViaBase() const;
	INLINE bool isBase() const;
	INLINE bool isBaseReal() const;
	INLINE bool isBaseViaCell() const;
	
	ScriptDefModule* pScriptDefModule();

protected:
	COMPONENT_ID							componentID_;			// Զ�˻��������ID
	Network::Address						addr_;					// Ƶ����ַ
	ENTITYCALL_TYPE							type_;					// ��entityCall������
	ENTITY_ID								id_;					// entityID
	ENTITY_SCRIPT_UID						utype_;					// entity��utype����entities.xml�еĶ���˳��

	static EntityCallCallHookFunc*			__hookCallFuncPtr;
	static FindChannelFunc					__findChannelFunc;
};

}

#ifdef CODE_INLINE
#include "entitycallabstract.inl"
#endif
#endif // KBE_ENTITYCALL_BASE_H
 