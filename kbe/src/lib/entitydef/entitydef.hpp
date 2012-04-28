/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __ENTITYDEF_H__
#define __ENTITYDEF_H__
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif
// common include
#include "method.hpp"	
#include "property.hpp"
#include "math/math.hpp"
#include "pyscript/scriptobject.hpp"
#include "xmlplus/xmlplus.hpp"	
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>	
#include <map>	
#include <vector>
#include <algorithm>
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
#define LIB_DLLAPI  __declspec(dllexport)

#ifdef __cplusplus  
extern "C" {  
#endif  

#ifdef __cplusplus  
}
#endif 

namespace KBEngine{

class ScriptModule
{
public:
	typedef std::map<std::string, PropertyDescription*> PROPERTYDESCRIPTION_MAP;
	typedef std::map<std::string, MethodDescription*> METHODDESCRIPTION_MAP;
	typedef std::map<uint32, PropertyDescription*> PROPERTYDESCRIPTION_UIDMAP;
	typedef std::map<uint32, MethodDescription*> METHODDESCRIPTION_UIDMAP;
public:	
	ScriptModule();
	~ScriptModule();	

	uint16 getUType(void);
	void setUType(uint16 utype){ uType_ = utype; }
	PyTypeObject* getScriptType(void);
	void setScriptType(PyTypeObject* scriptType){ scriptType_ = scriptType; }

	DetailLevel& getDetailLevel(void){ return detailLevel_; }
	
	PyObject* createObject(void);
	PyObject* getInitDict(void);

	void setCell(bool have){ hasCell_ = have; }
	void setBase(bool have){ hasBase_ = have; }
	void setClient(bool have){ hasClient_ = have; }

	bool hasCell(void)const{ return hasCell_; }
	bool hasBase(void)const{ return hasBase_; }
	bool hasClient(void)const{ return hasClient_; }

	PropertyDescription* findCellPropertyDescription(const char* attrName);
	PropertyDescription* findBasePropertyDescription(const char* attrName);
	PropertyDescription* findClientPropertyDescription(const char* attrName);
	PropertyDescription* findCellPropertyDescription(const uint32& utype);
	PropertyDescription* findBasePropertyDescription(const uint32& utype);
	PropertyDescription* findClientPropertyDescription(const uint32& utype);

	PROPERTYDESCRIPTION_MAP& getCellPropertyDescriptions(){ return cellPropertyDescr_; }
	PROPERTYDESCRIPTION_MAP& getCellPropertyDescriptionsByDetailLevel(const int8& detailLevel){ return cellDetailLevelPropertyDescrs_[detailLevel]; }
	PROPERTYDESCRIPTION_MAP& getBasePropertyDescriptions(){ return basePropertyDescr_; }
	PROPERTYDESCRIPTION_MAP& getClientPropertyDescriptions(){ return clientPropertyDescr_; }
	PROPERTYDESCRIPTION_UIDMAP& getCellPropertyDescriptions_uidmap(){ return cellPropertyDescr_uidmap_; }
	PROPERTYDESCRIPTION_UIDMAP& getBasePropertyDescriptions_uidmap(){ return basePropertyDescr_uidmap_; }
	PROPERTYDESCRIPTION_UIDMAP& getClientPropertyDescriptions_uidmap(){ return clientPropertyDescr_uidmap_; }

	bool addPropertyDescription(const char* attrName, PropertyDescription* propertyDescription, COMPONENT_TYPE propertyType);

	
	MethodDescription* findCellMethodDescription(const char* attrName);
	MethodDescription* findCellMethodDescription(uint32 utype);
	bool addCellMethodDescription(const char* attrName, MethodDescription* methodDescription);
	METHODDESCRIPTION_MAP& getCellMethodDescriptions(void){ return methodCellDescr_; }
	
	MethodDescription* findBaseMethodDescription(const char* attrName);
	MethodDescription* findBaseMethodDescription(uint32 utype);
	bool addBaseMethodDescription(const char* attrName, MethodDescription* methodDescription);
	METHODDESCRIPTION_MAP& getBaseMethodDescriptions(void){ return methodBaseDescr_; }
	
	MethodDescription* findClientMethodDescription(const char* attrName);
	MethodDescription* findClientMethodDescription(uint32 utype);
	bool addClientMethodDescription(const char* attrName, MethodDescription* methodDescription);
	METHODDESCRIPTION_MAP& getClientMethodDescriptions(void){ return methodClientDescr_; }		
protected:
	PyTypeObject*						scriptType_;							// 脚本类别
	uint16								uType_;									// 数字类别  主要用于方便查找和网络间传输识别这个脚本模块
	
	PROPERTYDESCRIPTION_MAP				cellPropertyDescr_;						// 这个脚本cell部分所拥有的所有属性描述
	PROPERTYDESCRIPTION_MAP				cellDetailLevelPropertyDescrs_[3];		// cell近中远级别属性描述
	PROPERTYDESCRIPTION_MAP				basePropertyDescr_;						// 这个脚本base部分所拥有的属性描述
	PROPERTYDESCRIPTION_MAP				clientPropertyDescr_;					// 这个脚本client部分所拥有的属性描述
	
	PROPERTYDESCRIPTION_UIDMAP			cellPropertyDescr_uidmap_;				// 这个脚本所拥有的属性描述uid映射
	PROPERTYDESCRIPTION_UIDMAP			basePropertyDescr_uidmap_;				// 这个脚本所拥有的属性描述uid映射
	PROPERTYDESCRIPTION_UIDMAP			clientPropertyDescr_uidmap_;				// 这个脚本所拥有的属性描述uid映射
	
	METHODDESCRIPTION_MAP				methodCellDescr_;						// 这个脚本所拥有的方法描述
	METHODDESCRIPTION_MAP				methodBaseDescr_;						// 这个脚本所拥有的方法描述
	METHODDESCRIPTION_MAP				methodClientDescr_;						// 这个脚本所拥有的方法描述
	
	METHODDESCRIPTION_UIDMAP			methodCellDescr_uidmap_;				// 这个脚本所拥有的方法描述uid映射
	METHODDESCRIPTION_UIDMAP			methodBaseDescr_uidmap_;				// 这个脚本所拥有的方法描述uid映射
	METHODDESCRIPTION_UIDMAP			methodClientDescr_uidmap_;				// 这个脚本所拥有的方法描述uid映射
	
	bool								hasCell_;								// 是否有cell部分
	bool								hasBase_;								// 是否有base部分
	bool								hasClient_;								// 是否有client部分
	
	DetailLevel							detailLevel_;							// entity的详情级别数据
};

class EntityDef
{
private:
	static std::vector<ScriptModule *> __scriptModules;							// 所有的扩展脚本模块都存储在这里
	static std::map<std::string, uint16> __scriptTypeMappingUType;				// 脚本类别映射utype
	static COMPONENT_TYPE __loadComponentType;									// 所需关系的组件类别的相关数据							
public:
	EntityDef();
	~EntityDef();
	
	static bool initialize(const std::string entitiesPath, std::vector<PyTypeObject*>& scriptBaseTypes, COMPONENT_TYPE loadComponentType);
	static bool finish(void);

	static bool loadAllScriptModule(std::string entitiesPath, std::vector<PyTypeObject*>& scriptBaseTypes);
	static bool loadAllDefDescription(std::string& moduleName, XmlPlus* defxml, TiXmlNode* defNode, ScriptModule* scriptModule);
	static bool loadDefPropertys(XmlPlus* xml, TiXmlNode* defPropertyNode, ScriptModule* scriptModule);
	static bool loadDefCellMethods(XmlPlus* xml, TiXmlNode* defMethodNode, ScriptModule* scriptModule);
	static bool loadDefBaseMethods(XmlPlus* xml, TiXmlNode* defMethodNode, ScriptModule* scriptModule);
	static bool loadDefClientMethods(XmlPlus* xml, TiXmlNode* defMethodNode, ScriptModule* scriptModule);
	static bool loadInterfaces(std::string& defFilePath, std::string& moduleName, XmlPlus* defxml, TiXmlNode* defNode, ScriptModule* scriptModule);
	static bool loadParentClass(std::string& defFilePath, std::string& moduleName, XmlPlus* defxml, TiXmlNode* defNode, ScriptModule* scriptModule);
	static bool loadDefInfo(std::string& defFilePath, std::string& moduleName, XmlPlus* defxml, TiXmlNode* defNode, ScriptModule* scriptModule);
	static bool loadDetailLevelInfo(std::string& defFilePath, std::string& moduleName, XmlPlus* defxml, TiXmlNode* defNode, ScriptModule* scriptModule);
	
	/** 是否加载这个脚本模块 */
	static bool isLoadScriptModule(ScriptModule* scriptModule);
	/** 检查脚本模块中被定义的方法是否存在 */
	static bool checkDefMethod(ScriptModule* scriptModule, PyObject* moduleObj, std::string& moduleName);
	
	/** 通过标记来寻找到对应的脚本模块对象 */
	static ScriptModule* findScriptModule(uint16 utype);
	static ScriptModule* findScriptModule(const char* scriptName);

	static bool installScript(PyObject* mod);
	static bool uninstallScript();
};

}
#endif
