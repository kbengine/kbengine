﻿/*
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


#include "entitydef.h"
#include "scriptdef_module.h"
#include "datatypes.h"
#include "common.h"
#include "blob.h"
#include "resmgr/resmgr.h"
#include "common/smartpointer.h"
#include "entitydef/entity_mailbox.h"

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

// 方法产生时自动产生utype用的
ENTITY_METHOD_UID g_methodUtypeAuto = 1;
std::vector<ENTITY_METHOD_UID> g_methodCusUtypes;																									

ENTITY_PROPERTY_UID auto_puid = 1;
static ENTITY_PROPERTY_UID auto_puid_end = 20000;  ///< 显示制定的属性id必要大于这个值
std::vector<ENTITY_PROPERTY_UID> puids;  ///< 所有显示制定属性的id们

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

	auto_puid = 1;
	puids.clear();

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

	EntityDef::__scriptModules.clear();
	EntityDef::__scriptTypeMappingUType.clear();
	g_methodCusUtypes.clear();
	DataType::finalise();
	DataTypes::finalise();
	return true;
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
		KBE_ASSERT(ret && "EntityDef::reload: finalise is error!");

		ret = initialize(EntityDef::__scriptBaseTypes, EntityDef::__loadComponentType);
		KBE_ASSERT(ret && "EntityDef::reload: initialize is error!");
	}
	else
	{
		loadAllScriptModules(EntityDef::__entitiesPath, EntityDef::__scriptBaseTypes);
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

	g_entityFlagMapping["CELL_PUBLIC"]							= ED_FLAG_CELL_PUBLIC;
	g_entityFlagMapping["CELL_PRIVATE"]							= ED_FLAG_CELL_PRIVATE;
	g_entityFlagMapping["ALL_CLIENTS"]							= ED_FLAG_ALL_CLIENTS;
	g_entityFlagMapping["CELL_PUBLIC_AND_OWN"]					= ED_FLAG_CELL_PUBLIC_AND_OWN;
	g_entityFlagMapping["BASE_AND_CLIENT"]						= ED_FLAG_BASE_AND_CLIENT;
	g_entityFlagMapping["BASE"]									= ED_FLAG_BASE;
	g_entityFlagMapping["OTHER_CLIENTS"]						= ED_FLAG_OTHER_CLIENTS;
	g_entityFlagMapping["OWN_CLIENT"]							= ED_FLAG_OWN_CLIENT;

	std::string entitiesFile = __entitiesPath + "entities.xml";
	std::string defFilePath = __entitiesPath + "entity_defs/";
	ENTITY_SCRIPT_UID utype = 1;
	
	// 初始化数据类别
	// assets/scripts/entity_defs/alias.xml
	if(!DataTypes::initialize(defFilePath + "alias.xml"))
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
	{ // Account/Avatar/Spaces...
		std::string moduleName = xml.get()->getKey(node);
		__scriptTypeMappingUType[moduleName] = utype;
		ScriptDefModule* scriptModule = new ScriptDefModule(moduleName, utype++);
		EntityDef::__scriptModules.push_back(scriptModule);

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
		if(!loadDefInfo(defFilePath, moduleName, defxml.get(), defNode, scriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: failed to load entity:{} parentClass.\n",
				moduleName.c_str()));

			return false;
		}
		
		// 尝试在主entity文件中加载detailLevel数据
		if(!loadDetailLevelInfo(defFilePath, moduleName, defxml.get(), defNode, scriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: failed to load entity:{} DetailLevelInfo.\n",
				moduleName.c_str()));

			return false;
		}

		scriptModule->onLoaded();
	}
	XML_FOR_END(node);

	EntityDef::md5().final();

	if(loadComponentType == DBMGR_TYPE)
		return true;

	return loadAllScriptModules(__entitiesPath, scriptBaseTypes) && initializeWatcher();
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDefInfo(const std::string& defFilePath, 
							const std::string& moduleName, 
							XML* defxml, 
							TiXmlNode* defNode, 
							ScriptDefModule* scriptModule)
{
	if(!loadAllDefDescriptions(moduleName, defxml, defNode, scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to loadAllDefDescription(), entity:{}\n",
			moduleName.c_str()));

		return false;
	}
	
	// 遍历所有的interface， 并将他们的方法和属性加入到模块中
	if(!loadInterfaces(defFilePath, moduleName, defxml, defNode, scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} interface.\n",
			moduleName.c_str()));

		return false;
	}
	
	// 加载父类所有的内容
	if(!loadParentClass(defFilePath, moduleName, defxml, defNode, scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} parentClass.\n",
			moduleName.c_str()));

		return false;
	}

	// 尝试加载detailLevel数据
	if(!loadDetailLevelInfo(defFilePath, moduleName, defxml, defNode, scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} DetailLevelInfo.\n",
			moduleName.c_str()));

		return false;
	}

	// 尝试加载VolatileInfo数据
	if(!loadVolatileInfo(defFilePath, moduleName, defxml, defNode, scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} VolatileInfo.\n",
			moduleName.c_str()));

		return false;
	}
	
	scriptModule->autoMatchCompOwn();
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDetailLevelInfo(const std::string& defFilePath, 
									const std::string& moduleName, 
									XML* defxml, 
									TiXmlNode* defNode, 
									ScriptDefModule* scriptModule)
{
	TiXmlNode* detailLevelNode = defxml->enterNode(defNode, "DetailLevels");
	if(detailLevelNode == NULL)
		return true;

	DetailLevel& dlInfo = scriptModule->getDetailLevel();
	
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
									ScriptDefModule* scriptModule)
{
	TiXmlNode* pNode = defxml->enterNode(defNode, "Volatile");
	if(pNode == NULL)
		return true;

	VolatileInfo& vInfo = scriptModule->getVolatileInfo();
	
	TiXmlNode* node = defxml->enterNode(pNode, "position");
	if(node) 
	{
		vInfo.position((float)defxml->getValFloat(node));
	}
	else
	{
		if(defxml->hasNode(pNode, "position"))
			vInfo.position(VolatileInfo::ALWAYS);
		else
			vInfo.position(-1.f);
	}

	node = defxml->enterNode(pNode, "yaw");
	if(node) 
	{
		vInfo.yaw((float)defxml->getValFloat(node));
	}
	else
	{
		if(defxml->hasNode(pNode, "yaw"))
			vInfo.yaw(VolatileInfo::ALWAYS);
		else
			vInfo.yaw(-1.f);
	}

	node = defxml->enterNode(pNode, "pitch");
	if(node) 
	{
		vInfo.pitch((float)defxml->getValFloat(node));
	}
	else
	{
		if(defxml->hasNode(pNode, "pitch"))
			vInfo.pitch(VolatileInfo::ALWAYS);
		else
			vInfo.pitch(-1.f);
	}

	node = defxml->enterNode(pNode, "roll");
	if(node) 
	{
		vInfo.roll((float)defxml->getValFloat(node));
	}
	else
	{
		if(defxml->hasNode(pNode, "roll"))
			vInfo.roll(VolatileInfo::ALWAYS);
		else
			vInfo.roll(-1.f);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadInterfaces(const std::string& defFilePath, 
							   const std::string& moduleName, 
							   XML* defxml, 
							   TiXmlNode* defNode, 
							   ScriptDefModule* scriptModule)
{
	TiXmlNode* implementsNode = defxml->enterNode(defNode, "Implements");
	if(implementsNode == NULL)
		return true;

	XML_FOR_BEGIN(implementsNode)
	{
		TiXmlNode* interfaceNode = defxml->enterNode(implementsNode, "Interface");
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

		if(!loadAllDefDescriptions(moduleName, interfaceXml.get(), interfaceRootNode, scriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: interface[{}] is error!\n", 
				interfaceName.c_str()));

			return false;
		}

		// 尝试加载detailLevel数据
		if(!loadDetailLevelInfo(defFilePath, moduleName, interfaceXml.get(), interfaceRootNode, scriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::loadInterfaces: failed to load entity:{} DetailLevelInfo.\n",
				moduleName.c_str()));

			return false;
		}

		// 遍历所有的interface， 并将他们的方法和属性加入到模块中
		if(!loadInterfaces(defFilePath, moduleName, interfaceXml.get(), interfaceRootNode, scriptModule))
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
bool EntityDef::loadParentClass(const std::string& defFilePath, 
								const std::string& moduleName, 
								XML* defxml, 
								TiXmlNode* defNode, 
								ScriptDefModule* scriptModule)
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
	if(!loadDefInfo(defFilePath, parentClassName, parentClassXml.get(), parentClassdefNode, scriptModule))
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
									  ScriptDefModule* scriptModule)
{
	// 加载属性描述
	if(!loadDefPropertys(moduleName, defxml, defxml->enterNode(defNode, "Properties"), scriptModule))
		return false;
	
	// 加载cell方法描述
	if(!loadDefCellMethods(moduleName, defxml, defxml->enterNode(defNode, "CellMethods"), scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadAllDefDescription:loadDefCellMethods[{}] is failed!\n",
			moduleName.c_str()));

		return false;
	}

	// 加载base方法描述
	if(!loadDefBaseMethods(moduleName, defxml, defxml->enterNode(defNode, "BaseMethods"), scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadAllDefDescription:loadDefBaseMethods[{}] is failed!\n",
			moduleName.c_str()));

		return false;
	}

	// 加载client方法描述
	if(!loadDefClientMethods(moduleName, defxml, defxml->enterNode(defNode, "ClientMethods"), scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadAllDefDescription:loadDefClientMethods[{}] is failed!\n",
			moduleName.c_str()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::validDefPropertyName(ScriptDefModule* scriptModule, const std::string& name)
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

	return true;
}

bool EntityDef::loadDefPropertys(const std::string& moduleName, 
								 XML* xml, 
								 TiXmlNode* defPropertyNode, 
								 ScriptDefModule* scriptModule)
{
	if(defPropertyNode)
	{
		XML_FOR_BEGIN(defPropertyNode)
		{ // 迭代Properties下的属性节点
			ENTITY_PROPERTY_UID			futype = 0;  ///< 属性id, Utype节点值
			uint32						flags = 0;   ///< Flags节点值
			int32						hasBaseFlags = 0;  ///< Flags是否包含BASE
			int32						hasCellFlags = 0;  ///< Flags是否包含CELL
			int32						hasClientFlags = 0;  ///< Flags是否包含CLIENT 
			DataType*					dataType = NULL;  ///< Type节点值
			bool						isPersistent = false;  ///< Persistent节点值
			bool						isIdentifier = false;		// 是否是一个索引键
			uint32						databaseLength = 0;			// 这个属性在数据库中的长度
			std::string					indexType;
			DETAIL_TYPE					detailLevel = DETAIL_LEVEL_FAR;  ///< DetailLevel节点值
			std::string					detailLevelStr = "";
			std::string					strType;
			std::string					strisPersistent;
			std::string					strFlags;
			std::string					strIdentifierNode;
			std::string					defaultStr;  ///< Default节点值
			std::string					name = "";  ///< 属性节点名，如: characters, lastSelCharacter

			name = xml->getKey(defPropertyNode);
			if(!validDefPropertyName(scriptModule, name))
			{
				ERROR_MSG(fmt::format("EntityDef::loadDefPropertys: '{}' is limited, in module({})!\n", 
					name, moduleName));

				return false;
			}

			// 检视Flags标签值，检视结果置入flags，且检视过程中操作scriptModule->setBase/Cell/Client(true)
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
					scriptModule->setBase(true);

				hasCellFlags = flags & ENTITY_CELL_DATA_FLAGS;
				if(hasCellFlags > 0)
					scriptModule->setCell(true);

				hasClientFlags = flags & ENTITY_CLIENT_DATA_FLAGS;
				if(hasClientFlags > 0)
					scriptModule->setClient(true);

				if(hasBaseFlags <= 0 && hasCellFlags <= 0)
				{
					ERROR_MSG(fmt::format("EntityDef::loadDefPropertys: not fount flags[{}], is {}.{}!\n",
						strFlags.c_str(), moduleName, name.c_str()));

					return false;
				}
			}

			// 检视Persistent标签值,检视结果置入isPersistent
			TiXmlNode* persistentNode = xml->enterNode(defPropertyNode->FirstChild(), "Persistent");
			if(persistentNode)
			{
				strisPersistent = xml->getValStr(persistentNode);

				std::transform(strisPersistent.begin(), strisPersistent.end(), 
					strisPersistent.begin(), tolower);

				if(strisPersistent == "true")
					isPersistent = true;
			}

			// 检视Type标签值，检视结果置入dataType
			TiXmlNode* typeNode = xml->enterNode(defPropertyNode->FirstChild(), "Type");
			if(typeNode)
			{
				strType = xml->getValStr(typeNode);

				if(strType == "ARRAY")
				{
					FixedArrayType* dataType1 = new FixedArrayType();
					if(dataType1->initialize(xml, typeNode))
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

			// 检视Index标签，检视结果置入indexType
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

			if(utypeValNode)
			{
				int iUtype = xml->getValInt(utypeValNode);
                KBE_ASSERT(iUtype > auto_puid_end)
				futype = iUtype;

				if (iUtype != int(futype))
				{
					ERROR_MSG(fmt::format("EntityDef::loadDefPropertys: 'Utype' has overflowed({} > 65535), is {}.{}!\n",
						iUtype, moduleName, name.c_str()));

					return false;
				}

				puids.push_back(futype);
			}
			else
			{
				while(true)
				{
                    // 自动属性id分配耗竭
                    if(auto_puid > auto_puid_end)
                    {
                        ERROR_MSG(fmt::format("auto_puid > auto_puid_end: {}.\n", auto_puid_end));
                        return (false);
                    }
					futype = auto_puid++;
					std::vector<ENTITY_PROPERTY_UID>::iterator iter = 
								std::find(puids.begin(), puids.end(), futype);

					if(iter == puids.end())
						break;
				}
			}

			// 产生一个属性描述实例
			PropertyDescription* propertyDescription = PropertyDescription::createDescription(futype, strType, 
															name, flags, isPersistent, 
															dataType, isIdentifier, indexType,
															databaseLength, defaultStr, 
															detailLevel);

			bool ret = true;

			// 添加到模块中
			if(hasCellFlags > 0)
				ret = scriptModule->addPropertyDescription(name.c_str(), 
						propertyDescription, CELLAPP_TYPE);

			if(hasBaseFlags > 0)
				ret = scriptModule->addPropertyDescription(name.c_str(), 
						propertyDescription, BASEAPP_TYPE);

			if(hasClientFlags > 0)
				ret = scriptModule->addPropertyDescription(name.c_str(), 
						propertyDescription, CLIENT_TYPE);

			if(!ret)
			{
				ERROR_MSG(fmt::format("EntityDef::addPropertyDescription({}): {}.\n", 
					moduleName.c_str(), xml->getTxdoc()->Value()));
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
								   ScriptDefModule* scriptModule)
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
							if(dataType1->initialize(xml, typeNode))
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
			}

			scriptModule->addCellMethodDescription(name.c_str(), methodDescription);
		}
		XML_FOR_END(defMethodNode);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDefBaseMethods(const std::string& moduleName, XML* xml, 
								   TiXmlNode* defMethodNode, ScriptDefModule* scriptModule)
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
							if(dataType1->initialize(xml, typeNode))
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
			}

			scriptModule->addBaseMethodDescription(name.c_str(), methodDescription);
		}
		XML_FOR_END(defMethodNode);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDefClientMethods(const std::string& moduleName, XML* xml, 
									 TiXmlNode* defMethodNode, ScriptDefModule* scriptModule)
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
							if(dataType1->initialize(xml, typeNode))
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
			}

			scriptModule->addClientMethodDescription(name.c_str(), methodDescription);
		}
		XML_FOR_END(defMethodNode);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::isLoadScriptModule(ScriptDefModule* scriptModule)
{
	switch(__loadComponentType)
	{
	case BASEAPP_TYPE:
		{
			if(!scriptModule->hasBase())
				return false;

			break;
		}
	case CELLAPP_TYPE:
		{
			if(!scriptModule->hasCell())
				return false;

			break;
		}
	case CLIENT_TYPE:
	case BOTS_TYPE:
		{
			if(!scriptModule->hasClient())
				return false;

			break;
		}
	default:
		{
			if(!scriptModule->hasCell())
				return false;

			break;
		}
	};

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::checkDefMethod(ScriptDefModule* scriptModule, 
							   PyObject* moduleObj, const std::string& moduleName)
{
	ScriptDefModule::METHODDESCRIPTION_MAP* methodDescrsPtr = NULL;
	
	switch(__loadComponentType)
	{
	case BASEAPP_TYPE:
		methodDescrsPtr = 
			(ScriptDefModule::METHODDESCRIPTION_MAP*)&scriptModule->getBaseMethodDescriptions();
		break;
	case CELLAPP_TYPE:
		methodDescrsPtr = 
			(ScriptDefModule::METHODDESCRIPTION_MAP*)&scriptModule->getCellMethodDescriptions();
		break;
	case CLIENT_TYPE:
	case BOTS_TYPE:
		methodDescrsPtr = 
			(ScriptDefModule::METHODDESCRIPTION_MAP*)&scriptModule->getClientMethodDescriptions();
		break;
	default:
		methodDescrsPtr = 
			(ScriptDefModule::METHODDESCRIPTION_MAP*)&scriptModule->getCellMethodDescriptions();
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
			ERROR_MSG(fmt::format("EntityDef::checkDefMethod:class {} does not have method[{}].\n",
					moduleName.c_str(), iter->first.c_str()));

			return false;
		}
	}
	
	return true;	
}

//-------------------------------------------------------------------------------------
void EntityDef::setScriptModuleHasComponentEntity(ScriptDefModule* scriptModule, 
												  bool has)
{
	switch(__loadComponentType)
	{
	case BASEAPP_TYPE:
		scriptModule->setBase(has);
		return;
	case CELLAPP_TYPE:
		scriptModule->setCell(has);
		return;
	case CLIENT_TYPE:
	case BOTS_TYPE:
		scriptModule->setClient(has);
		return;
	default:
		scriptModule->setCell(has);
		return;
	};
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadAllScriptModules(std::string entitiesPath, 
									std::vector<PyTypeObject*>& scriptBaseTypes)
{
	std::string entitiesFile = entitiesPath + "entities.xml";

	SmartPointer<XML> xml(new XML());
	if(!xml->openSection(entitiesFile.c_str()))
		return false;

	TiXmlNode* node = xml->getRootNode();
	if(node == NULL)
		return true;

	XML_FOR_BEGIN(node)
	{
		std::string moduleName = xml.get()->getKey(node);
		ScriptDefModule* scriptModule = findScriptModule(moduleName.c_str());

		PyObject* pyModule = 
			PyImport_ImportModule(const_cast<char*>(moduleName.c_str()));

		if(g_isReload)
			pyModule = PyImport_ReloadModule(pyModule);

		if (pyModule == NULL)
		{
			// 是否加载这个模块 （取决于是否在def文件中定义了与当前组件相关的方法或者属性）
			if(isLoadScriptModule(scriptModule))
			{
				ERROR_MSG(fmt::format("EntityDef::initialize:Could not load module[{}]\n", 
					moduleName.c_str()));

				PyErr_Print();
				return false;
			}

			PyErr_Clear();

			// 必须在这里才设置， 在这之前设置会导致isLoadScriptModule失效，从而没有错误输出
			setScriptModuleHasComponentEntity(scriptModule, false);
			continue;
		}

		setScriptModuleHasComponentEntity(scriptModule, true);

		PyObject* pyClass = 
			PyObject_GetAttrString(pyModule, const_cast<char *>(moduleName.c_str()));

		if (pyClass == NULL)
		{
			ERROR_MSG(fmt::format("EntityDef::initialize:Could not find class[{}]\n",
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
				ERROR_MSG(fmt::format("EntityDef::initialize:Class {} is not derived from KBEngine.[{}]\n",
					moduleName.c_str(), typeNames.c_str()));

				return false;
			}
		}

		if(!PyType_Check(pyClass))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize:class[{}] is valid!\n",
				moduleName.c_str()));

			return false;
		}
		
		if(!checkDefMethod(scriptModule, pyClass, moduleName))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize:class[{}] checkDefMethod is failed!\n",
				moduleName.c_str()));

			return false;
		}
		
		DEBUG_MSG(fmt::format("loaded script:{}({}).\n", moduleName.c_str(), 
			scriptModule->getUType()));

		scriptModule->setScriptType((PyTypeObject *)pyClass);
		S_RELEASE(pyModule);
	}
	XML_FOR_END(node);

	return true;
}

//-------------------------------------------------------------------------------------
ScriptDefModule* EntityDef::findScriptModule(ENTITY_SCRIPT_UID utype)
{
	if (utype >= __scriptModules.size() + 1)
	{
		ERROR_MSG(fmt::format("EntityDef::findScriptModule: is not exist(utype:{})!\n", utype));
		return NULL;
	}

	return __scriptModules[utype - 1].get();
}

//-------------------------------------------------------------------------------------
ScriptDefModule* EntityDef::findScriptModule(const char* scriptName)
{
	std::map<std::string, ENTITY_SCRIPT_UID>::iterator iter = 
		__scriptTypeMappingUType.find(scriptName);

	if(iter == __scriptTypeMappingUType.end())
	{
		ERROR_MSG(fmt::format("EntityDef::findScriptModule: [{}] not found!\n", scriptName));
		return NULL;
	}

	return findScriptModule(iter->second);
}

//-------------------------------------------------------------------------------------
ScriptDefModule* EntityDef::findOldScriptModule(const char* scriptName)
{
	std::map<std::string, ENTITY_SCRIPT_UID>::iterator iter = 
		__oldScriptTypeMappingUType.find(scriptName);

	if(iter == __oldScriptTypeMappingUType.end())
	{
		ERROR_MSG(fmt::format("EntityDef::findOldScriptModule: [{}] not found!\n", scriptName));
		return NULL;
	}

	if (iter->second >= __oldScriptModules.size() + 1)
	{
		ERROR_MSG(fmt::format("EntityDef::findOldScriptModule: is not exist(utype:{})!\n", iter->second));
		return NULL;
	}

	return __oldScriptModules[iter->second - 1].get();

}

//-------------------------------------------------------------------------------------
bool EntityDef::installScript(PyObject* mod)
{
	if(_isInit)
		return true;

	Blob::installScript(NULL);
	APPEND_SCRIPT_MODULE_METHOD(mod, Blob, Blob::py_new, METH_VARARGS, 0);

	EntityMailbox::installScript(NULL);
	FixedArray::installScript(NULL);
	FixedDict::installScript(NULL);

	_isInit = true;
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::uninstallScript()
{
	if(_isInit)
	{
		Blob::uninstallScript();
		EntityMailbox::uninstallScript();
		FixedArray::uninstallScript();
		FixedDict::uninstallScript();
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
