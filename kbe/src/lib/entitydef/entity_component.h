// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_ENTITY_COMPONENT_H
#define KBE_ENTITY_COMPONENT_H
	
#include "common/common.h"
#include "common/timer.h"
#include "pyscript/scriptobject.h"
#include "entitydef/common.h"
#include "entitydef/scriptdef_module.h"

namespace KBEngine {

// 调用所有组件的方法
#define CALL_ENTITY_AND_COMPONENTS_METHOD(ENTITYOBJ, CALLCODE)													\
{																												\
	{																											\
		bool GETERR = false;																					\
		Py_INCREF(ENTITYOBJ);																					\
		PyObject* pyTempObj = ENTITYOBJ;																		\
		CALLCODE;																								\
		CALL_ENTITY_COMPONENTS_METHOD(ENTITYOBJ, CALLCODE);														\
		Py_DECREF(ENTITYOBJ);																					\
	}																											\
}																												\


#define CALL_COMPONENTS_AND_ENTITY_METHOD(ENTITYOBJ, CALLCODE)													\
{																												\
	{																											\
		Py_INCREF(ENTITYOBJ);																					\
		PyObject* pyTempObj = ENTITYOBJ;																		\
		CALL_ENTITY_COMPONENTS_METHOD(ENTITYOBJ, CALLCODE);														\
		bool GETERR = false;																					\
		CALLCODE;																								\
		Py_DECREF(ENTITYOBJ);																					\
	}																											\
}																												\


#define CALL_ENTITY_COMPONENTS_METHOD(ENTITYOBJ, CALLCODE)														\
	{																											\
		bool GETERR = false;																					\
		ScriptDefModule::COMPONENTDESCRIPTION_MAP& componentDescrs = pScriptModule_->getComponentDescrs();		\
		ScriptDefModule::COMPONENTDESCRIPTION_MAP::iterator comps_iter = componentDescrs.begin();				\
		for (; comps_iter != componentDescrs.end(); ++comps_iter)												\
		{																										\
			if(g_componentType == BASEAPP_TYPE)																	\
			{																									\
				if (!comps_iter->second->hasBase())																\
					continue;																					\
			}																									\
			else if (g_componentType == CELLAPP_TYPE)															\
			{																									\
				if (!comps_iter->second->hasCell())																\
					continue;																					\
			}																									\
			else																								\
			{																									\
				if (!comps_iter->second->hasClient())															\
					continue;																					\
			}																									\
																												\
			PyObject* pyTempObj = PyObject_GetAttrString(ENTITYOBJ, comps_iter->first.c_str());					\
			if (pyTempObj)																						\
			{																									\
				CALLCODE;																						\
				Py_DECREF(pyTempObj);																			\
			}																									\
			else																								\
			{																									\
				SCRIPT_ERROR_CHECK();																			\
			}																									\
		}																										\
	}																											\


class EntityComponent : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	BASE_SCRIPT_HREADER(EntityComponent, ScriptObject)
public:
	EntityComponent(ENTITY_ID ownerID, ScriptDefModule* pComponentDescrs, COMPONENT_TYPE assignmentToComponentType/*属性所属实体的哪一部分，cell或者base?*/);
	~EntityComponent();

	/** 
		获取entityID 
	*/
	ENTITY_ID ownerID() const;

	DECLARE_PY_GET_MOTHOD(pyGetOwnerID);

	PyObject* owner(bool attempt = false);
	void updateOwner(ENTITY_ID id, PyObject* pOwner);

	DECLARE_PY_GET_MOTHOD(pyIsDestroyed);

	void destroyed() {
		ownerID_ = 0;
	}

	bool isDestroyed() const {
		return ownerID() == 0;
	}

	DECLARE_PY_GET_MOTHOD(pyGetOwner);

	DECLARE_PY_GET_MOTHOD(pyGetClassName);

	DECLARE_PY_MOTHOD_ARG3(pyAddTimer, float, float, int32);
	DECLARE_PY_MOTHOD_ARG1(pyDelTimer, PyObject_ptr);

	/** 
		获得描述 
	*/
	INLINE ScriptDefModule* pComponentDescrs(void) const;

	/**
		脚本被安装时被调用
	*/
	static void onInstallScript(PyObject* mod);

	/** 
		支持pickler 方法 
	*/
	static PyObject* __py_reduce_ex__(PyObject* self, PyObject* protocol);
	
	/**
		unpickle方法
	*/
	static PyObject* __unpickle__(PyObject* self, PyObject* args);

	/**
		脚本请求获取属性或者方法
	*/
	PyObject* onScriptGetAttribute(PyObject* attr);
	int onScriptSetAttribute(PyObject* attr, PyObject* value);
	int onScriptDelAttribute(PyObject* attr);

	/**
		这些初始化接口由entity在相对应的接口中初始化
	*/
	void initializeScript();

	void onAttached();
	void onDetached();

	void initProperty(bool isReload = false);

	/**
		获得对象的描述
	*/
	PyObject* tp_repr();
	PyObject* tp_str();

	void c_str(char* s, size_t size);

	void reload();

	typedef std::vector<EntityComponent*> ENTITY_COMPONENTS;
	static ENTITY_COMPONENTS entity_components;

	bool isSameType(PyObject* pyValue);
	void addToStream(MemoryStream* mstream, PyObject* pyValue);
	PyObject* createFromStream(MemoryStream* mstream);

	bool isSamePersistentType(PyObject* pyValue);
	void addPersistentToStream(MemoryStream* mstream, PyObject* pyValue);
	void addPersistentToStreamTemplates(ScriptDefModule* pScriptModule, MemoryStream* mstream);
	PyObject* createFromPersistentStream(ScriptDefModule* pScriptModule, MemoryStream* mstream);

	PropertyDescription* getProperty(ENTITY_PROPERTY_UID child_uid);
	
	void componentType(COMPONENT_TYPE ctype) {
		componentType_ = ctype;
	}

	COMPONENT_TYPE componentType() const {
		return componentType_;
	}

	const ScriptDefModule::PROPERTYDESCRIPTION_MAP* pChildPropertyDescrs();

	ScriptDefModule* pComponentScriptDefModuleDescrs() {
		return pComponentDescrs_;
	}

	typedef std::tr1::function<void (EntityComponent*, const PropertyDescription*, PyObject*)> OnDataChangedEvent;

	static void onEntityDestroy(PyObject* pEntity, ScriptDefModule* pEntityScriptDescrs, bool callScript, bool beforeDestroy);
	void onOwnerDestroyBegin(PyObject* pEntity, ScriptDefModule* pEntityScriptDescrs, bool callScript);
	void onOwnerDestroyEnd(PyObject* pEntity, ScriptDefModule* pEntityScriptDescrs, bool callScript);

	PropertyDescription* pPropertyDescription() const {
		return pPropertyDescription_;
	}

	void pPropertyDescription(PropertyDescription* v) {
		pPropertyDescription_ = v;
	}

	PyObject* createCellData();

	void createFromDict(PyObject* pyDict, bool persistentData);
	void updateFromDict(PyObject* pOwner, PyObject* pyDict);

	static void convertDictDataToEntityComponent(ENTITY_ID entityID, PyObject* pEntity, ScriptDefModule* pEntityScriptDescrs, PyObject* cellData, bool persistentData);
	static std::vector<EntityComponent*> getComponents(const std::string& name, PyObject* pEntity, ScriptDefModule* pEntityScriptDescrs);

	/**
		脚本请求获取client地址
	*/
	DECLARE_PY_GET_MOTHOD(pyName);

	/**
		脚本获取entityCall
	*/
	DECLARE_PY_GET_MOTHOD(pyGetCellEntityCall);

	/**
		脚本获取entityCall
	*/
	DECLARE_PY_GET_MOTHOD(pyGetBaseEntityCall);

	/**
		脚本获取entityCall
	*/
	DECLARE_PY_GET_MOTHOD(pyGetClientEntityCall);

	/**
		脚本获取entityCall
	*/
	DECLARE_PY_GET_MOTHOD(pyGetAllClients);

	/**
		脚本获取entityCall
	*/
	DECLARE_PY_GET_MOTHOD(pyGetOtherClients);

	/**
		调用客户端实体的方法
	*/
	DECLARE_PY_MOTHOD_ARG1(pyClientEntity, ENTITY_ID);

protected:
	void addToServerStream(MemoryStream* mstream, PyObject* pyValue);
	void addToClientStream(MemoryStream* mstream, PyObject* pyValue);

protected:
	COMPONENT_TYPE							componentType_;
	ENTITY_ID								ownerID_;								// entityID
	PyObject*								owner_;
	ScriptDefModule*						pComponentDescrs_;						// 组件的描述

	void _setATIdx(ENTITY_COMPONENTS::size_type idx) {
		atIdx_ = idx;
	}

	ENTITY_COMPONENTS::size_type			atIdx_;

	OnDataChangedEvent						onDataChangedEvent_;

	PropertyDescription*					pPropertyDescription_;					// 承载这个组件自身的实体属性描述

private:
	int32									clientappID_;
};

}

#ifdef CODE_INLINE
#include "entity_component.inl"
#endif
#endif // KBE_ENTITY_COMPONENT_H
