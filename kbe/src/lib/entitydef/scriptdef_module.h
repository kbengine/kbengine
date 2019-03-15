// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_SCRIPT_DEF_MODULE_H
#define KBE_SCRIPT_DEF_MODULE_H

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif

#include "method.h"
#include "property.h"
#include "detaillevel.h"
#include "volatileinfo.h"
#include "math/math.h"
#include "pyscript/scriptobject.h"
#include "xml/xml.h"
#include "common/refcountable.h"


namespace KBEngine{

/**
	描述一个脚本def模块
*/
class ScriptDefModule : public RefCountable
{
public:
	typedef std::map<std::string, PropertyDescription*> PROPERTYDESCRIPTION_MAP;
	typedef std::map<std::string, MethodDescription*> METHODDESCRIPTION_MAP;
	typedef std::map<std::string, ScriptDefModule*> COMPONENTDESCRIPTION_MAP;
	typedef std::map<std::string, PropertyDescription*> COMPONENTPROPERTYDESCRIPTION_MAP;

	typedef std::map<ENTITY_PROPERTY_UID, PropertyDescription*> PROPERTYDESCRIPTION_UIDMAP;
	typedef std::map<ENTITY_METHOD_UID, MethodDescription*> METHODDESCRIPTION_UIDMAP;
	typedef std::map<ENTITY_COMPONENT_UID, ScriptDefModule*> COMPONENTDESCRIPTION_UIDMAP;

	typedef std::map<ENTITY_DEF_ALIASID, PropertyDescription*> PROPERTYDESCRIPTION_ALIASMAP;
	typedef std::map<ENTITY_DEF_ALIASID, MethodDescription*> METHODDESCRIPTION_ALIASMAP;
	typedef std::map<ENTITY_COMPONENT_ALIASID, ScriptDefModule*> COMPONENTDESCRIPTION_ALIASMAP;
	
	typedef std::map<ENTITY_COMPONENT_UID, ENTITY_COMPONENT_ALIASID> COMPONENTDESCRIPTION_TYPE2ALIASMAP;

	typedef std::vector<ScriptDefModule*> COMPONENTDESCRIPTIONS;

	ScriptDefModule(std::string name, ENTITY_SCRIPT_UID utype);
	~ScriptDefModule();

	void finalise(void);
	void onLoaded(void);

	void addSmartUTypeToStream(MemoryStream* pStream);
	void addSmartUTypeToBundle(Network::Bundle* pBundle);

	INLINE ENTITY_SCRIPT_UID getUType(void);
	INLINE ENTITY_DEF_ALIASID getAliasID(void);
	void setUType(ENTITY_SCRIPT_UID utype);

	INLINE PyTypeObject* getScriptType(void);
	INLINE void setScriptType(PyTypeObject* scriptType);

	INLINE DetailLevel& getDetailLevel(void);
	INLINE VolatileInfo* getPVolatileInfo(void);

	PyObject* createObject(void);
	PyObject* getInitDict(void);

	INLINE void setCell(bool have);
	INLINE void setBase(bool have);
	INLINE void setClient(bool have);

	INLINE bool hasCell(void) const;
	INLINE bool hasBase(void) const;
	INLINE bool hasClient(void) const;

	PropertyDescription* findCellPropertyDescription(const char* attrName);
	PropertyDescription* findBasePropertyDescription(const char* attrName);
	PropertyDescription* findClientPropertyDescription(const char* attrName);
	PropertyDescription* findPersistentPropertyDescription(const char* attrName);
	PropertyDescription* findPropertyDescription(const char* attrName, COMPONENT_TYPE componentType);
	PropertyDescription* findComponentPropertyDescription(const char* attrName);

	PropertyDescription* findCellPropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findBasePropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findClientPropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findPersistentPropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findPropertyDescription(ENTITY_PROPERTY_UID utype, COMPONENT_TYPE componentType);

	PropertyDescription* findAliasPropertyDescription(ENTITY_DEF_ALIASID aliasID);
	MethodDescription* findAliasMethodDescription(ENTITY_DEF_ALIASID aliasID);

	size_t numPropertys() {
		return getCellPropertyDescriptions().size() + getBasePropertyDescriptions().size();
	}

	INLINE PROPERTYDESCRIPTION_MAP& getCellPropertyDescriptions();
	INLINE PROPERTYDESCRIPTION_MAP& getCellPropertyDescriptionsByDetailLevel(int8 detailLevel);
	INLINE PROPERTYDESCRIPTION_MAP& getBasePropertyDescriptions();
	INLINE PROPERTYDESCRIPTION_MAP& getClientPropertyDescriptions();
	INLINE PROPERTYDESCRIPTION_MAP& getPersistentPropertyDescriptions();

	INLINE PROPERTYDESCRIPTION_UIDMAP& getCellPropertyDescriptions_uidmap();
	INLINE PROPERTYDESCRIPTION_UIDMAP& getBasePropertyDescriptions_uidmap();
	INLINE PROPERTYDESCRIPTION_UIDMAP& getClientPropertyDescriptions_uidmap();
	INLINE PROPERTYDESCRIPTION_UIDMAP& getPersistentPropertyDescriptions_uidmap();

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& getPropertyDescrs();

	bool addPropertyDescription(const char* attrName, 
									PropertyDescription* propertyDescription, 
									COMPONENT_TYPE componentType, bool ignoreConflict = false);

	
	MethodDescription* findCellMethodDescription(const char* attrName);
	MethodDescription* findCellMethodDescription(ENTITY_METHOD_UID utype);
	bool addCellMethodDescription(const char* attrName, MethodDescription* methodDescription);
	INLINE METHODDESCRIPTION_MAP& getCellMethodDescriptions(void);
	
	MethodDescription* findBaseMethodDescription(const char* attrName);
	MethodDescription* findBaseMethodDescription(ENTITY_METHOD_UID utype);
	bool addBaseMethodDescription(const char* attrName, MethodDescription* methodDescription);
	INLINE METHODDESCRIPTION_MAP& getBaseMethodDescriptions(void);
	
	MethodDescription* findClientMethodDescription(const char* attrName);
	MethodDescription* findClientMethodDescription(ENTITY_METHOD_UID utype);
	bool addClientMethodDescription(const char* attrName, MethodDescription* methodDescription);
	INLINE METHODDESCRIPTION_MAP& getClientMethodDescriptions(void);

	MethodDescription* findMethodDescription(const char* attrName, COMPONENT_TYPE componentType);
	MethodDescription* findMethodDescription(ENTITY_METHOD_UID utype, COMPONENT_TYPE componentType);

	bool hasPropertyName(const std::string& name);
	bool hasMethodName(const std::string& name);
	bool hasComponentName(const std::string& name);
	bool hasName(const std::string& name);

	INLINE METHODDESCRIPTION_MAP& getBaseExposedMethodDescriptions(void);
	INLINE METHODDESCRIPTION_MAP& getCellExposedMethodDescriptions(void);

	INLINE const char* getName();

	void autoMatchCompOwn();

	INLINE bool isPersistent() const;
	INLINE void isPersistent(bool v);

	void c_str();

	INLINE bool usePropertyDescrAlias() const;
	INLINE bool useMethodDescrAlias() const;
	
	bool addComponentDescription(const char* compName,
		ScriptDefModule* compDescription);

	ScriptDefModule* findComponentDescription(const char* compName);
	ScriptDefModule* findComponentDescription(ENTITY_PROPERTY_UID utype);
	ScriptDefModule* findComponentDescription(ENTITY_COMPONENT_ALIASID aliasID);

	ScriptDefModule::COMPONENTDESCRIPTION_MAP& getComponentDescrs() {
		return componentDescr_;
	}

	bool isComponentModule() const {
		return isComponentModule_;
	}

	void isComponentModule(bool v) {
		isComponentModule_ = v;
	}

protected:
	// 脚本类别
	PyTypeObject*						scriptType_;

	// 数字类别  主要用于方便查找和网络间传输识别这个脚本模块
	ENTITY_SCRIPT_UID					uType_;
	
	// 这个脚本所有的存储到db的属性
	PROPERTYDESCRIPTION_MAP				persistentPropertyDescr_;

	// 这个脚本cell部分所拥有的所有属性描述
	PROPERTYDESCRIPTION_MAP				cellPropertyDescr_;

	// cell近中远级别属性描述
	PROPERTYDESCRIPTION_MAP				cellDetailLevelPropertyDescrs_[3];

	// 这个脚本base部分所拥有的属性描述
	PROPERTYDESCRIPTION_MAP				basePropertyDescr_;

	// 这个脚本client部分所拥有的属性描述
	PROPERTYDESCRIPTION_MAP				clientPropertyDescr_;
	
	// 这个脚本所拥有的属性描述uid映射
	PROPERTYDESCRIPTION_UIDMAP			persistentPropertyDescr_uidmap_;
	PROPERTYDESCRIPTION_UIDMAP			cellPropertyDescr_uidmap_;
	PROPERTYDESCRIPTION_UIDMAP			basePropertyDescr_uidmap_;
	PROPERTYDESCRIPTION_UIDMAP			clientPropertyDescr_uidmap_;
	
	// 这个脚本所拥有的属性描述aliasID映射
	PROPERTYDESCRIPTION_ALIASMAP		propertyDescr_aliasmap_;

	// 这个脚本所拥有的方法描述
	METHODDESCRIPTION_MAP				methodCellDescr_;
	METHODDESCRIPTION_MAP				methodBaseDescr_;
	METHODDESCRIPTION_MAP				methodClientDescr_;
	
	METHODDESCRIPTION_MAP				methodBaseExposedDescr_;
	METHODDESCRIPTION_MAP				methodCellExposedDescr_;

	// 这个脚本所拥有的方法描述uid映射
	METHODDESCRIPTION_UIDMAP			methodCellDescr_uidmap_;
	METHODDESCRIPTION_UIDMAP			methodBaseDescr_uidmap_;
	METHODDESCRIPTION_UIDMAP			methodClientDescr_uidmap_;
			
	METHODDESCRIPTION_ALIASMAP			methodDescr_aliasmap_;

	// 是否有cell部分等
	bool								hasCell_;
	bool								hasBase_;
	bool								hasClient_;
	
	// entity的详情级别数据
	DetailLevel							detailLevel_;
	VolatileInfo*						pVolatileinfo_;

	// 这个模块的名称
	std::string							name_;

	bool								usePropertyDescrAlias_;
	bool								useMethodDescrAlias_;
	bool								useComponentDescrAlias_;

	COMPONENTDESCRIPTION_UIDMAP			componentDescr_uidmap_;
	COMPONENTDESCRIPTIONS				componentDescrVec_;
	COMPONENTDESCRIPTION_MAP			componentDescr_;
	COMPONENTPROPERTYDESCRIPTION_MAP	componentPropertyDescr_;

	bool								persistent_;

	bool								isComponentModule_;
};


}

#ifdef CODE_INLINE
#include "scriptdef_module.inl"
#endif
#endif // KBE_SCRIPT_DEF_MODULE_H

