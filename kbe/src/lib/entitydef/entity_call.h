// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_ENTITY_CELL_BASE_CLIENT__ENTITYCALL_H
#define KBE_ENTITY_CELL_BASE_CLIENT__ENTITYCALL_H
	
#include "common/common.h"
#include "entitydef/entitycallabstract.h"
#include "pyscript/scriptobject.h"

	
namespace KBEngine{


class EntityCall : public EntityCallAbstract
{
	/** ���໯ ��һЩpy�������������� */
	INSTANCE_SCRIPT_HREADER(EntityCall, EntityCallAbstract)
public:
	EntityCall(ScriptDefModule* pScriptModule, const Network::Address* pAddr, COMPONENT_ID componentID, 
		ENTITY_ID eid, ENTITYCALL_TYPE type);

	~EntityCall();
	
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
		unpickle���� 
	*/
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	/** 
		�ű�����װʱ������ 
	*/
	static void onInstallScript(PyObject* mod);

	virtual RemoteEntityMethod* createRemoteMethod(MethodDescription* pMethodDescription);

	void reload();

	typedef std::vector<EntityCall*> ENTITYCALLS;
	static ENTITYCALLS entityCalls;
	
	ScriptDefModule* pScriptModule() {
		return pScriptModule_;
	}
	
	virtual void newCall(Network::Bundle& bundle);

	PyObject* pyGetComponent(const std::string& componentName, bool all);
	static PyObject* __py_pyGetComponent(PyObject* self, PyObject* args);

protected:
	std::string								scriptModuleName_;

	// ��entity��ʹ�õĽű�ģ�����
	ScriptDefModule*						pScriptModule_;	

	void _setATIdx(ENTITYCALLS::size_type idx) {
		atIdx_ = idx; 
	}

	ENTITYCALLS::size_type	atIdx_;
};

}
#endif // KBE_ENTITY_CELL_BASE_CLIENT__ENTITYCALL_H
