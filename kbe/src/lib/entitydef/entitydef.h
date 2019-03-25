// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_ENTITYDEF_H
#define KBE_ENTITYDEF_H

#include "common/common.h"
#include "common/md5.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif

#include "method.h"	
#include "property.h"
#include "entity_call.h"
#include "math/math.h"
#include "pyscript/scriptobject.h"
#include "xml/xml.h"	
#include "common/smartpointer.h"


namespace KBEngine{

class ScriptDefModule;
typedef SmartPointer<ScriptDefModule> ScriptDefModulePtr;

class EntityDef
{
public:
	typedef std::vector<ScriptDefModulePtr> SCRIPT_MODULES;	
	typedef std::map<std::string, ENTITY_SCRIPT_UID> SCRIPT_MODULE_UID_MAP;	

	typedef std::tr1::function<PyObject* (COMPONENT_ID componentID, ENTITY_ID& eid)> GetEntityFunc;
	typedef std::tr1::function<Network::Channel* (EntityCall&)> FindChannelFunc;

	EntityDef();
	~EntityDef();
	
	/** 
		��ʼ��
	*/
	static bool initialize(std::vector<PyTypeObject*>& scriptBaseTypes, 
		COMPONENT_TYPE loadComponentType);

	static bool finalise(bool isReload = false);

	static void reload(bool fullReload);

	/**
		ͨ��entity��ID����Ѱ������ʵ��
	*/
	static PyObject* tryGetEntity(COMPONENT_ID componentID, ENTITY_ID entityID);

	/**
		����entityCall��__getEntityFunc������ַ
	*/
	static void setGetEntityFunc(GetEntityFunc func) {
		__getEntityFunc = func;
	};

	/** 
		�����������
	*/
	static bool loadAllEntityScriptModules(std::string entitiesPath, 
		std::vector<PyTypeObject*>& scriptBaseTypes);

	static bool loadAllComponentScriptModules(std::string entitiesPath,
		std::vector<PyTypeObject*>& scriptBaseTypes);

	static bool loadAllDefDescriptions(const std::string& moduleName, 
		XML* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* pScriptModule);

	static bool loadDefPropertys(const std::string& moduleName, 
		XML* xml, 
		TiXmlNode* defPropertyNode, 
		ScriptDefModule* pScriptModule);

	static bool calcDefPropertyUType(const std::string& moduleName, 
		const std::string& name, int iUtype, ScriptDefModule* pScriptModule, ENTITY_PROPERTY_UID& outUtype);

	static bool loadDefCellMethods(const std::string& moduleName, 
		XML* xml, 
		TiXmlNode* defMethodNode, 
		ScriptDefModule* pScriptModule);

	static bool loadDefBaseMethods(const std::string& moduleName, 
		XML* xml, 
		TiXmlNode* defMethodNode, 
		ScriptDefModule* pScriptModule);

	static bool loadDefClientMethods(const std::string& moduleName, 
		XML* xml, 
		TiXmlNode* defMethodNode, 
		ScriptDefModule* pScriptModule);

	static bool loadInterfaces(const std::string& defFilePath, 
		const std::string& moduleName, 
		XML* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* pScriptModule, bool ignoreComponents = false);

	static bool loadComponents(const std::string& defFilePath,
		const std::string& moduleName,
		XML* defxml,
		TiXmlNode* defNode,
		ScriptDefModule* pScriptModule);

	static PropertyDescription* addComponentProperty(ENTITY_PROPERTY_UID utype,
		const std::string& componentTypeName,
		const std::string& componentName,
		uint32 flags,
		bool isPersistent,
		bool isIdentifier,
		std::string indexType,
		uint32 databaseLength,
		const std::string& defaultStr,
		DETAIL_TYPE detailLevel,
		ScriptDefModule* pScriptModule,
		ScriptDefModule* pCompScriptDefModule);

	static bool loadParentClass(const std::string& defFilePath, 
		const std::string& moduleName, 
		XML* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* pScriptModule);

	static bool loadDefInfo(const std::string& defFilePath, 
		const std::string& moduleName, 
		XML* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* pScriptModule);

	static bool loadDetailLevelInfo(const std::string& defFilePath, 
		const std::string& moduleName, 
		XML* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* pScriptModule);

	static bool loadVolatileInfo(const std::string& defFilePath, 
		const std::string& moduleName, 
		XML* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* pScriptModule);

	static PyObject* loadScriptModule(std::string moduleName);

	/** 
		�Ƿ��������ű�ģ�� 
	*/
	static bool isLoadScriptModule(ScriptDefModule* pScriptModule);

	/** 
		���ݵ�ǰ�����������Ƿ���cell ����base 
	*/
	static void setScriptModuleHasComponentEntity(ScriptDefModule* pScriptModule, bool has);

	/** 
		���ű�ģ���б�����ķ����Ƿ���� 
	*/
	static bool checkDefMethod(ScriptDefModule* pScriptModule, PyObject* moduleObj, 
		const std::string& moduleName);
	
	/** 
		���ű�ģ���б�����������Ƿ�Ϸ� 
	*/
	static bool validDefPropertyName(const std::string& name);

	/** 
		ͨ�������Ѱ�ҵ���Ӧ�Ľű�ģ����� 
	*/
	static ScriptDefModule* findScriptModule(ENTITY_SCRIPT_UID utype, bool notFoundOutErr = true);
	static ScriptDefModule* findScriptModule(const char* scriptName, bool notFoundOutErr = true);
	static ScriptDefModule* findOldScriptModule(const char* scriptName, bool notFoundOutErr = true);

	static bool installScript(PyObject* mod);
	static bool uninstallScript();

	static const SCRIPT_MODULES& getScriptModules() { 
		return EntityDef::__scriptModules; 
	}

	static KBE_MD5& md5(){ return __md5; }

	static bool initializeWatcher();

	static void entitydefAliasID(bool v)
	{ 
		__entitydefAliasID = v; 
	}

	static bool entitydefAliasID()
	{ 
		return __entitydefAliasID; 
	}

	static void entityAliasID(bool v)
	{ 
		__entityAliasID = v; 
	}

	static bool entityAliasID()
	{ 
		return __entityAliasID; 
	}

	static bool scriptModuleAliasID()
	{ 
		return __entitydefAliasID && __scriptModules.size() <= 255; 
	}

	struct Context
	{
		Context()
		{
			currEntityID = 0;
			currClientappID = 0;
			currComponentType = UNKNOWN_COMPONENT_TYPE;
		}

		ENTITY_ID currEntityID;
		COMPONENT_TYPE currComponentType;
		int32 currClientappID;
	};

	static Context& context() {
		return __context;
	}

	static ScriptDefModule* registerNewScriptDefModule(const std::string& moduleName);

	static bool isReload();

private:
	static SCRIPT_MODULES __scriptModules;										// ���е���չ�ű�ģ�鶼�洢������
	static SCRIPT_MODULES __oldScriptModules;									// reloadʱ�ɵ�ģ���ŵ����������ж�

	static SCRIPT_MODULE_UID_MAP __scriptTypeMappingUType;						// �ű����ӳ��utype
	static SCRIPT_MODULE_UID_MAP __oldScriptTypeMappingUType;					// reloadʱ�ɵĽű����ӳ��utype

	static COMPONENT_TYPE __loadComponentType;									// �����ϵ����������������		
	static std::vector<PyTypeObject*> __scriptBaseTypes;
	static std::string __entitiesPath;

	static KBE_MD5 __md5;														// defs-md5

	static bool _isInit;

	static bool __entityAliasID;												// �Ż�EntityID��view��Χ��С��255��EntityID, ���䵽clientʱʹ��1�ֽ�αID 
	static bool __entitydefAliasID;												// �Ż�entity���Ժͷ����㲥ʱռ�õĴ���entity�ͻ������Ի��߿ͻ��˲�����255��ʱ�� ����uid������uid���䵽clientʱʹ��1�ֽڱ���ID
													
	static GetEntityFunc __getEntityFunc;										// ���һ��entity��ʵ��ĺ�����ַ

	// ���õ�ǰ������һЩ������
	static Context __context;
};

}

#ifdef CODE_INLINE
#include "entitydef.inl"
#endif
#endif // KBE_ENTITYDEF_H

