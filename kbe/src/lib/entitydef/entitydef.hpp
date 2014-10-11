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


#ifndef KBE_ENTITYDEF_HPP
#define KBE_ENTITYDEF_HPP

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/md5.hpp"
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
#include "cstdkbe/smartpointer.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine{

class ScriptDefModule;
typedef SmartPointer<ScriptDefModule> ScriptDefModulePtr;

class EntityDef
{
public:
	typedef std::vector<ScriptDefModulePtr> SCRIPT_MODULES;	
	typedef std::map<std::string, ENTITY_SCRIPT_UID> SCRIPT_MODULE_UID_MAP;	

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
		�����������
	*/
	static bool loadAllScriptModule(std::string entitiesPath, 
		std::vector<PyTypeObject*>& scriptBaseTypes);

	static bool loadAllDefDescription(const std::string& moduleName, 
		XmlPlus* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* scriptModule);

	static bool loadDefPropertys(const std::string& moduleName, 
		XmlPlus* xml, 
		TiXmlNode* defPropertyNode, 
		ScriptDefModule* scriptModule);

	static bool loadDefCellMethods(const std::string& moduleName, 
		XmlPlus* xml, 
		TiXmlNode* defMethodNode, 
		ScriptDefModule* scriptModule);

	static bool loadDefBaseMethods(const std::string& moduleName, 
		XmlPlus* xml, 
		TiXmlNode* defMethodNode, 
		ScriptDefModule* scriptModule);

	static bool loadDefClientMethods(const std::string& moduleName, 
		XmlPlus* xml, 
		TiXmlNode* defMethodNode, 
		ScriptDefModule* scriptModule);

	static bool loadInterfaces(const std::string& defFilePath, 
		const std::string& moduleName, 
		XmlPlus* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* scriptModule);

	static bool loadParentClass(const std::string& defFilePath, 
		const std::string& moduleName, 
		XmlPlus* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* scriptModule);

	static bool loadDefInfo(const std::string& defFilePath, 
		const std::string& moduleName, 
		XmlPlus* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* scriptModule);

	static bool loadDetailLevelInfo(const std::string& defFilePath, 
		const std::string& moduleName, 
		XmlPlus* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* scriptModule);

	static bool loadVolatileInfo(const std::string& defFilePath, 
		const std::string& moduleName, 
		XmlPlus* defxml, 
		TiXmlNode* defNode, 
		ScriptDefModule* scriptModule);

	/** 
		�Ƿ��������ű�ģ�� 
	*/
	static bool isLoadScriptModule(ScriptDefModule* scriptModule);

	/** 
		���ݵ�ǰ�����������Ƿ���cell ����base 
	*/
	static void setScriptModuleHasComponentEntity(ScriptDefModule* scriptModule, bool has);

	/** 
		���ű�ģ���б�����ķ����Ƿ���� 
	*/
	static bool checkDefMethod(ScriptDefModule* scriptModule, PyObject* moduleObj, 
		const std::string& moduleName);
	
	/** 
		ͨ�������Ѱ�ҵ���Ӧ�Ľű�ģ����� 
	*/
	static ScriptDefModule* findScriptModule(ENTITY_SCRIPT_UID utype);
	static ScriptDefModule* findScriptModule(const char* scriptName);
	static ScriptDefModule* findOldScriptModule(const char* scriptName);

	static bool installScript(PyObject* mod);
	static bool uninstallScript();

	static const SCRIPT_MODULES& getScriptModules(){ 
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

	static bool __entityAliasID;												// �Ż�EntityID��aoi��Χ��С��255��EntityID, ���䵽clientʱʹ��1�ֽ�αID 
	static bool __entitydefAliasID;												// �Ż�entity���Ժͷ����㲥ʱռ�õĴ���entity�ͻ������Ի��߿ͻ��˲�����255��ʱ�� ����uid������uid���䵽clientʱʹ��1�ֽڱ���ID
};

}

#ifdef CODE_INLINE
#include "entitydef.ipp"
#endif
#endif // KBE_ENTITYDEF_HPP

