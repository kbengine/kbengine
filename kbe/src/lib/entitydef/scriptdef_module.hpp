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


#ifndef __SCRIPT_DEF_MODULE_H__
#define __SCRIPT_DEF_MODULE_H__
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
#include "cstdkbe/refcountable.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	描述一个脚本def模块
*/
class ScriptDefModule : public RefCountable
{
public:
	typedef std::map<std::string, PropertyDescription*> PROPERTYDESCRIPTION_MAP;
	typedef std::map<std::string, MethodDescription*> METHODDESCRIPTION_MAP;
	typedef std::map<ENTITY_PROPERTY_UID, PropertyDescription*> PROPERTYDESCRIPTION_UIDMAP;
	typedef std::map<ENTITY_METHOD_UID, MethodDescription*> METHODDESCRIPTION_UIDMAP;

	ScriptDefModule(std::string name);
	~ScriptDefModule();	

	void finalise(void);

	ENTITY_SCRIPT_UID getUType(void);
	void setUType(ENTITY_SCRIPT_UID utype);

	PyTypeObject* getScriptType(void);
	INLINE void setScriptType(PyTypeObject* scriptType);

	INLINE DetailLevel& getDetailLevel(void);
	
	PyObject* createObject(void);
	PyObject* getInitDict(void);

	INLINE void setCell(bool have);
	INLINE void setBase(bool have);
	INLINE void setClient(bool have);

	INLINE bool hasCell(void)const;
	INLINE bool hasBase(void)const;
	INLINE bool hasClient(void)const;

	PropertyDescription* findCellPropertyDescription(const char* attrName);
	PropertyDescription* findBasePropertyDescription(const char* attrName);
	PropertyDescription* findClientPropertyDescription(const char* attrName);
	PropertyDescription* findPersistentPropertyDescription(const char* attrName);

	PropertyDescription* findCellPropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findBasePropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findClientPropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findPersistentPropertyDescription(ENTITY_PROPERTY_UID utype);

	PropertyDescription* findPropertyDescription(const char* attrName, COMPONENT_TYPE componentType);
	PropertyDescription* findPropertyDescription(ENTITY_PROPERTY_UID utype, COMPONENT_TYPE componentType);

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
									COMPONENT_TYPE componentType);

	
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

	INLINE const char* getName();

	void autoMatchCompOwn();
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
	
	// 这个脚本所拥有的方法描述
	METHODDESCRIPTION_MAP				methodCellDescr_;						
	METHODDESCRIPTION_MAP				methodBaseDescr_;						
	METHODDESCRIPTION_MAP				methodClientDescr_;						
	
	// 这个脚本所拥有的方法描述uid映射
	METHODDESCRIPTION_UIDMAP			methodCellDescr_uidmap_;				
	METHODDESCRIPTION_UIDMAP			methodBaseDescr_uidmap_;				
	METHODDESCRIPTION_UIDMAP			methodClientDescr_uidmap_;				
	
	// 是否有cell部分等
	bool								hasCell_;								
	bool								hasBase_;								
	bool								hasClient_;							
	
	// entity的详情级别数据
	DetailLevel							detailLevel_;							

	// 这个模块的名称
	std::string							name_;									
};


}

#ifdef CODE_INLINE
#include "scriptdef_module.ipp"
#endif
#endif
