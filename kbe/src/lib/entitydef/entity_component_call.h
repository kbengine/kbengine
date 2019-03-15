// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_ENTITY_COMPONENT_CELL_BASE_CLIENT__CALL_H
#define KBE_ENTITY_COMPONENT_CELL_BASE_CLIENT__CALL_H
	
#include "common/common.h"
#include "pyscript/scriptobject.h"
#include "entitydef/entitycallabstract.h"
	
#ifdef KBE_SERVER
#include "server/components.h"
#endif

	
namespace KBEngine{

namespace Network
{
class Channel;
}

class EntityCall;
class ScriptDefModule;
class RemoteEntityMethod;
class MethodDescription;
class PropertyDescription;

class EntityComponentCall : public EntityCallAbstract
{
	/** ���໯ ��һЩpy�������������� */
	INSTANCE_SCRIPT_HREADER(EntityComponentCall, EntityCallAbstract)
public:
	typedef std::tr1::function<RemoteEntityMethod* (MethodDescription* pMethodDescription, EntityComponentCall* pEntityCall)> EntityCallCallHookFunc;
	typedef std::tr1::function<Network::Channel* (EntityComponentCall&)> FindChannelFunc;

	EntityComponentCall(EntityCall* pEntityCall, PropertyDescription* pComponentPropertyDescription);

	~EntityComponentCall();

	virtual ENTITYCALL_CLASS entitycallClass() const {
		return ENTITYCALL_CLASS_ENTITY_COMPONENT;
	}

	/** 
		�ű������ȡ���Ի��߷��� 
	*/
	PyObject* onScriptGetAttribute(PyObject* attr);						

	/** 
		��ö�������� 
	*/
	PyObject* tp_repr();
	PyObject* tp_str();
	
	void c_str(char* s, size_t size);

	/** 
		�ű�����װʱ������ 
	*/
	static void onInstallScript(PyObject* mod);

	virtual RemoteEntityMethod* createRemoteMethod(MethodDescription* pMethodDescription);

	/**
		unpickle����
	*/
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	ScriptDefModule* pComponentScriptDefModule();

	virtual void newCall(Network::Bundle& bundle);

	static std::vector<EntityComponentCall*> getComponents(const std::string& name, EntityCall* pEntity, ScriptDefModule* pEntityScriptDescrs);

protected:
	EntityCall*								pEntityCall_;
	PropertyDescription*					pComponentPropertyDescription_;
};

}
#endif // KBE_ENTITY_COMPONENT_CELL_BASE_CLIENT__CALL_H
