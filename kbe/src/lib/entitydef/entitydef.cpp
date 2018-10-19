// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "entitydef.h"
#include "scriptdef_module.h"
#include "datatypes.h"
#include "common.h"
#include "entity_component.h"
#include "pyscript/py_memorystream.h"
#include "resmgr/resmgr.h"
#include "common/smartpointer.h"
#include "entitydef/volatileinfo.h"
#include "entitydef/entity_call.h"
#include "entitydef/entity_component_call.h"

#ifndef CODE_INLINE
#include "entitydef.inl"
#endif

namespace KBEngine{
std::vector<ScriptDefModulePtr>	EntityDef::__scriptModules;
std::vector<ScriptDefModulePtr>	EntityDef::__oldScriptModules;

std::map<std::string, ENTITY_SCRIPT_UID> EntityDef::__scriptTypeMappingUType;
std::map<std::string, ENTITY_SCRIPT_UID> EntityDef::__oldScriptTypeMappingUType;

COMPONENT_TYPE EntityDef::__loadComponentType;
std::vector<PyTypeObject*> EntityDef::__scriptBaseTypes;
std::string EntityDef::__entitiesPath;

KBE_MD5 EntityDef::__md5;
bool EntityDef::_isInit = false;
bool g_isReload = false;

bool EntityDef::__entityAliasID = false;
bool EntityDef::__entitydefAliasID = false;

EntityDef::Context EntityDef::__context;

// 方法产生时自动产生utype用的
ENTITY_METHOD_UID g_methodUtypeAuto = 1;
std::vector<ENTITY_METHOD_UID> g_methodCusUtypes;																									

ENTITY_PROPERTY_UID g_propertyUtypeAuto = 1;
std::vector<ENTITY_PROPERTY_UID> g_propertyUtypes;

// 产生新的脚本模块时自动产生utype
ENTITY_SCRIPT_UID g_scriptUtype = 1;

// 获得某个entity的函数地址
EntityDef::GetEntityFunc EntityDef::__getEntityFunc;

static std::map<std::string, std::vector<PropertyDescription*> > g_logComponentPropertys;

//-------------------------------------------------------------------------------------
EntityDef::EntityDef()
{
}

//-------------------------------------------------------------------------------------
EntityDef::~EntityDef()
{
	EntityDef::finalise();
}

//-------------------------------------------------------------------------------------
bool EntityDef::finalise(bool isReload)
{
	PropertyDescription::resetDescriptionCount();
	MethodDescription::resetDescriptionCount();

	EntityDef::__md5.clear();
	g_methodUtypeAuto = 1;
	EntityDef::_isInit = false;

	g_propertyUtypeAuto = 1;
	g_propertyUtypes.clear();

	if(!isReload)
	{
		std::vector<ScriptDefModulePtr>::iterator iter = EntityDef::__scriptModules.begin();
		for(; iter != EntityDef::__scriptModules.end(); ++iter)
		{
			(*iter)->finalise();
		}

		iter = EntityDef::__oldScriptModules.begin();
		for(; iter != EntityDef::__oldScriptModules.end(); ++iter)
		{
			(*iter)->finalise();
		}

		EntityDef::__oldScriptModules.clear();
		EntityDef::__oldScriptTypeMappingUType.clear();
	}

	g_scriptUtype = 1;
	EntityDef::__scriptModules.clear();
	EntityDef::__scriptTypeMappingUType.clear();
	g_methodCusUtypes.clear();
	DataType::finalise();
	DataTypes::finalise();
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* EntityDef::tryGetEntity(COMPONENT_ID componentID, ENTITY_ID entityID)
{
	return __getEntityFunc(componentID, entityID);
}

//-------------------------------------------------------------------------------------
void EntityDef::reload(bool fullReload)
{
	g_isReload = true;
	if(fullReload)
	{
		EntityDef::__oldScriptModules.clear();
		EntityDef::__oldScriptTypeMappingUType.clear();

		std::vector<ScriptDefModulePtr>::iterator iter = EntityDef::__scriptModules.begin();
		for(; iter != EntityDef::__scriptModules.end(); ++iter)
		{
			__oldScriptModules.push_back((*iter));
			__oldScriptTypeMappingUType[(*iter)->getName()] = (*iter)->getUType();
		}

		bool ret = finalise(true);
		KBE_ASSERT(ret && "EntityDef::reload: finalise error!");

		ret = initialize(EntityDef::__scriptBaseTypes, EntityDef::__loadComponentType);
		KBE_ASSERT(ret && "EntityDef::reload: initialize error!");
	}
	else
	{
		loadAllEntityScriptModules(EntityDef::__entitiesPath, EntityDef::__scriptBaseTypes);
	}

	EntityDef::_isInit = true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::initialize(std::vector<PyTypeObject*>& scriptBaseTypes, 
						   COMPONENT_TYPE loadComponentType)
{
	__loadComponentType = loadComponentType;
	__scriptBaseTypes = scriptBaseTypes;

	__entitiesPath = Resmgr::getSingleton().getPyUserScriptsPath();

	g_entityFlagMapping["CELL"]									= ED_FLAG_CELL_PUBLIC;
	g_entityFlagMapping["CELL_AND_CLIENT"]						= ED_FLAG_CELL_PUBLIC_AND_OWN;
	g_entityFlagMapping["CELL_AND_CLIENTS"]						= ED_FLAG_ALL_CLIENTS;
	g_entityFlagMapping["CELL_AND_OTHER_CLIENTS"]				= ED_FLAG_OTHER_CLIENTS;
	g_entityFlagMapping["BASE_AND_CLIENT"]						= ED_FLAG_BASE_AND_CLIENT;
	g_entityFlagMapping["BASE"]									= ED_FLAG_BASE;

	g_entityFlagMapping["CELL_PUBLIC"]							= ED_FLAG_CELL_PUBLIC;
	g_entityFlagMapping["CELL_PRIVATE"]							= ED_FLAG_CELL_PRIVATE;
	g_entityFlagMapping["ALL_CLIENTS"]							= ED_FLAG_ALL_CLIENTS;
	g_entityFlagMapping["CELL_PUBLIC_AND_OWN"]					= ED_FLAG_CELL_PUBLIC_AND_OWN;

	g_entityFlagMapping["OTHER_CLIENTS"]						= ED_FLAG_OTHER_CLIENTS;
	g_entityFlagMapping["OWN_CLIENT"]							= ED_FLAG_OWN_CLIENT;

	std::string entitiesFile = __entitiesPath + "entities.xml";
	std::string defFilePath = __entitiesPath + "entity_defs/";
	
	// 初始化数据类别
	// assets/scripts/entity_defs/types.xml
	if(!DataTypes::initialize(defFilePath + "types.xml"))
		return false;

	// 打开这个entities.xml文件
	SmartPointer<XML> xml(new XML());
	if(!xml->openSection(entitiesFile.c_str()))
		return false;
	
	// 获得entities.xml根节点, 如果没有定义一个entity那么直接返回true
	TiXmlNode* node = xml->getRootNode();
	if(node == NULL)
		return true;

	// 开始遍历所有的entity节点
	XML_FOR_BEGIN(node)
	{
		std::string moduleName = xml.get()->getKey(node);
		__scriptTypeMappingUType[moduleName] = g_scriptUtype;
		ScriptDefModule* pScriptModule = new ScriptDefModule(moduleName, g_scriptUtype++);
		EntityDef::__scriptModules.push_back(pScriptModule);

		std::string deffile = defFilePath + moduleName + ".def";
		SmartPointer<XML> defxml(new XML());

		if(!defxml->openSection(deffile.c_str()))
			return false;

		TiXmlNode* defNode = defxml->getRootNode();
		if(defNode == NULL)
		{
			// root节点下没有子节点了
			continue;
		}

		// 加载def文件中的定义
		if(!loadDefInfo(defFilePath, moduleName, defxml.get(), defNode, pScriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: failed to load entity({}) module!\n",
				moduleName.c_str()));

			return false;
		}
		
		// 尝试在主entity文件中加载detailLevel数据
		if(!loadDetailLevelInfo(defFilePath, moduleName, defxml.get(), defNode, pScriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: failed to load entity({}) DetailLevelInfo!\n",
				moduleName.c_str()));

			return false;
		}

		pScriptModule->onLoaded();
	}
	XML_FOR_END(node);

	EntityDef::md5().final();

	if(loadComponentType == DBMGR_TYPE)
		return true;

	return loadAllEntityScriptModules(__entitiesPath, scriptBaseTypes) && initializeWatcher();
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDefInfo(const std::string& defFilePath, 
							const std::string& moduleName, 
							XML* defxml, 
							TiXmlNode* defNode, 
							ScriptDefModule* pScriptModule)
{
	if(!loadAllDefDescriptions(moduleName, defxml, defNode, pScriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to loadAllDefDescription(), entity:{}\n",
			moduleName.c_str()));

		return false;
	}
	
	// 遍历所有的interface， 并将他们的方法和属性加入到模块中
	if(!loadInterfaces(defFilePath, moduleName, defxml, defNode, pScriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} interface.\n",
			moduleName.c_str()));

		return false;
	}
	
	// 遍历所有的interface， 并将他们的方法和属性加入到模块中
	if (!loadComponents(defFilePath, moduleName, defxml, defNode, pScriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} component.\n",
			moduleName.c_str()));

		return false;
	}

	// 加载父类所有的内容
	if(!loadParentClass(defFilePath, moduleName, defxml, defNode, pScriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} parentClass.\n",
			moduleName.c_str()));

		return false;
	}

	// 尝试加载detailLevel数据
	if(!loadDetailLevelInfo(defFilePath, moduleName, defxml, defNode, pScriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} DetailLevelInfo.\n",
			moduleName.c_str()));

		return false;
	}

	// 尝试加载VolatileInfo数据
	if(!loadVolatileInfo(defFilePath, moduleName, defxml, defNode, pScriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} VolatileInfo.\n",
			moduleName.c_str()));

		return false;
	}
	
	pScriptModule->autoMatchCompOwn();
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDetailLevelInfo(const std::string& defFilePath, 
									const std::string& moduleName, 
									XML* defxml, 
									TiXmlNode* defNode, 
									ScriptDefModule* pScriptModule)
{
	TiXmlNode* detailLevelNode = defxml->enterNode(defNode, "DetailLevels");
	if(detailLevelNode == NULL)
		return true;

	DetailLevel& dlInfo = pScriptModule->getDetailLevel();
	
	TiXmlNode* node = defxml->enterNode(detailLevelNode, "NEAR");
	TiXmlNode* radiusNode = defxml->enterNode(node, "radius");
	TiXmlNode* hystNode = defxml->enterNode(node, "hyst");
	if(node == NULL || radiusNode == NULL || hystNode == NULL) 
	{
		ERROR_MSG(fmt::format("EntityDef::loadDetailLevelInfo: failed to load entity:{} NEAR-DetailLevelInfo.\n",
			moduleName.c_str()));

		return false;
	}
	
	dlInfo.level[DETAIL_LEVEL_NEAR].radius = (float)defxml->getValFloat(radiusNode);
	dlInfo.level[DETAIL_LEVEL_NEAR].hyst = (float)defxml->getValFloat(hystNode);
	
	node = defxml->enterNode(detailLevelNode, "MEDIUM");
	radiusNode = defxml->enterNode(node, "radius");
	hystNode = defxml->enterNode(node, "hyst");
	if(node == NULL || radiusNode == NULL || hystNode == NULL) 
	{
		ERROR_MSG(fmt::format("EntityDef::loadDetailLevelInfo: failed to load entity:{} MEDIUM-DetailLevelInfo.\n",
			moduleName.c_str()));

		return false;
	}
	
	dlInfo.level[DETAIL_LEVEL_MEDIUM].radius = (float)defxml->getValFloat(radiusNode);

	dlInfo.level[DETAIL_LEVEL_MEDIUM].radius += dlInfo.level[DETAIL_LEVEL_NEAR].radius + 
												dlInfo.level[DETAIL_LEVEL_NEAR].hyst;

	dlInfo.level[DETAIL_LEVEL_MEDIUM].hyst = (float)defxml->getValFloat(hystNode);
	
	node = defxml->enterNode(detailLevelNode, "FAR");
	radiusNode = defxml->enterNode(node, "radius");
	hystNode = defxml->enterNode(node, "hyst");
	if(node == NULL || radiusNode == NULL || hystNode == NULL) 
	{
		ERROR_MSG(fmt::format("EntityDef::loadDetailLevelInfo: failed to load entity:{} FAR-DetailLevelInfo.\n", 
			moduleName.c_str()));

		return false;
	}
	
	dlInfo.level[DETAIL_LEVEL_FAR].radius = (float)defxml->getValFloat(radiusNode);

	dlInfo.level[DETAIL_LEVEL_FAR].radius += dlInfo.level[DETAIL_LEVEL_MEDIUM].radius + 
													dlInfo.level[DETAIL_LEVEL_MEDIUM].hyst;

	dlInfo.level[DETAIL_LEVEL_FAR].hyst = (float)defxml->getValFloat(hystNode);

	return true;

}

//-------------------------------------------------------------------------------------
bool EntityDef::loadVolatileInfo(const std::string& defFilePath, 
									const std::string& moduleName, 
									XML* defxml, 
									TiXmlNode* defNode, 
									ScriptDefModule* pScriptModule)
{
	TiXmlNode* pNode = defxml->enterNode(defNode, "Volatile");
	if(pNode == NULL)
		return true;

	VolatileInfo* pVolatileInfo = pScriptModule->getPVolatileInfo();
	
	TiXmlNode* node = defxml->enterNode(pNode, "position");
	if(node) 
	{
		pVolatileInfo->position((float)defxml->getValFloat(node));
	}
	else
	{
		if(defxml->hasNode(pNode, "position"))
			pVolatileInfo->position(VolatileInfo::ALWAYS);
		else
			pVolatileInfo->position(-1.f);
	}

	node = defxml->enterNode(pNode, "yaw");
	if(node) 
	{
		pVolatileInfo->yaw((float)defxml->getValFloat(node));
	}
	else
	{
		if(defxml->hasNode(pNode, "yaw"))
			pVolatileInfo->yaw(VolatileInfo::ALWAYS);
		else
			pVolatileInfo->yaw(-1.f);
	}

	node = defxml->enterNode(pNode, "pitch");
	if(node) 
	{
		pVolatileInfo->pitch((float)defxml->getValFloat(node));
	}
	else
	{
		if(defxml->hasNode(pNode, "pitch"))
			pVolatileInfo->pitch(VolatileInfo::ALWAYS);
		else
			pVolatileInfo->pitch(-1.f);
	}

	node = defxml->enterNode(pNode, "roll");
	if(node) 
	{
		pVolatileInfo->roll((float)defxml->getValFloat(node));
	}
	else
	{
		if(defxml->hasNode(pNode, "roll"))
			pVolatileInfo->roll(VolatileInfo::ALWAYS);
		else
			pVolatileInfo->roll(-1.f);
	}

	node = defxml->enterNode(pNode, "optimized");
	if (node)
	{
		pVolatileInfo->optimized(defxml->getBool(node));
	}
	else
	{
		if (defxml->hasNode(pNode, "optimized"))
			pVolatileInfo->optimized(true);
		else
			pVolatileInfo->optimized(true);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadInterfaces(const std::string& defFilePath, 
							   const std::string& moduleName, 
							   XML* defxml, 
							   TiXmlNode* defNode, 
							   ScriptDefModule* pScriptModule, bool ignoreComponents)
{
	TiXmlNode* implementsNode = defxml->enterNode(defNode, "Interfaces");
	if(implementsNode == NULL)
		return true;

	XML_FOR_BEGIN(implementsNode)
	{
		if (defxml->getKey(implementsNode) != "interface" && defxml->getKey(implementsNode) != "Interface" && 
			defxml->getKey(implementsNode) != "type" && defxml->getKey(implementsNode) != "Type")
			continue;

		TiXmlNode* interfaceNode = defxml->enterNode(implementsNode, "Interface");
		if (!interfaceNode)
		{
			interfaceNode = defxml->enterNode(implementsNode, "interface");
			if (!interfaceNode)
			{
				interfaceNode = defxml->enterNode(implementsNode, "Type");
				if (!interfaceNode)
				{
					interfaceNode = defxml->enterNode(implementsNode, "type");
					if (!interfaceNode)
					{
						continue;
					}
				}
			}
		}

		std::string interfaceName = defxml->getKey(interfaceNode);
		std::string interfacefile = defFilePath + "interfaces/" + interfaceName + ".def";
		SmartPointer<XML> interfaceXml(new XML());
		if(!interfaceXml.get()->openSection(interfacefile.c_str()))
			return false;

		TiXmlNode* interfaceRootNode = interfaceXml->getRootNode();
		if(interfaceRootNode == NULL)
		{
			// root节点下没有子节点了
			return true;
		}

		if(!loadAllDefDescriptions(moduleName, interfaceXml.get(), interfaceRootNode, pScriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: interface[{}] error!\n", 
				interfaceName.c_str()));

			return false;
		}

		// 尝试加载detailLevel数据
		if(!loadDetailLevelInfo(defFilePath, moduleName, interfaceXml.get(), interfaceRootNode, pScriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::loadInterfaces: failed to load entity:{} DetailLevelInfo.\n",
				moduleName.c_str()));

			return false;
		}

		// 遍历所有的interface， 并将他们的方法和属性加入到模块中
		if (!ignoreComponents)
		{
			if (!loadComponents(defFilePath, moduleName, interfaceXml.get(), interfaceRootNode, pScriptModule))
			{
				ERROR_MSG(fmt::format("EntityDef::loadInterfaces: failed to load entity:{} component.\n",
					moduleName.c_str()));

				return false;
			}
		}

		// 遍历所有的interface， 并将他们的方法和属性加入到模块中
		if(!loadInterfaces(defFilePath, moduleName, interfaceXml.get(), interfaceRootNode, pScriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::loadInterfaces: failed to load entity:{} interface.\n",
				moduleName.c_str()));

			return false;
		}
	}
	XML_FOR_END(implementsNode);

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadComponents(const std::string& defFilePath,
	const std::string& moduleName,
	XML* defxml,
	TiXmlNode* defNode,
	ScriptDefModule* pScriptModule)
{
	TiXmlNode* implementsNode = defxml->enterNode(defNode, "Components");
	if (implementsNode == NULL)
		return true;

	XML_FOR_BEGIN(implementsNode)
	{
		std::string componentName = defxml->getKey(implementsNode);

		TiXmlNode* componentNode = defxml->enterNode(implementsNode, componentName.c_str());
		if (!componentNode)
			continue;

		if (!validDefPropertyName(pScriptModule, componentName))
		{
			ERROR_MSG(fmt::format("EntityDef::loadComponents: '{}' is limited, in module({})!\n",
				componentName, moduleName));

			return false;
		}

		std::string componentTypeName = "";
		TiXmlNode* componentTypeNameNode = defxml->enterNode(componentNode, "Type");
		if (componentTypeNameNode)
			componentTypeName = defxml->getKey(componentTypeNameNode);

		if (componentTypeName == "")
		{
			ERROR_MSG(fmt::format("EntityDef::loadComponents: component name is NULL.\n",
				componentName.c_str()));

			return false;
		}

		std::string componentfile = defFilePath + "components/" + componentTypeName + ".def";
		SmartPointer<XML> componentXml(new XML());
		if (!componentXml.get()->openSection(componentfile.c_str()))
			return false;

		// 产生一个属性描述实例
		ENTITY_PROPERTY_UID			futype = 0;
		uint32						flags = ED_FLAG_BASE | ED_FLAG_CELL_PUBLIC | ENTITY_CLIENT_DATA_FLAGS;
		bool						isPersistent = true;
		bool						isIdentifier = false;		// 是否是一个索引键
		uint32						databaseLength = 0;			// 这个属性在数据库中的长度
		std::string					indexType = "";
		DETAIL_TYPE					detailLevel = DETAIL_LEVEL_FAR;
		std::string					detailLevelStr = "";
		std::string					strisPersistent;
		std::string					defaultStr = "";

		TiXmlNode* utypeValNode = defxml->enterNode(componentNode, "Utype");

		if (!calcDefPropertyUType(moduleName, componentName, (utypeValNode ? defxml->getValInt(utypeValNode) : -1), pScriptModule, futype))
			return false;

		TiXmlNode* persistentNode = defxml->enterNode(componentNode, "Persistent");
		if (persistentNode)
		{
			strisPersistent = defxml->getValStr(persistentNode);

			std::transform(strisPersistent.begin(), strisPersistent.end(),
				strisPersistent.begin(), tolower);

			if (strisPersistent == "false")
				isPersistent = false;
		}

		// 查找是否有这个模块，如果有说明已经加载过相关描述，这里无需再次加载
		ScriptDefModule* pCompScriptDefModule = findScriptModule(componentTypeName.c_str(), false);

		if (!pCompScriptDefModule)
		{
			__scriptTypeMappingUType[componentTypeName] = g_scriptUtype;
			pCompScriptDefModule = new ScriptDefModule(componentTypeName, g_scriptUtype++);
			pCompScriptDefModule->isPersistent(false);
			pCompScriptDefModule->isComponentModule(true);

			EntityDef::__scriptModules.push_back(pCompScriptDefModule);
		}
		else
		{
			flags = ED_FLAG_UNKOWN;

			if (pCompScriptDefModule->hasBase())
				flags |= ED_FLAG_BASE;

			if (pCompScriptDefModule->hasCell())
				flags |= ED_FLAG_CELL_PUBLIC;

			if (pCompScriptDefModule->hasClient())
			{
				if (pCompScriptDefModule->hasBase())
					flags |= ED_FLAG_BASE_AND_CLIENT;
				else
					flags |= (ED_FLAG_ALL_CLIENTS | ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OTHER_CLIENTS | ED_FLAG_OWN_CLIENT);
			}

			g_logComponentPropertys[pScriptModule->getName()].push_back(addComponentProperty(futype, componentTypeName, componentName, flags, isPersistent, isIdentifier,
				indexType, databaseLength, defaultStr, detailLevel, pScriptModule, pCompScriptDefModule));

			pScriptModule->addComponentDescription(componentName.c_str(), pCompScriptDefModule);
			continue;
		}

		TiXmlNode* componentRootNode = componentXml->getRootNode();
		if (componentRootNode == NULL)
		{
			// root节点下没有子节点了
			return true;
		}

		if (!loadAllDefDescriptions(componentTypeName, componentXml.get(), componentRootNode, pCompScriptDefModule))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: component[{}] error!\n",
				componentTypeName.c_str()));

			return false;
		}

		// 尝试加载detailLevel数据
		if (!loadDetailLevelInfo(defFilePath, componentTypeName, componentXml.get(), componentRootNode, pCompScriptDefModule))
		{
			ERROR_MSG(fmt::format("EntityDef::loadComponents: failed to load component:{} DetailLevelInfo.\n",
				componentTypeName.c_str()));

			return false;
		}

		// 遍历所有的interface， 并将他们的方法和属性加入到模块中
		if (!loadInterfaces(defFilePath, componentTypeName, componentXml.get(), componentRootNode, pCompScriptDefModule, true))
		{
			ERROR_MSG(fmt::format("EntityDef::loadComponents: failed to load component:{} interface.\n",
				componentTypeName.c_str()));

			return false;
		}
		
		pCompScriptDefModule->autoMatchCompOwn();

		flags = ED_FLAG_UNKOWN;

		if (pCompScriptDefModule->hasBase())
			flags |= ED_FLAG_BASE;

		if (pCompScriptDefModule->hasCell())
			flags |= ED_FLAG_CELL_PUBLIC;

		if (pCompScriptDefModule->hasClient())
		{
			if (pCompScriptDefModule->hasBase())
				flags |= ED_FLAG_BASE_AND_CLIENT;
			
			if (pCompScriptDefModule->hasCell())
				flags |= (ED_FLAG_ALL_CLIENTS | ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OTHER_CLIENTS | ED_FLAG_OWN_CLIENT);
		}

		g_logComponentPropertys[pScriptModule->getName()].push_back(addComponentProperty(futype, componentTypeName, componentName, flags, isPersistent, isIdentifier,
			indexType, databaseLength, defaultStr, detailLevel, pScriptModule, pCompScriptDefModule));

		pScriptModule->addComponentDescription(componentName.c_str(), pCompScriptDefModule);
	}
	XML_FOR_END(implementsNode);

	return true;
}

//-------------------------------------------------------------------------------------
PropertyDescription* EntityDef::addComponentProperty(ENTITY_PROPERTY_UID utype,
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
	ScriptDefModule* pCompScriptDefModule)
{
	DataType* pEntityComponentType = DataTypes::getDataType(componentTypeName, false);

	if (!pEntityComponentType)
		pEntityComponentType = new EntityComponentType(pCompScriptDefModule);

	PropertyDescription* propertyDescription = PropertyDescription::createDescription(utype, "EntityComponent",
		componentName, flags, isPersistent,
		pEntityComponentType, isIdentifier, indexType,
		databaseLength, defaultStr,
		detailLevel);

	bool ret = true;

	int32 hasBaseFlags = 0;
	int32 hasCellFlags = 0;
	int32 hasClientFlags = 0;

	hasBaseFlags = flags & ENTITY_BASE_DATA_FLAGS;
	if (hasBaseFlags > 0)
		pScriptModule->setBase(true);

	hasCellFlags = flags & ENTITY_CELL_DATA_FLAGS;
	if (hasCellFlags > 0)
		pScriptModule->setCell(true);

	hasClientFlags = flags & ENTITY_CLIENT_DATA_FLAGS;
	if (hasClientFlags > 0)
		pScriptModule->setClient(true);

	// 添加到模块中
	if (hasCellFlags > 0)
		ret = pScriptModule->addPropertyDescription(componentName.c_str(),
			propertyDescription, CELLAPP_TYPE);

	if (hasBaseFlags > 0)
		ret = pScriptModule->addPropertyDescription(componentName.c_str(),
			propertyDescription, BASEAPP_TYPE);

	if (hasClientFlags > 0)
		ret = pScriptModule->addPropertyDescription(componentName.c_str(),
			propertyDescription, CLIENT_TYPE);

	if (!ret)
	{
		ERROR_MSG(fmt::format("EntityDef::addComponentProperty({}): {}.\n",
			pScriptModule->getName(), componentName));

		SAFE_RELEASE(propertyDescription);
		return NULL;
	}

	return propertyDescription;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadParentClass(const std::string& defFilePath, 
								const std::string& moduleName, 
								XML* defxml, 
								TiXmlNode* defNode, 
								ScriptDefModule* pScriptModule)
{
	TiXmlNode* parentClassNode = defxml->enterNode(defNode, "Parent");
	if(parentClassNode == NULL)
		return true;

	std::string parentClassName = defxml->getKey(parentClassNode);
	std::string parentClassfile = defFilePath + parentClassName + ".def";
	
	SmartPointer<XML> parentClassXml(new XML());
	if(!parentClassXml->openSection(parentClassfile.c_str()))
		return false;
	
	TiXmlNode* parentClassdefNode = parentClassXml->getRootNode();
	if(parentClassdefNode == NULL)
	{
		// root节点下没有子节点了
		return true;
	}

	// 加载def文件中的定义
	if(!loadDefInfo(defFilePath, parentClassName, parentClassXml.get(), parentClassdefNode, pScriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadParentClass: failed to load entity:{} parentClass.\n",
			moduleName.c_str()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadAllDefDescriptions(const std::string& moduleName, 
									  XML* defxml, 
									  TiXmlNode* defNode, 
									  ScriptDefModule* pScriptModule)
{
	// 加载属性描述
	if(!loadDefPropertys(moduleName, defxml, defxml->enterNode(defNode, "Properties"), pScriptModule))
		return false;
	
	// 加载cell方法描述
	if(!loadDefCellMethods(moduleName, defxml, defxml->enterNode(defNode, "CellMethods"), pScriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadAllDefDescription:loadDefCellMethods[{}] is failed!\n",
			moduleName.c_str()));

		return false;
	}

	// 加载base方法描述
	if(!loadDefBaseMethods(moduleName, defxml, defxml->enterNode(defNode, "BaseMethods"), pScriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadAllDefDescription:loadDefBaseMethods[{}] is failed!\n",
			moduleName.c_str()));

		return false;
	}

	// 加载client方法描述
	if(!loadDefClientMethods(moduleName, defxml, defxml->enterNode(defNode, "ClientMethods"), pScriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadAllDefDescription:loadDefClientMethods[{}] is failed!\n",
			moduleName.c_str()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::validDefPropertyName(ScriptDefModule* pScriptModule, const std::string& name)
{
	int i = 0;

	while(true)
	{
		std::string limited = ENTITY_LIMITED_PROPERTYS[i];

		if(limited == "")
			break;

		if(name == limited)
			return false;
		
		++i;
	};

	PyObject* pyKBEModule =
		PyImport_ImportModule(const_cast<char*>("KBEngine"));

	PyObject* pyEntityModule =
		PyObject_GetAttrString(pyKBEModule, const_cast<char *>("Entity"));

	Py_DECREF(pyKBEModule);

	if (pyEntityModule != NULL)
	{
		PyObject* pyEntityAttr =
			PyObject_GetAttrString(pyEntityModule, const_cast<char *>(name.c_str()));

		if (pyEntityAttr != NULL)
		{
			Py_DECREF(pyEntityAttr);
			Py_DECREF(pyEntityModule);
			return false;
		}
		else
		{
			PyErr_Clear();
		}
	}
	else
	{
		PyErr_Clear();
	}

	Py_XDECREF(pyEntityModule);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::calcDefPropertyUType(const std::string& moduleName, 
	const std::string& name, int iUtype, ScriptDefModule* pScriptModule, ENTITY_PROPERTY_UID& outUtype)
{
	ENTITY_PROPERTY_UID futype = 0;
	outUtype = futype;

	if (iUtype > 0)
	{
		futype = iUtype;

		if (iUtype != int(futype))
		{
			ERROR_MSG(fmt::format("EntityDef::calcDefPropertyUType: 'Utype' has overflowed({} > 65535), is {}.{}!\n",
				iUtype, moduleName, name.c_str()));

			return false;
		}

		// 检查是否有重复的Utype
		std::vector<ENTITY_PROPERTY_UID>::iterator iter =
			std::find(g_propertyUtypes.begin(), g_propertyUtypes.end(), futype);

		if (iter != g_propertyUtypes.end())
		{
			bool foundConflict = false;

			PropertyDescription* pConflictPropertyDescription = pScriptModule->findPropertyDescription(futype, BASEAPP_TYPE);
			if (pConflictPropertyDescription)
			{
				ERROR_MSG(fmt::format("EntityDef::calcDefPropertyUType: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
					moduleName, name.c_str(), iUtype, moduleName, pConflictPropertyDescription->getName(), iUtype));

				foundConflict = true;
			}

			pConflictPropertyDescription = pScriptModule->findPropertyDescription(futype, CELLAPP_TYPE);
			if (pConflictPropertyDescription)
			{
				ERROR_MSG(fmt::format("EntityDef::calcDefPropertyUType: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
					moduleName, name.c_str(), iUtype, moduleName, pConflictPropertyDescription->getName(), iUtype));

				foundConflict = true;
			}

			pConflictPropertyDescription = pScriptModule->findPropertyDescription(futype, CLIENT_TYPE);
			if (pConflictPropertyDescription)
			{
				ERROR_MSG(fmt::format("EntityDef::calcDefPropertyUType: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
					moduleName, name.c_str(), iUtype, moduleName, pConflictPropertyDescription->getName(), iUtype));

				foundConflict = true;
			}

			if (foundConflict)
				return false;
		}

		g_propertyUtypes.push_back(futype);
	}
	else
	{
		while (true)
		{
			futype = g_propertyUtypeAuto++;
			std::vector<ENTITY_PROPERTY_UID>::iterator iter =
				std::find(g_propertyUtypes.begin(), g_propertyUtypes.end(), futype);

			if (iter == g_propertyUtypes.end())
				break;
		}

		g_propertyUtypes.push_back(futype);
	}

	outUtype = futype;
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDefPropertys(const std::string& moduleName, 
								 XML* xml, 
								 TiXmlNode* defPropertyNode, 
								 ScriptDefModule* pScriptModule)
{
	if(defPropertyNode)
	{
		XML_FOR_BEGIN(defPropertyNode)
		{
			ENTITY_PROPERTY_UID			futype = 0;
			uint32						flags = 0;
			int32						hasBaseFlags = 0;
			int32						hasCellFlags = 0;
			int32						hasClientFlags = 0;
			DataType*					dataType = NULL;
			bool						isPersistent = false;
			bool						isIdentifier = false;		// 是否是一个索引键
			uint32						databaseLength = 0;			// 这个属性在数据库中的长度
			std::string					indexType;
			DETAIL_TYPE					detailLevel = DETAIL_LEVEL_FAR;
			std::string					detailLevelStr = "";
			std::string					strType;
			std::string					strisPersistent;
			std::string					strFlags;
			std::string					strIdentifierNode;
			std::string					defaultStr;
			std::string					name = "";

			name = xml->getKey(defPropertyNode);
			if(!validDefPropertyName(pScriptModule, name))
			{
				ERROR_MSG(fmt::format("EntityDef::loadDefPropertys: '{}' is limited, in module({})!\n", 
					name, moduleName));

				return false;
			}

			TiXmlNode* flagsNode = xml->enterNode(defPropertyNode->FirstChild(), "Flags");
			if(flagsNode)
			{
				strFlags = xml->getValStr(flagsNode);
				std::transform(strFlags.begin(), strFlags.end(), strFlags.begin(), toupper);

				ENTITYFLAGMAP::iterator iter = g_entityFlagMapping.find(strFlags.c_str());
				if(iter == g_entityFlagMapping.end())
				{
					ERROR_MSG(fmt::format("EntityDef::loadDefPropertys: not fount flags[{}], is {}.{}!\n", 
						strFlags, moduleName, name));

					return false;
				}

				flags = iter->second;
				hasBaseFlags = flags & ENTITY_BASE_DATA_FLAGS;
				if(hasBaseFlags > 0)
					pScriptModule->setBase(true);

				hasCellFlags = flags & ENTITY_CELL_DATA_FLAGS;
				if(hasCellFlags > 0)
					pScriptModule->setCell(true);

				hasClientFlags = flags & ENTITY_CLIENT_DATA_FLAGS;
				if(hasClientFlags > 0)
					pScriptModule->setClient(true);

				if(hasBaseFlags <= 0 && hasCellFlags <= 0)
				{
					ERROR_MSG(fmt::format("EntityDef::loadDefPropertys: not fount flags[{}], is {}.{}!\n",
						strFlags.c_str(), moduleName, name.c_str()));

					return false;
				}
			}
			else
			{
				ERROR_MSG(fmt::format("EntityDef::loadDefPropertys: not fount flagsNode, is {}.{}!\n",
					moduleName, name.c_str()));

				return false;
			}

			TiXmlNode* persistentNode = xml->enterNode(defPropertyNode->FirstChild(), "Persistent");
			if(persistentNode)
			{
				strisPersistent = xml->getValStr(persistentNode);

				std::transform(strisPersistent.begin(), strisPersistent.end(), 
					strisPersistent.begin(), tolower);

				if(strisPersistent == "true")
					isPersistent = true;
			}

			TiXmlNode* typeNode = xml->enterNode(defPropertyNode->FirstChild(), "Type");
			if(typeNode)
			{
				strType = xml->getValStr(typeNode);

				if(strType == "ARRAY")
				{
					FixedArrayType* dataType1 = new FixedArrayType();
					if(dataType1->initialize(xml, typeNode, moduleName + "_" + name))
						dataType = dataType1;
					else
						return false;
				}
				else
				{
					dataType = DataTypes::getDataType(strType);
				}

				if(dataType == NULL)
				{
					return false;
				}
			}
			else
			{
				ERROR_MSG(fmt::format("EntityDef::loadDefPropertys: not fount TypeNode, is {}.{}!\n",
					moduleName, name.c_str()));

				return false;
			}

			TiXmlNode* indexTypeNode = xml->enterNode(defPropertyNode->FirstChild(), "Index");
			if(indexTypeNode)
			{
				indexType = xml->getValStr(indexTypeNode);

				std::transform(indexType.begin(), indexType.end(), 
					indexType.begin(), toupper);
			}
			

			TiXmlNode* identifierNode = xml->enterNode(defPropertyNode->FirstChild(), "Identifier");
			if(identifierNode)
			{
				strIdentifierNode = xml->getValStr(identifierNode);
				std::transform(strIdentifierNode.begin(), strIdentifierNode.end(), 
					strIdentifierNode.begin(), tolower);

				if(strIdentifierNode == "true")
					isIdentifier = true;
			}

			TiXmlNode* databaseLengthNode = xml->enterNode(defPropertyNode->FirstChild(), "DatabaseLength");
			if(databaseLengthNode)
			{
				databaseLength = xml->getValInt(databaseLengthNode);
			}

			TiXmlNode* defaultValNode = 
				xml->enterNode(defPropertyNode->FirstChild(), "Default");

			if(defaultValNode)
			{
				defaultStr = xml->getValStr(defaultValNode);
			}
			
			TiXmlNode* detailLevelNode = 
				xml->enterNode(defPropertyNode->FirstChild(), "DetailLevel");

			if(detailLevelNode)
			{
				detailLevelStr = xml->getValStr(detailLevelNode);
				if(detailLevelStr == "FAR")
					detailLevel = DETAIL_LEVEL_FAR;
				else if(detailLevelStr == "MEDIUM")
					detailLevel = DETAIL_LEVEL_MEDIUM;
				else if(detailLevelStr == "NEAR")
					detailLevel = DETAIL_LEVEL_NEAR;
				else
					detailLevel = DETAIL_LEVEL_FAR;
			}
			
			TiXmlNode* utypeValNode = 
				xml->enterNode(defPropertyNode->FirstChild(), "Utype");

			if (!calcDefPropertyUType(moduleName, name, (utypeValNode ? xml->getValInt(utypeValNode) : -1), pScriptModule, futype))
				return false;

			// 产生一个属性描述实例
			PropertyDescription* propertyDescription = PropertyDescription::createDescription(futype, strType, 
															name, flags, isPersistent, 
															dataType, isIdentifier, indexType,
															databaseLength, defaultStr, 
															detailLevel);

			bool ret = true;

			// 添加到模块中
			if(hasCellFlags > 0)
				ret = pScriptModule->addPropertyDescription(name.c_str(),
						propertyDescription, CELLAPP_TYPE);

			if(hasBaseFlags > 0)
				ret = pScriptModule->addPropertyDescription(name.c_str(),
						propertyDescription, BASEAPP_TYPE);

			if(hasClientFlags > 0)
				ret = pScriptModule->addPropertyDescription(name.c_str(),
						propertyDescription, CLIENT_TYPE);

			if(!ret)
			{
				ERROR_MSG(fmt::format("EntityDef::addPropertyDescription({}): {}.\n", 
					moduleName.c_str(), xml->getTxdoc()->Value()));
				
				return false;
			}
		}
		XML_FOR_END(defPropertyNode);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDefCellMethods(const std::string& moduleName, 
								   XML* xml, 
								   TiXmlNode* defMethodNode, 
								   ScriptDefModule* pScriptModule)
{
	if(defMethodNode)
	{
		XML_FOR_BEGIN(defMethodNode)
		{
			std::string name = xml->getKey(defMethodNode);
			MethodDescription* methodDescription = new MethodDescription(0, CELLAPP_TYPE, name);
			TiXmlNode* argNode = defMethodNode->FirstChild();
			
			// 可能没有参数
			if(argNode)
			{
				XML_FOR_BEGIN(argNode)
				{
					std::string argType = xml->getKey(argNode);

					if(argType == "Exposed")
					{
						methodDescription->setExposed();
					}
					else if(argType == "Arg")
					{
						DataType* dataType = NULL;
						TiXmlNode* typeNode = argNode->FirstChild();
						std::string strType = xml->getValStr(typeNode);

						if(strType == "ARRAY")
						{
							FixedArrayType* dataType1 = new FixedArrayType();
							if(dataType1->initialize(xml, typeNode, moduleName + "_" + name))
								dataType = dataType1;
						}
						else
						{
							dataType = DataTypes::getDataType(strType);
						}

						if(dataType == NULL)
						{
							ERROR_MSG(fmt::format("EntityDef::loadDefCellMethods: dataType[{}] not found, in {}!\n", 
								strType.c_str(), name.c_str()));

							return false;
						}

						methodDescription->pushArgType(dataType);
					}
					else if(argType == "Utype")
					{
						TiXmlNode* typeNode = argNode->FirstChild();

						int iUtype = xml->getValInt(typeNode);
						ENTITY_METHOD_UID muid = iUtype;
						
						if (iUtype != int(muid))
						{
							ERROR_MSG(fmt::format("EntityDef::loadDefCellMethods: 'Utype' has overflowed({} > 65535), is {}.{}!\n",
								iUtype, moduleName, name.c_str()));

							return false;
						}

						methodDescription->setUType(muid);
						g_methodCusUtypes.push_back(muid);
					}
				}
				XML_FOR_END(argNode);		
			}

			// 如果配置中没有设置过utype, 则产生
			if(methodDescription->getUType() <= 0)
			{
				ENTITY_METHOD_UID muid = 0;
				while(true)
				{
					muid = g_methodUtypeAuto++;
					std::vector<ENTITY_METHOD_UID>::iterator iterutype = 
						std::find(g_methodCusUtypes.begin(), g_methodCusUtypes.end(), muid);

					if(iterutype == g_methodCusUtypes.end())
					{
						break;
					}
				}

				methodDescription->setUType(muid);
				g_methodCusUtypes.push_back(muid);
			}
			else
			{
				// 检查是否有重复的Utype
				ENTITY_METHOD_UID muid = methodDescription->getUType();
				std::vector<ENTITY_METHOD_UID>::iterator iter =
					std::find(g_methodCusUtypes.begin(), g_methodCusUtypes.end(), muid);

				if (iter != g_methodCusUtypes.end())
				{
					bool foundConflict = false;

					MethodDescription* pConflictMethodDescription = pScriptModule->findBaseMethodDescription(muid);
					if (pConflictMethodDescription)
					{
						ERROR_MSG(fmt::format("EntityDef::loadDefCellMethods: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
							moduleName, name.c_str(), muid, moduleName, pConflictMethodDescription->getName(), muid));

						foundConflict = true;
					}

					pConflictMethodDescription = pScriptModule->findCellMethodDescription(muid);
					if (pConflictMethodDescription)
					{
						ERROR_MSG(fmt::format("EntityDef::loadDefCellMethods: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
							moduleName, name.c_str(), muid, moduleName, pConflictMethodDescription->getName(), muid));

						foundConflict = true;
					}

					pConflictMethodDescription = pScriptModule->findClientMethodDescription(muid);
					if (pConflictMethodDescription)
					{
						ERROR_MSG(fmt::format("EntityDef::loadDefCellMethods: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
							moduleName, name.c_str(), muid, moduleName, pConflictMethodDescription->getName(), muid));

						foundConflict = true;
					}

					if (foundConflict)
						return false;
				}
			}

			if(!pScriptModule->addCellMethodDescription(name.c_str(), methodDescription))
				return false;
		}
		XML_FOR_END(defMethodNode);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDefBaseMethods(const std::string& moduleName, XML* xml, 
								   TiXmlNode* defMethodNode, ScriptDefModule* pScriptModule)
{
	if(defMethodNode)
	{
		XML_FOR_BEGIN(defMethodNode)
		{
			std::string name = xml->getKey(defMethodNode);
			MethodDescription* methodDescription = new MethodDescription(0, BASEAPP_TYPE, name);
			TiXmlNode* argNode = defMethodNode->FirstChild();

			// 可能没有参数
			if(argNode)
			{
				XML_FOR_BEGIN(argNode)
				{
					std::string argType = xml->getKey(argNode);

					if(argType == "Exposed")
					{
						methodDescription->setExposed();
					}
					else if(argType == "Arg")
					{
						DataType* dataType = NULL;
						TiXmlNode* typeNode = argNode->FirstChild();
						std::string strType = xml->getValStr(typeNode);

						if(strType == "ARRAY")
						{
							FixedArrayType* dataType1 = new FixedArrayType();
							if(dataType1->initialize(xml, typeNode, moduleName + "_" + name))
								dataType = dataType1;
						}
						else
						{
							dataType = DataTypes::getDataType(strType);
						}

						if(dataType == NULL)
						{
							ERROR_MSG(fmt::format("EntityDef::loadDefBaseMethods: dataType[{}] not found, in {}!\n",
								strType.c_str(), name.c_str()));

							return false;
						}

						methodDescription->pushArgType(dataType);
					}
					else if(argType == "Utype")
					{
						TiXmlNode* typeNode = argNode->FirstChild();

						int iUtype = xml->getValInt(typeNode);
						ENTITY_METHOD_UID muid = iUtype;

						if (iUtype != int(muid))
						{
							ERROR_MSG(fmt::format("EntityDef::loadDefBaseMethods: 'Utype' has overflowed({} > 65535), is {}.{}!\n",
								iUtype, moduleName, name.c_str()));

							return false;
						}

						methodDescription->setUType(muid);
						g_methodCusUtypes.push_back(muid);
					}
				}
				XML_FOR_END(argNode);		
			}

			// 如果配置中没有设置过utype, 则产生
			if(methodDescription->getUType() <= 0)
			{
				ENTITY_METHOD_UID muid = 0;
				while(true)
				{
					muid = g_methodUtypeAuto++;
					std::vector<ENTITY_METHOD_UID>::iterator iterutype = 
						std::find(g_methodCusUtypes.begin(), g_methodCusUtypes.end(), muid);

					if(iterutype == g_methodCusUtypes.end())
					{
						break;
					}
				}

				methodDescription->setUType(muid);
				g_methodCusUtypes.push_back(muid);
			}
			else
			{
				// 检查是否有重复的Utype
				ENTITY_METHOD_UID muid = methodDescription->getUType();
				std::vector<ENTITY_METHOD_UID>::iterator iter =
					std::find(g_methodCusUtypes.begin(), g_methodCusUtypes.end(), muid);

				if (iter != g_methodCusUtypes.end())
				{
					bool foundConflict = false;

					MethodDescription* pConflictMethodDescription = pScriptModule->findBaseMethodDescription(muid);
					if (pConflictMethodDescription)
					{
						ERROR_MSG(fmt::format("EntityDef::loadDefBaseMethods: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
							moduleName, name.c_str(), muid, moduleName, pConflictMethodDescription->getName(), muid));

						foundConflict = true;
					}

					pConflictMethodDescription = pScriptModule->findCellMethodDescription(muid);
					if (pConflictMethodDescription)
					{
						ERROR_MSG(fmt::format("EntityDef::loadDefBaseMethods: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
							moduleName, name.c_str(), muid, moduleName, pConflictMethodDescription->getName(), muid));

						foundConflict = true;
					}

					pConflictMethodDescription = pScriptModule->findClientMethodDescription(muid);
					if (pConflictMethodDescription)
					{
						ERROR_MSG(fmt::format("EntityDef::loadDefBaseMethods: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
							moduleName, name.c_str(), muid, moduleName, pConflictMethodDescription->getName(), muid));

						foundConflict = true;
					}

					if (foundConflict)
						return false;
				}
			}

			if(!pScriptModule->addBaseMethodDescription(name.c_str(), methodDescription))
				return false;
		}
		XML_FOR_END(defMethodNode);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDefClientMethods(const std::string& moduleName, XML* xml, 
									 TiXmlNode* defMethodNode, ScriptDefModule* pScriptModule)
{
	if(defMethodNode)
	{
		XML_FOR_BEGIN(defMethodNode)
		{
			std::string name = xml->getKey(defMethodNode);
			MethodDescription* methodDescription = new MethodDescription(0, CLIENT_TYPE, name);
			TiXmlNode* argNode = defMethodNode->FirstChild();

			// 可能没有参数
			if(argNode)
			{
				XML_FOR_BEGIN(argNode)
				{
					std::string argType = xml->getKey(argNode);

					if(argType == "Arg")
					{
						DataType* dataType = NULL;
						TiXmlNode* typeNode = argNode->FirstChild();
						std::string strType = xml->getValStr(typeNode);

						if(strType == "ARRAY")
						{
							FixedArrayType* dataType1 = new FixedArrayType();
							if(dataType1->initialize(xml, typeNode, moduleName + "_" + name))
								dataType = dataType1;
						}
						else
						{
							dataType = DataTypes::getDataType(strType);
						}

						if(dataType == NULL)
						{
							ERROR_MSG(fmt::format("EntityDef::loadDefClientMethods: dataType[{}] not found, in {}!\n",
								strType.c_str(), name.c_str()));

							return false;
						}

						methodDescription->pushArgType(dataType);
					}
					else if(argType == "Utype")
					{
						TiXmlNode* typeNode = argNode->FirstChild();

						int iUtype = xml->getValInt(typeNode);
						ENTITY_METHOD_UID muid = iUtype;

						if (iUtype != int(muid))
						{
							ERROR_MSG(fmt::format("EntityDef::loadDefClientMethods: 'Utype' has overflowed({} > 65535), is {}.{}!\n",
								iUtype, moduleName, name.c_str()));

							return false;
						}

						methodDescription->setUType(muid);
						g_methodCusUtypes.push_back(muid);
					}
				}
				XML_FOR_END(argNode);		
			}

			// 如果配置中没有设置过utype, 则产生
			if(methodDescription->getUType() <= 0)
			{
				ENTITY_METHOD_UID muid = 0;
				while(true)
				{
					muid = g_methodUtypeAuto++;
					std::vector<ENTITY_METHOD_UID>::iterator iterutype = 
						std::find(g_methodCusUtypes.begin(), g_methodCusUtypes.end(), muid);

					if(iterutype == g_methodCusUtypes.end())
					{
						break;
					}
				}

				methodDescription->setUType(muid);
				g_methodCusUtypes.push_back(muid);
			}
			else
			{
				// 检查是否有重复的Utype
				ENTITY_METHOD_UID muid = methodDescription->getUType();
				std::vector<ENTITY_METHOD_UID>::iterator iter =
					std::find(g_methodCusUtypes.begin(), g_methodCusUtypes.end(), muid);

				if (iter != g_methodCusUtypes.end())
				{
					bool foundConflict = false;

					MethodDescription* pConflictMethodDescription = pScriptModule->findBaseMethodDescription(muid);
					if (pConflictMethodDescription)
					{
						ERROR_MSG(fmt::format("EntityDef::loadDefClientMethods: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
							moduleName, name.c_str(), muid, moduleName, pConflictMethodDescription->getName(), muid));

						foundConflict = true;
					}

					pConflictMethodDescription = pScriptModule->findCellMethodDescription(muid);
					if (pConflictMethodDescription)
					{
						ERROR_MSG(fmt::format("EntityDef::loadDefClientMethods: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
							moduleName, name.c_str(), muid, moduleName, pConflictMethodDescription->getName(), muid));

						foundConflict = true;
					}

					pConflictMethodDescription = pScriptModule->findClientMethodDescription(muid);
					if (pConflictMethodDescription)
					{
						ERROR_MSG(fmt::format("EntityDef::loadDefClientMethods: {}.{}, 'Utype' {} Conflict({}.{} 'Utype' {})!\n",
							moduleName, name.c_str(), muid, moduleName, pConflictMethodDescription->getName(), muid));

						foundConflict = true;
					}

					if (foundConflict)
						return false;
				}
			}

			if(!pScriptModule->addClientMethodDescription(name.c_str(), methodDescription))
				return false;
		}
		XML_FOR_END(defMethodNode);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::isLoadScriptModule(ScriptDefModule* pScriptModule)
{
	switch(__loadComponentType)
	{
	case BASEAPP_TYPE:
		{
			if(!pScriptModule->hasBase())
				return false;

			break;
		}
	case CELLAPP_TYPE:
		{
			if(!pScriptModule->hasCell())
				return false;

			break;
		}
	case CLIENT_TYPE:
	case BOTS_TYPE:
		{
			if(!pScriptModule->hasClient())
				return false;

			break;
		}
	case TOOL_TYPE:
	{
		return false;
		break;
	}
	default:
		{
			if(!pScriptModule->hasCell())
				return false;

			break;
		}
	};

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::checkDefMethod(ScriptDefModule* pScriptModule, 
							   PyObject* moduleObj, const std::string& moduleName)
{
	ScriptDefModule::METHODDESCRIPTION_MAP* methodDescrsPtr = NULL;
	
	switch(__loadComponentType)
	{
	case BASEAPP_TYPE:
		methodDescrsPtr = 
			(ScriptDefModule::METHODDESCRIPTION_MAP*)&pScriptModule->getBaseMethodDescriptions();
		break;
	case CELLAPP_TYPE:
		methodDescrsPtr = 
			(ScriptDefModule::METHODDESCRIPTION_MAP*)&pScriptModule->getCellMethodDescriptions();
		break;
	case CLIENT_TYPE:
	case BOTS_TYPE:
		methodDescrsPtr = 
			(ScriptDefModule::METHODDESCRIPTION_MAP*)&pScriptModule->getClientMethodDescriptions();
		break;
	default:
		methodDescrsPtr = 
			(ScriptDefModule::METHODDESCRIPTION_MAP*)&pScriptModule->getCellMethodDescriptions();
		break;
	};

	ScriptDefModule::METHODDESCRIPTION_MAP::iterator iter = methodDescrsPtr->begin();
	for(; iter != methodDescrsPtr->end(); ++iter)
	{
		PyObject* pyMethod = 
			PyObject_GetAttrString(moduleObj, const_cast<char *>(iter->first.c_str()));

		if (pyMethod != NULL)
		{
			Py_DECREF(pyMethod);
		}
		else
		{
			ERROR_MSG(fmt::format("EntityDef::checkDefMethod: class {} does not have method[{}], defined in {}.def!\n",
					moduleName.c_str(), iter->first.c_str(), moduleName));

			PyErr_Clear();
			return false;
		}
	}
	
	return true;	
}

//-------------------------------------------------------------------------------------
void EntityDef::setScriptModuleHasComponentEntity(ScriptDefModule* pScriptModule, 
												  bool has)
{
	switch(__loadComponentType)
	{
	case BASEAPP_TYPE:
		pScriptModule->setBase(has);
		return;
	case CELLAPP_TYPE:
		pScriptModule->setCell(has);
		return;
	case CLIENT_TYPE:
	case BOTS_TYPE:
		pScriptModule->setClient(has);
		return;
	default:
		pScriptModule->setCell(has);
		return;
	};
}

//-------------------------------------------------------------------------------------
PyObject* EntityDef::loadScriptModule(std::string moduleName)
{
	PyObject* pyModule =
		PyImport_ImportModule(const_cast<char*>(moduleName.c_str()));

	if (g_isReload)
		pyModule = PyImport_ReloadModule(pyModule);

	// 检查该模块路径是否是KBE脚本目录下的，防止因用户取名与python模块名称冲突而误导入了系统模块
	if (pyModule)
	{
		std::string userScriptsPath = Resmgr::getSingleton().getPyUserScriptsPath();
		std::string pyModulePath = "";

		const char *pModulePath = PyModule_GetFilename(pyModule);
		if (pModulePath)
			pyModulePath = pModulePath;

		strutil::kbe_replace(userScriptsPath, "/", "");
		strutil::kbe_replace(userScriptsPath, "\\", "");
		strutil::kbe_replace(pyModulePath, "/", "");
		strutil::kbe_replace(pyModulePath, "\\", "");

		if (pyModulePath.find(userScriptsPath) == std::string::npos)
		{
			WARNING_MSG(fmt::format("EntityDef::initialize: The script module name[{}] and system module name conflict!\n",
				moduleName.c_str()));

			pyModule = NULL;
		}
	}

	return pyModule;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadAllComponentScriptModules(std::string entitiesPath, std::vector<PyTypeObject*>& scriptBaseTypes)
{
	std::string entitiesFile = entitiesPath + "entities.xml";

	SmartPointer<XML> xml(new XML());
	if (!xml->openSection(entitiesFile.c_str()))
		return false;

	TiXmlNode* node = xml->getRootNode();
	if (node == NULL)
		return true;

	// 所有需要加载脚本的组件类别名称
	std::set<std::string> componentTypes;

	XML_FOR_BEGIN(node)
	{
		std::string moduleName = xml.get()->getKey(node);
		ScriptDefModule* pScriptModule = findScriptModule(moduleName.c_str());

		const ScriptDefModule::COMPONENTDESCRIPTION_MAP& componentDescrs = pScriptModule->getComponentDescrs();
		ScriptDefModule::COMPONENTDESCRIPTION_MAP::const_iterator comp_iter = componentDescrs.begin();
		for (; comp_iter != componentDescrs.end(); ++comp_iter)
		{
			componentTypes.insert(comp_iter->second->getName());
		}
	}
	XML_FOR_END(node);

	// 加载实体组件的脚本
	std::set<std::string>::iterator comp_iter = componentTypes.begin();
	for (; comp_iter != componentTypes.end(); ++comp_iter)
	{
		std::string componentScriptName = (*comp_iter);
		ScriptDefModule* pScriptModule = findScriptModule(componentScriptName.c_str());
		PyObject* pyModule = loadScriptModule(componentScriptName);

		if (pyModule == NULL)
		{
			// 如果当前是工具，如kbcmd， 那么无法加载脚本，如果某个模块没有客户端则删除它
			if (g_componentType == TOOL_TYPE)
			{
				if (!pScriptModule->hasClient())
				{
					goto ERASE_PROPERTYS;
				}
				else
				{
					PyErr_Clear();
					continue;
				}
			}

			// 是否加载这个模块 （取决于是否在def文件中定义了与当前组件相关的方法或者属性）
			if (isLoadScriptModule(pScriptModule))
			{
				ERROR_MSG(fmt::format("EntityDef::initialize: Could not load EntityComponentModule[{}]\n",
					componentScriptName.c_str()));

				PyErr_Print();
				return false;
			}

ERASE_PROPERTYS:
			std::vector<ScriptDefModulePtr>::iterator entityScriptModuleIter = EntityDef::__scriptModules.begin();
			for (; entityScriptModuleIter != EntityDef::__scriptModules.end(); ++entityScriptModuleIter)
			{
				ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = (*entityScriptModuleIter)->getPropertyDescrs();
				ScriptDefModule::PROPERTYDESCRIPTION_MAP::iterator compPropertyInter = propertyDescrs.begin();
				for (; compPropertyInter != propertyDescrs.end();)
				{
					if (compPropertyInter->second->getDataType()->type() == DATA_TYPE_ENTITY_COMPONENT)
					{
						ScriptDefModule* pCompScriptModule = static_cast<EntityComponentType*>(compPropertyInter->second->getDataType())->pScriptDefModule();
						if (pCompScriptModule->getName() == componentScriptName)
						{
							uint32 flags = compPropertyInter->second->getFlags();

							if (g_componentType == BASEAPP_TYPE)
							{
								flags &= ~ENTITY_BASE_DATA_FLAGS;
								flags &= ~ED_FLAG_BASE_AND_CLIENT;
							}
							else if (g_componentType == CELLAPP_TYPE)
							{
								flags &= ~ENTITY_CELL_DATA_FLAGS;
								flags &= ~(ED_FLAG_ALL_CLIENTS | ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OTHER_CLIENTS | ED_FLAG_OWN_CLIENT);
							}
							else
							{
								flags &= ~ENTITY_CLIENT_DATA_FLAGS;
							}

							compPropertyInter->second->setFlags(flags);
							compPropertyInter->second->decRef();

							propertyDescrs.erase(compPropertyInter++);
							continue;
						}
					}

					compPropertyInter++;
				}
			}

			PyErr_Clear();

			// 必须在这里才设置， 在这之前设置会导致isLoadScriptModule失效，从而没有错误输出
			setScriptModuleHasComponentEntity(pScriptModule, false);
			continue;
		}

		setScriptModuleHasComponentEntity(pScriptModule, true);

		{
			std::vector<ScriptDefModulePtr>::iterator entityScriptModuleIter = EntityDef::__scriptModules.begin();
			for (; entityScriptModuleIter != EntityDef::__scriptModules.end(); ++entityScriptModuleIter)
			{
				std::vector<PropertyDescription*>& componentPropertys = g_logComponentPropertys[(*entityScriptModuleIter)->getName()];
				std::vector<PropertyDescription*>::iterator componentPropertysIter = componentPropertys.begin();
				for (; componentPropertysIter != componentPropertys.end(); ++componentPropertysIter)
				{
					PropertyDescription* pComponentPropertyDescription = (*componentPropertysIter);
					ScriptDefModule* pCompScriptModule = static_cast<EntityComponentType*>(pComponentPropertyDescription->getDataType())->pScriptDefModule();

					if (pCompScriptModule->getName() != componentScriptName)
						continue;

					uint32 pflags = pComponentPropertyDescription->getFlags();

  					if (g_componentType == BASEAPP_TYPE)
					{
						pflags |= ENTITY_BASE_DATA_FLAGS;

						if(pCompScriptModule->hasClient())
							pflags |= ED_FLAG_BASE_AND_CLIENT;
					}
					else if (g_componentType == CELLAPP_TYPE)
					{
						pflags |= ENTITY_CELL_DATA_FLAGS;

						if (pCompScriptModule->hasClient())
							pflags |= (ED_FLAG_ALL_CLIENTS | ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OTHER_CLIENTS | ED_FLAG_OWN_CLIENT);
					}
					else
					{
						pflags |= ENTITY_CLIENT_DATA_FLAGS;
					}

					pComponentPropertyDescription->setFlags(pflags);
					if (pComponentPropertyDescription->isPersistent() && pCompScriptModule->numPropertys() == 0)
					{
						pComponentPropertyDescription->isPersistent(false);

						if ((*entityScriptModuleIter)->findPersistentPropertyDescription(pComponentPropertyDescription->getUType()))
						{
							(*entityScriptModuleIter)->getPersistentPropertyDescriptions().erase(pComponentPropertyDescription->getName());
							(*entityScriptModuleIter)->getPersistentPropertyDescriptions_uidmap().erase(pComponentPropertyDescription->getUType());
						}
					}

					if ((*entityScriptModuleIter)->findPropertyDescription(pComponentPropertyDescription->getName(), g_componentType) != pComponentPropertyDescription)
					{
						(*entityScriptModuleIter)->addPropertyDescription(pComponentPropertyDescription->getName(), pComponentPropertyDescription, g_componentType, true);
					}
				}
			}
		}

		PyObject* pyClass =
			PyObject_GetAttrString(pyModule, const_cast<char *>(componentScriptName.c_str()));

		if (pyClass == NULL)
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: Could not find ComponentClass[{}]\n",
				componentScriptName.c_str()));

			return false;
		}
		else
		{
			std::string typeNames = "";
			bool valid = false;
			std::vector<PyTypeObject*>::iterator iter = scriptBaseTypes.begin();
			for (; iter != scriptBaseTypes.end(); ++iter)
			{
				if (!PyObject_IsSubclass(pyClass, (PyObject *)(*iter)))
				{
					typeNames += "'";
					typeNames += (*iter)->tp_name;
					typeNames += "'";
				}
				else
				{
					valid = true;
					break;
				}
			}

			if (!valid)
			{
				ERROR_MSG(fmt::format("EntityDef::initialize: ComponentClass {} is not derived from KBEngine.[{}]\n",
					componentScriptName.c_str(), typeNames.c_str()));

				return false;
			}
		}

		if (!PyType_Check(pyClass))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: ComponentClass[{}] is valid!\n",
				componentScriptName.c_str()));

			return false;
		}

		if (!checkDefMethod(pScriptModule, pyClass, componentScriptName))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: ComponentClass[{}] checkDefMethod is failed!\n",
				componentScriptName.c_str()));

			return false;
		}

		DEBUG_MSG(fmt::format("loaded component-script:{}({}).\n", componentScriptName.c_str(),
			pScriptModule->getUType()));

		pScriptModule->setScriptType((PyTypeObject *)pyClass);
		S_RELEASE(pyModule);
	}

	g_logComponentPropertys.clear();
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadAllEntityScriptModules(std::string entitiesPath,
									std::vector<PyTypeObject*>& scriptBaseTypes)
{
	if (!loadAllComponentScriptModules(entitiesPath, scriptBaseTypes))
		return false;

	std::string entitiesFile = entitiesPath + "entities.xml";

	SmartPointer<XML> xml(new XML());
	if(!xml->openSection(entitiesFile.c_str()))
		return false;

	TiXmlNode* node = xml->getRootNode();
	if(node == NULL)
		return true;

	// 所有需要加载脚本的组件类别名称
	std::set<std::string> checkedComponentTypes;

	XML_FOR_BEGIN(node)
	{
		std::string moduleName = xml.get()->getKey(node);
		ScriptDefModule* pScriptModule = findScriptModule(moduleName.c_str());

		PyObject* pyModule = loadScriptModule(moduleName);
		if (pyModule == NULL)
		{
			// 是否加载这个模块 （取决于是否在def文件中定义了与当前组件相关的方法或者属性）
			if(isLoadScriptModule(pScriptModule))
			{
				ERROR_MSG(fmt::format("EntityDef::initialize: Could not load EntityModule[{}]\n", 
					moduleName.c_str()));

				PyErr_Print();
				return false;
			}

			PyErr_Clear();

			// 必须在这里才设置， 在这之前设置会导致isLoadScriptModule失效，从而没有错误输出
			setScriptModuleHasComponentEntity(pScriptModule, false);
			continue;
		}

		setScriptModuleHasComponentEntity(pScriptModule, true);

		PyObject* pyClass = 
			PyObject_GetAttrString(pyModule, const_cast<char *>(moduleName.c_str()));

		if (pyClass == NULL)
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: Could not find EntityClass[{}]\n",
				moduleName.c_str()));

			return false;
		}
		else 
		{
			std::string typeNames = "";
			bool valid = false;
			std::vector<PyTypeObject*>::iterator iter = scriptBaseTypes.begin();
			for(; iter != scriptBaseTypes.end(); ++iter)
			{
				if(!PyObject_IsSubclass(pyClass, (PyObject *)(*iter)))
				{
					typeNames += "'";
					typeNames += (*iter)->tp_name;
					typeNames += "'";
				}
				else
				{
					valid = true;
					break;
				}
			}
			
			if(!valid)
			{
				ERROR_MSG(fmt::format("EntityDef::initialize: EntityClass {} is not derived from KBEngine.[{}]\n",
					moduleName.c_str(), typeNames.c_str()));

				return false;
			}
		}

		if(!PyType_Check(pyClass))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: EntityClass[{}] is valid!\n",
				moduleName.c_str()));

			return false;
		}
		
		if(!checkDefMethod(pScriptModule, pyClass, moduleName))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: EntityClass[{}] checkDefMethod is failed!\n",
				moduleName.c_str()));

			return false;
		}
		
		DEBUG_MSG(fmt::format("loaded entity-script:{}({}).\n", moduleName.c_str(), 
			pScriptModule->getUType()));

		pScriptModule->setScriptType((PyTypeObject *)pyClass);
		S_RELEASE(pyModule);

		// 查找实体在该进程上是否有相对应需要实现的实体组件，如果没有则提示错误
		const ScriptDefModule::COMPONENTDESCRIPTION_MAP& componentDescrs = pScriptModule->getComponentDescrs();
		ScriptDefModule::COMPONENTDESCRIPTION_MAP::const_iterator comp_iter = componentDescrs.begin();
		for (; comp_iter != componentDescrs.end(); ++comp_iter)
		{
			std::string componentScriptName = comp_iter->second->getName();

			std::set<std::string>::iterator fiter = checkedComponentTypes.find(componentScriptName);
			if (fiter != checkedComponentTypes.end())
				continue;

			ScriptDefModule* pComponentScriptModule = findScriptModule(componentScriptName.c_str());

			// 是否加载这个模块，如果需要加载且当前没有模块则提示错误
			if (!pComponentScriptModule->getScriptType() && isLoadScriptModule(pComponentScriptModule))
			{
				ERROR_MSG(fmt::format("EntityDef::initialize: Could not load ComponentModule[{}]\n",
					componentScriptName.c_str()));

				PyErr_Print();
				return false;
			}

			checkedComponentTypes.insert(componentScriptName);
		}
	}
	XML_FOR_END(node);

	return true;
}

//-------------------------------------------------------------------------------------
ScriptDefModule* EntityDef::findScriptModule(ENTITY_SCRIPT_UID utype, bool notFoundOutErr)
{
	// utype 最小为1
	if (utype == 0 || utype >= __scriptModules.size() + 1)
	{
		if (notFoundOutErr)
		{
			ERROR_MSG(fmt::format("EntityDef::findScriptModule: is not exist(utype:{})!\n", utype));
		}

		return NULL;
	}

	return __scriptModules[utype - 1].get();
}

//-------------------------------------------------------------------------------------
ScriptDefModule* EntityDef::findScriptModule(const char* scriptName, bool notFoundOutErr)
{
	std::map<std::string, ENTITY_SCRIPT_UID>::iterator iter = 
		__scriptTypeMappingUType.find(scriptName);

	if(iter == __scriptTypeMappingUType.end())
	{
		if (notFoundOutErr)
		{
			ERROR_MSG(fmt::format("EntityDef::findScriptModule: [{}] not found!\n", scriptName));
		}

		return NULL;
	}

	return findScriptModule(iter->second);
}

//-------------------------------------------------------------------------------------
ScriptDefModule* EntityDef::findOldScriptModule(const char* scriptName, bool notFoundOutErr)
{
	std::map<std::string, ENTITY_SCRIPT_UID>::iterator iter = 
		__oldScriptTypeMappingUType.find(scriptName);

	if(iter == __oldScriptTypeMappingUType.end())
	{
		if (notFoundOutErr)
		{
			ERROR_MSG(fmt::format("EntityDef::findOldScriptModule: [{}] not found!\n", scriptName));
		}

		return NULL;
	}

	if (iter->second >= __oldScriptModules.size() + 1)
	{
		if (notFoundOutErr)
		{
			ERROR_MSG(fmt::format("EntityDef::findOldScriptModule: is not exist(utype:{})!\n", iter->second));
		}

		return NULL;
	}

	return __oldScriptModules[iter->second - 1].get();

}

//-------------------------------------------------------------------------------------
bool EntityDef::installScript(PyObject* mod)
{
	if(_isInit)
		return true;

	script::PyMemoryStream::installScript(NULL);
	APPEND_SCRIPT_MODULE_METHOD(mod, MemoryStream, script::PyMemoryStream::py_new, METH_VARARGS, 0);

	EntityCall::installScript(NULL);
	EntityComponentCall::installScript(NULL);
	FixedArray::installScript(NULL);
	FixedDict::installScript(NULL);
	VolatileInfo::installScript(NULL);

	_isInit = true;
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::uninstallScript()
{
	if(_isInit)
	{
		script::PyMemoryStream::uninstallScript();
		EntityCall::uninstallScript();
		EntityComponentCall::uninstallScript();
		FixedArray::uninstallScript();
		FixedDict::uninstallScript();
		VolatileInfo::uninstallScript();
	}

	return EntityDef::finalise();
}

//-------------------------------------------------------------------------------------
bool EntityDef::initializeWatcher()
{
	return true;
}

//-------------------------------------------------------------------------------------
}
