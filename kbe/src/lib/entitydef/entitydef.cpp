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


#include "entitydef.hpp"
#include "scriptdef_module.hpp"
#include "datatypes.hpp"
#include "common.hpp"
#include "blob.hpp"
#include "resmgr/resmgr.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "entitydef/entity_mailbox.hpp"

#ifndef CODE_INLINE
#include "entitydef.ipp"
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

// ��������ʱ�Զ�����utype�õ�
ENTITY_METHOD_UID g_methodUtypeAuto = 1;
std::vector<ENTITY_METHOD_UID> g_methodCusUtypes;																									

ENTITY_PROPERTY_UID auto_puid = 1;
std::vector<ENTITY_PROPERTY_UID> puids;

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
	PropertyDescription::propertyDescriptionCount_ = 0;
	MethodDescription::methodDescriptionCount_ = 0;

	EntityDef::__md5.clear();
	g_methodUtypeAuto = 1;
	EntityDef::_isInit = false;

	auto_puid = 1;
	puids.clear();

	if(!isReload)
	{
		std::vector<ScriptDefModulePtr>::iterator iter = EntityDef::__scriptModules.begin();
		for(; iter != EntityDef::__scriptModules.end(); iter++)
		{
			(*iter)->finalise();
		}

		iter = EntityDef::__oldScriptModules.begin();
		for(; iter != EntityDef::__oldScriptModules.end(); iter++)
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
		for(; iter != EntityDef::__scriptModules.end(); iter++)
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
		loadAllScriptModule(EntityDef::__entitiesPath, EntityDef::__scriptBaseTypes);
	}

	EntityDef::_isInit = true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::initialize(std::vector<PyTypeObject*>& scriptBaseTypes, 
						   COMPONENT_TYPE loadComponentType)
{
	__loadComponentType = loadComponentType;
	__scriptBaseTypes = scriptBaseTypes;

	__entitiesPath = Resmgr::getSingleton().getPyUserResPath() + "scripts/";

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
	
	// ��ʼ���������
	// demo/res/scripts/entity_defs/alias.xml
	if(!DataTypes::initialize(defFilePath + "alias.xml"))
		return false;

	// �����entities.xml�ļ�
	SmartPointer<XmlPlus> xml(new XmlPlus());
	if(!xml.get()->openSection(entitiesFile.c_str()) && xml.get()->getRootElement() == NULL)
		return false;
	
	// ���entities.xml���ڵ�, ���û�ж���һ��entity��ôֱ�ӷ���true
	TiXmlNode* node = xml.get()->getRootNode();
	if(node == NULL)
		return true;

	// ��ʼ�������е�entity�ڵ�
	XML_FOR_BEGIN(node)
	{
		std::string moduleName = xml.get()->getKey(node);
		__scriptTypeMappingUType[moduleName] = utype;
		ScriptDefModule* scriptModule = new ScriptDefModule(moduleName);
		scriptModule->setUType(utype++);
		EntityDef::__scriptModules.push_back(scriptModule);

		std::string deffile = defFilePath + moduleName + ".def";
		SmartPointer<XmlPlus> defxml(new XmlPlus());

		if(!defxml.get()->openSection(deffile.c_str()))
			return false;

		TiXmlNode* defNode = defxml.get()->getRootNode();

		// ����def�ļ��еĶ���
		if(!loadDefInfo(defFilePath, moduleName, defxml.get(), defNode, scriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: failed to load entity:{} parentClass.\n",
				moduleName.c_str()));

			return false;
		}
		
		// ��������entity�ļ��м���detailLevel����
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

	return loadAllScriptModule(__entitiesPath, scriptBaseTypes) && initializeWatcher();
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDefInfo(const std::string& defFilePath, 
							const std::string& moduleName, 
							XmlPlus* defxml, 
							TiXmlNode* defNode, 
							ScriptDefModule* scriptModule)
{
	if(!loadAllDefDescription(moduleName, defxml, defNode, scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to loadAllDefDescription(), entity:{}\n",
			moduleName.c_str()));

		return false;
	}
	
	// �������е�interface�� �������ǵķ��������Լ��뵽ģ����
	if(!loadInterfaces(defFilePath, moduleName, defxml, defNode, scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} interface.\n",
			moduleName.c_str()));

		return false;
	}
	
	// ���ظ������е�����
	if(!loadParentClass(defFilePath, moduleName, defxml, defNode, scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} parentClass.\n",
			moduleName.c_str()));

		return false;
	}

	// ���Լ���detailLevel����
	if(!loadDetailLevelInfo(defFilePath, moduleName, defxml, defNode, scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadDefInfo: failed to load entity:{} DetailLevelInfo.\n",
			moduleName.c_str()));

		return false;
	}

	// ���Լ���VolatileInfo����
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
									XmlPlus* defxml, 
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
									XmlPlus* defxml, 
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
							   XmlPlus* defxml, 
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
		SmartPointer<XmlPlus> interfaceXml(new XmlPlus());
		if(!interfaceXml.get()->openSection(interfacefile.c_str()))
			return false;

		TiXmlNode* interfaceRootNode = interfaceXml.get()->getRootNode();
		if(!loadAllDefDescription(moduleName, interfaceXml.get(), interfaceRootNode, scriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::initialize: interface[{}] is error!\n", 
				interfaceName.c_str()));

			return false;
		}

		// ���Լ���detailLevel����
		if(!loadDetailLevelInfo(defFilePath, moduleName, interfaceXml.get(), interfaceRootNode, scriptModule))
		{
			ERROR_MSG(fmt::format("EntityDef::loadInterfaces: failed to load entity:{} DetailLevelInfo.\n",
				moduleName.c_str()));

			return false;
		}

		// �������е�interface�� �������ǵķ��������Լ��뵽ģ����
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
								XmlPlus* defxml, 
								TiXmlNode* defNode, 
								ScriptDefModule* scriptModule)
{
	TiXmlNode* parentClassNode = defxml->enterNode(defNode, "Parent");
	if(parentClassNode == NULL)
		return true;
	
	std::string parentClassName = defxml->getKey(parentClassNode);
	std::string parentClassfile = defFilePath + parentClassName + ".def";
	
	SmartPointer<XmlPlus> parentClassXml(new XmlPlus());
	if(!parentClassXml.get()->openSection(parentClassfile.c_str()))
		return false;
	
	TiXmlNode* parentClassdefNode = parentClassXml.get()->getRootNode();

	// ����def�ļ��еĶ���
	if(!loadDefInfo(defFilePath, parentClassName, parentClassXml.get(), parentClassdefNode, scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadParentClass: failed to load entity:{} parentClass.\n",
			moduleName.c_str()));

		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadAllDefDescription(const std::string& moduleName, 
									  XmlPlus* defxml, 
									  TiXmlNode* defNode, 
									  ScriptDefModule* scriptModule)
{
	// ������������
	if(!loadDefPropertys(moduleName, defxml, defxml->enterNode(defNode, "Properties"), scriptModule))
		return false;
	
	// ����cell��������
	if(!loadDefCellMethods(moduleName, defxml, defxml->enterNode(defNode, "CellMethods"), scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadAllDefDescription:loadDefCellMethods[{}] is failed!\n",
			moduleName.c_str()));

		return false;
	}

	// ����base��������
	if(!loadDefBaseMethods(moduleName, defxml, defxml->enterNode(defNode, "BaseMethods"), scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadAllDefDescription:loadDefBaseMethods[{}] is failed!\n",
			moduleName.c_str()));

		return false;
	}

	// ����client��������
	if(!loadDefClientMethods(moduleName, defxml, defxml->enterNode(defNode, "ClientMethods"), scriptModule))
	{
		ERROR_MSG(fmt::format("EntityDef::loadAllDefDescription:loadDefClientMethods[{}] is failed!\n",
			moduleName.c_str()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityDef::loadDefPropertys(const std::string& moduleName, 
								 XmlPlus* xml, 
								 TiXmlNode* defPropertyNode, 
								 ScriptDefModule* scriptModule)
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
			bool						isIdentifier = false;													// �Ƿ���һ��������
			uint32						databaseLength = 0;														// ������������ݿ��еĳ���
			DETAIL_TYPE					detailLevel = DETAIL_LEVEL_FAR;
			std::string					detailLevelStr = "";
			std::string					strType;
			std::string					strisPersistent;
			std::string					strFlags;
			std::string					strIdentifierNode;
			std::string					defaultStr;
			std::string					name = "";

			name = xml->getKey(defPropertyNode);
			TiXmlNode* flagsNode = xml->enterNode(defPropertyNode->FirstChild(), "Flags");
			if(flagsNode)
			{
				strFlags = xml->getValStr(flagsNode);
				std::transform(strFlags.begin(), strFlags.end(), strFlags.begin(), toupper);					// ת��Ϊ��д

				ENTITYFLAGMAP::iterator iter = g_entityFlagMapping.find(strFlags.c_str());
				if(iter == g_entityFlagMapping.end())
				{
					ERROR_MSG(fmt::format("EntityDef::loadDefPropertys: can't fount flags[{}] in {}.\n", 
						strFlags.c_str(), name.c_str()));

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
					ERROR_MSG(fmt::format("EntityDef::loadDefPropertys: can't fount flags[{}] in {}.\n",
						strFlags.c_str(), name.c_str()));
					return false;
				}
			}


			TiXmlNode* persistentNode = xml->enterNode(defPropertyNode->FirstChild(), "Persistent");
			if(persistentNode)
			{
				strisPersistent = xml->getValStr(persistentNode);

				std::transform(strisPersistent.begin(), strisPersistent.end(), 
					strisPersistent.begin(), tolower);				// ת��ΪСд

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
					if(dataType1->initialize(xml, typeNode))
						dataType = dataType1;
					else
						return false;
				}
				else
					dataType = DataTypes::getDataType(strType);
			}

			TiXmlNode* identifierNode = xml->enterNode(defPropertyNode->FirstChild(), "Identifier");
			if(identifierNode)
			{
				strIdentifierNode = xml->getValStr(identifierNode);
				std::transform(strIdentifierNode.begin(), strIdentifierNode.end(), 
					strIdentifierNode.begin(), tolower);			// ת��ΪСд

				if(strIdentifierNode == "true")
					isIdentifier = true;
			}

			//TiXmlNode* databaseLengthNode = xml->enterNode(defPropertyNode->FirstChild(), "Identifier");
			if(identifierNode)
			{
				databaseLength = xml->getValInt(identifierNode);
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
				futype = iUtype;
				KBE_ASSERT(iUtype == int(futype) && "EntityDef::loadDefPropertys: Utype overflow!\n");

				puids.push_back(futype);
			}
			else
			{
				while(true)
				{
					futype = auto_puid++;
					std::vector<ENTITY_PROPERTY_UID>::iterator iter = 
								std::find(puids.begin(), puids.end(), futype);

					if(iter == puids.end())
						break;
				}
			}

			// ����һ����������ʵ��
			PropertyDescription* propertyDescription = PropertyDescription::createDescription(futype, strType, 
															name, flags, isPersistent, 
															dataType, isIdentifier, 
															databaseLength, defaultStr, 
															detailLevel);

			bool ret = true;

			// ��ӵ�ģ����
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
								   XmlPlus* xml, 
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
			
			// ����û�в���
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
						KBE_ASSERT(iUtype == int(muid) && "EntityDef::loadDefCellMethods: Utype overflow!\n");

						methodDescription->setUType(muid);
					}
				}
				XML_FOR_END(argNode);		
			}

			// ���������û�����ù�utype, �����
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
bool EntityDef::loadDefBaseMethods(const std::string& moduleName, XmlPlus* xml, 
								   TiXmlNode* defMethodNode, ScriptDefModule* scriptModule)
{
	if(defMethodNode)
	{
		XML_FOR_BEGIN(defMethodNode)
		{
			std::string name = xml->getKey(defMethodNode);
			MethodDescription* methodDescription = new MethodDescription(0, BASEAPP_TYPE, name);
			TiXmlNode* argNode = defMethodNode->FirstChild();

			// ����û�в���
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
						KBE_ASSERT(iUtype == int(muid) && "EntityDef::loadDefBaseMethods: Utype overflow!\n");

						methodDescription->setUType(muid);
					}
				}
				XML_FOR_END(argNode);		
			}

			// ���������û�����ù�utype, �����
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
bool EntityDef::loadDefClientMethods(const std::string& moduleName, XmlPlus* xml, 
									 TiXmlNode* defMethodNode, ScriptDefModule* scriptModule)
{
	if(defMethodNode)
	{
		XML_FOR_BEGIN(defMethodNode)
		{
			std::string name = xml->getKey(defMethodNode);
			MethodDescription* methodDescription = new MethodDescription(0, CLIENT_TYPE, name);
			TiXmlNode* argNode = defMethodNode->FirstChild();

			// ����û�в���
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
						KBE_ASSERT(iUtype == int(muid) && "EntityDef::loadDefClientMethods: Utype overflow!\n");

						methodDescription->setUType(muid);
					}
				}
				XML_FOR_END(argNode);		
			}

			// ���������û�����ù�utype, �����
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
		if(!scriptModule->hasBase())
			return false;
		break;
	case CELLAPP_TYPE:
		if(!scriptModule->hasCell())
			return false;
		break;
	case CLIENT_TYPE:
	case BOTS_TYPE:
		if(!scriptModule->hasClient())
			return false;
		break;
	default:
		if(!scriptModule->hasCell())
			return false;
		break;
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
	for(; iter != methodDescrsPtr->end(); iter++)
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
bool EntityDef::loadAllScriptModule(std::string entitiesPath, 
									std::vector<PyTypeObject*>& scriptBaseTypes)
{
	std::string entitiesFile = entitiesPath + "entities.xml";

	SmartPointer<XmlPlus> xml(new XmlPlus());
	if(!xml.get()->openSection(entitiesFile.c_str()))
		return false;

	TiXmlNode* node = xml.get()->getRootNode();
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
			// �Ƿ�������ģ�� ��ȡ�����Ƿ���def�ļ��ж������뵱ǰ�����صķ����������ԣ�
			if(isLoadScriptModule(scriptModule))
			{
				ERROR_MSG(fmt::format("EntityDef::initialize:Could not load module[{}]\n", 
					moduleName.c_str()));

				PyErr_Print();
				return false;
			}

			PyErr_Clear();
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
			for(; iter != scriptBaseTypes.end(); iter++)
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
