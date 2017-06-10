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
	����һ���ű�defģ��
*/
class ScriptDefModule : public RefCountable
{
public:
	typedef std::map<std::string, PropertyDescription*> PROPERTYDESCRIPTION_MAP;
	typedef std::map<std::string, MethodDescription*> METHODDESCRIPTION_MAP;
	typedef std::map<ENTITY_PROPERTY_UID, PropertyDescription*> PROPERTYDESCRIPTION_UIDMAP;
	typedef std::map<ENTITY_METHOD_UID, MethodDescription*> METHODDESCRIPTION_UIDMAP;
	typedef std::map<ENTITY_DEF_ALIASID, PropertyDescription*> PROPERTYDESCRIPTION_ALIASMAP;
	typedef std::map<ENTITY_DEF_ALIASID, MethodDescription*> METHODDESCRIPTION_ALIASMAP;

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

	PropertyDescription* findCellPropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findBasePropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findClientPropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findPersistentPropertyDescription(ENTITY_PROPERTY_UID utype);
	PropertyDescription* findPropertyDescription(ENTITY_PROPERTY_UID utype, COMPONENT_TYPE componentType);

	PropertyDescription* findAliasPropertyDescription(ENTITY_DEF_ALIASID aliasID);
	MethodDescription* findAliasMethodDescription(ENTITY_DEF_ALIASID aliasID);

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

	bool hasPropertyName(const std::string& name);
	bool hasMethodName(const std::string& name);
	
	INLINE METHODDESCRIPTION_MAP& getBaseExposedMethodDescriptions(void);
	INLINE METHODDESCRIPTION_MAP& getCellExposedMethodDescriptions(void);

	INLINE const char* getName();

	void autoMatchCompOwn();

	INLINE bool isPersistent() const;

	void c_str();

	INLINE bool usePropertyDescrAlias() const;
	INLINE bool useMethodDescrAlias() const;
	
protected:
	// �ű����
	PyTypeObject*						scriptType_;

	// �������  ��Ҫ���ڷ�����Һ�����䴫��ʶ������ű�ģ��
	ENTITY_SCRIPT_UID					uType_;
	
	// ����ű����еĴ洢��db������
	PROPERTYDESCRIPTION_MAP				persistentPropertyDescr_;

	// ����ű�cell������ӵ�е�������������
	PROPERTYDESCRIPTION_MAP				cellPropertyDescr_;

	// cell����Զ������������
	PROPERTYDESCRIPTION_MAP				cellDetailLevelPropertyDescrs_[3];

	// ����ű�base������ӵ�е���������
	PROPERTYDESCRIPTION_MAP				basePropertyDescr_;

	// ����ű�client������ӵ�е���������
	PROPERTYDESCRIPTION_MAP				clientPropertyDescr_;
	
	// ����ű���ӵ�е���������uidӳ��
	PROPERTYDESCRIPTION_UIDMAP			persistentPropertyDescr_uidmap_;
	PROPERTYDESCRIPTION_UIDMAP			cellPropertyDescr_uidmap_;
	PROPERTYDESCRIPTION_UIDMAP			basePropertyDescr_uidmap_;
	PROPERTYDESCRIPTION_UIDMAP			clientPropertyDescr_uidmap_;
	
	// ����ű���ӵ�е���������aliasIDӳ��
	PROPERTYDESCRIPTION_ALIASMAP		propertyDescr_aliasmap_;

	// ����ű���ӵ�еķ�������
	METHODDESCRIPTION_MAP				methodCellDescr_;
	METHODDESCRIPTION_MAP				methodBaseDescr_;
	METHODDESCRIPTION_MAP				methodClientDescr_;
	
	METHODDESCRIPTION_MAP				methodBaseExposedDescr_;
	METHODDESCRIPTION_MAP				methodCellExposedDescr_;

	// ����ű���ӵ�еķ�������uidӳ��
	METHODDESCRIPTION_UIDMAP			methodCellDescr_uidmap_;
	METHODDESCRIPTION_UIDMAP			methodBaseDescr_uidmap_;
	METHODDESCRIPTION_UIDMAP			methodClientDescr_uidmap_;
			
	METHODDESCRIPTION_ALIASMAP			methodDescr_aliasmap_;

	// �Ƿ���cell���ֵ�
	bool								hasCell_;
	bool								hasBase_;
	bool								hasClient_;
	
	// entity�����鼶������
	DetailLevel							detailLevel_;
	VolatileInfo*						pVolatileinfo_;

	// ���ģ�������
	std::string							name_;

	bool								usePropertyDescrAlias_;
	bool								useMethodDescrAlias_;
};


}

#ifdef CODE_INLINE
#include "scriptdef_module.inl"
#endif
#endif // KBE_SCRIPT_DEF_MODULE_H

