// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_ENTITY_CELL_BASE_CLIENT__ENTITYCALL_H
#define KBE_ENTITY_CELL_BASE_CLIENT__ENTITYCALL_H
	
#include "common/common.h"
#include "entitydef/entitycallabstract.h"
#include "pyscript/scriptobject.h"

	
namespace KBEngine{


class EntityCall : public EntityCallAbstract
{
	/** 子类化 将一些py操作填充进派生类 */
	INSTANCE_SCRIPT_HREADER(EntityCall, EntityCallAbstract)
public:
	EntityCall(ScriptDefModule* pScriptModule, const Network::Address* pAddr, COMPONENT_ID componentID, 
		ENTITY_ID eid, ENTITYCALL_TYPE type);

	~EntityCall();
	
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
		unpickle方法 
	*/
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	/** 
		脚本被安装时被调用 
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

	// 该entity所使用的脚本模块对象
	ScriptDefModule*						pScriptModule_;	

	void _setATIdx(ENTITYCALLS::size_type idx) {
		atIdx_ = idx; 
	}

	ENTITYCALLS::size_type	atIdx_;
};

}
#endif // KBE_ENTITY_CELL_BASE_CLIENT__ENTITYCALL_H
