// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "kbcmd.h"
#include "client_sdk.h"
#include "client_sdk_unity.h"	
#include "client_sdk_ue4.h"
#include "entitydef/entitydef.h"
#include "entitydef/scriptdef_module.h"
#include "entitydef/property.h"
#include "entitydef/method.h"
#include "entitydef/datatypes.h"
#include "entitydef/datatype.h"
#include "resmgr/resmgr.h"
#include "server/common.h"
#include "server/serverconfig.h"
#include "common/kbeversion.h"
#include "network/fixed_messages.h"

#include "client_lib/client_interface.h"
#include "baseapp/baseapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "dbmgr/dbmgr_interface.h"
#include "loginapp/loginapp_interface.h"

namespace KBEngine {	

//-------------------------------------------------------------------------------------
ClientSDK::ClientSDK():
	basepath_(),
	currSourcePath_(),
	currHeaderPath_(),
	sourcefileBody_(),
	sourcefileName_(),
	headerfileName_(),
	headerfileBody_()
{

}

//-------------------------------------------------------------------------------------
ClientSDK::~ClientSDK()
{

}

//-------------------------------------------------------------------------------------
ClientSDK* ClientSDK::createClientSDK(const std::string& type)
{
	std::string lowerType = type;
	std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(), tolower);

	if (lowerType == "unity")
	{
		return new ClientSDKUnity();
	}
	else if(lowerType == "ue4")
	{
		return new ClientSDKUE4();
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::good() const
{
	return true;
}

//-------------------------------------------------------------------------------------
void ClientSDK::onCreateEntityModuleFileName(const std::string& moduleName)
{
	sourcefileName_ = moduleName + ".unknown";
	headerfileName_ = "";
}

//-------------------------------------------------------------------------------------
bool ClientSDK::saveFile()
{
	bool done = false;

	if (sourcefileName_.size() > 0)
	{
		if (KBCMD::creatDir(currSourcePath_.c_str()) == -1)
		{
			ERROR_MSG(fmt::format("creating directory error! path={}\n", currSourcePath_));
			return false;
		}

		std::string path = currSourcePath_ + sourcefileName_;

		DEBUG_MSG(fmt::format("ClientSDK::saveFile(): {}\n",
			path));

		FILE *fp = fopen(path.c_str(), "w");

		if (NULL == fp)
		{
			ERROR_MSG(fmt::format("ClientSDK::saveFile(): fopen error! {}\n",
				path));

			return false;
		}

		int written = fwrite(sourcefileBody_.c_str(), 1, sourcefileBody_.size(), fp);
		if (written != (int)sourcefileBody_.size())
		{
			ERROR_MSG(fmt::format("ClientSDK::saveFile(): fwrite error! {}\n",
				path));

			return false;
		}

		if (fclose(fp))
		{
			ERROR_MSG(fmt::format("ClientSDK::saveFile(): fclose error! {}\n",
				path));

			return false;
		}

		done = true;
	}

	if (headerfileName_.size() > 0)
	{
		if (KBCMD::creatDir(currHeaderPath_.c_str()) == -1)
		{
			ERROR_MSG(fmt::format("creating directory error! path={}\n", currHeaderPath_));
			return false;
		}

		std::string path = currHeaderPath_ + headerfileName_;

		DEBUG_MSG(fmt::format("ClientSDK::saveFile(): {}\n",
			path));

		FILE *fp = fopen(path.c_str(), "w");

		if (NULL == fp)
		{
			ERROR_MSG(fmt::format("ClientSDK::saveFile(): fopen error! {}\n",
				path));

			return false;
		}

		int written = fwrite(headerfileBody_.c_str(), 1, headerfileBody_.size(), fp);
		if (written != (int)headerfileBody_.size())
		{
			ERROR_MSG(fmt::format("ClientSDK::saveFile(): fwrite error! {}\n",
				path));

			return false;
		}

		if (fclose(fp))
		{
			ERROR_MSG(fmt::format("ClientSDK::saveFile(): fclose error! {}\n",
				path));

			return false;
		}

		done = true;
	}

	return done;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::create(const std::string& path)
{
	basepath_ = path;

	if (basepath_[basepath_.size() - 1] != '\\' && basepath_[basepath_.size() - 1] != '/')
		basepath_ += "/";

	currHeaderPath_ = currSourcePath_ = basepath_;

	std::string findpath = "sdk_templates/client/" + name();

	std::string getpath = Resmgr::getSingleton().matchPath(findpath);

	if (getpath.size() == 0 || findpath == getpath)
	{
		ERROR_MSG(fmt::format("ClientSDK::create(): not found path({})\n",
			findpath));

		return false;
	}

	if (!copyPluginsSourceToPath(getpath))
		return false;

	if (!writeServerErrorDescrsModule())
		return false;

	if (!writeEngineMessagesModule())
		return false;

	if (!writeTypes())
		return false;

	if (!writeEntityDefsModule())
		return false;

	if (!writeCustomDataTypes())
		return false;
	
	const EntityDef::SCRIPT_MODULES& scriptModules = EntityDef::getScriptModules();
	EntityDef::SCRIPT_MODULES::const_iterator moduleIter = scriptModules.begin();
	for (; moduleIter != scriptModules.end(); ++moduleIter)
	{
		ScriptDefModule* pScriptDefModule = (*moduleIter).get();

		if (!writeEntityModule(pScriptDefModule))
			return false;

		if (!writeEntityCall(pScriptDefModule))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void ClientSDK::onCreateTypeFileName()
{
	sourcefileName_ = "KBEType.unknown";
	headerfileName_ = "";
}

//-------------------------------------------------------------------------------------
void ClientSDK::onCreateServerErrorDescrsModuleFileName()
{
	sourcefileName_ = "ServerErrDescrs.unknown";
	headerfileName_ = "";
}

//-------------------------------------------------------------------------------------
bool ClientSDK::copyPluginsSourceToPath(const std::string& path)
{
	wchar_t* wpath = strutil::char2wchar(path.c_str());
	std::wstring sourcePath = wpath;
	free(wpath);

	wpath = strutil::char2wchar(basepath_.c_str());
	std::wstring destPath = wpath;
	free(wpath);
	
	std::vector<std::wstring> results;
	if (!Resmgr::getSingleton().listPathRes(sourcePath, L"*", results))
		return false;

	wchar_t* wfindpath = strutil::char2wchar(std::string("sdk_templates/client/" + name()).c_str());
	std::wstring findpath = wfindpath;
	free(wfindpath);

	std::vector<std::wstring>::iterator iter = results.begin();
	for (; iter != results.end(); ++iter)
	{
		std::wstring::size_type fpos = (*iter).find(findpath);

		char* ccattr = strutil::wchar2char((*iter).c_str());
		std::string currpath = ccattr;
		free(ccattr);

		if (fpos == std::wstring::npos)
		{
			ERROR_MSG(fmt::format("ClientSDK::copyPluginsSourceToPath(): split path({}) error!\n",
				currpath));

			return false;
		}

		std::wstring targetFile = (*iter);
		targetFile.erase(0, fpos + findpath.size() + 1);
		targetFile = (destPath + targetFile);

		std::wstring basepath = targetFile;
		fpos = targetFile.rfind(L"/");

		ccattr = strutil::wchar2char(targetFile.c_str());
		std::string currTargetFile = ccattr;
		free(ccattr);

		if (fpos == std::wstring::npos)
		{
			ERROR_MSG(fmt::format("ClientSDK::copyPluginsSourceToPath(): split basepath({}) error!\n",
				currTargetFile));

			return false;
		}

		basepath.erase(fpos, basepath.size() - fpos);
		
		ccattr = strutil::wchar2char(basepath.c_str());
		std::string currbasepath = ccattr;
		free(ccattr);

		if (KBCMD::creatDir(currbasepath.c_str()) == -1)
		{
			ERROR_MSG(fmt::format("ClientSDK::copyPluginsSourceToPath(): creating directory error! path={}\n", currbasepath));
			return false;
		}

		std::ifstream input(currpath.c_str(), std::ios::binary);
		std::ofstream output(currTargetFile.c_str(), std::ios::binary);

		std::stringstream ss;
		std::string filebody;

		ss << input.rdbuf();
		filebody = ss.str();

		strutil::kbe_replace(filebody, "@{KBE_VERSION}", KBEVersion::versionString());
		strutil::kbe_replace(filebody, "@{KBE_SCRIPT_VERSION}", KBEVersion::scriptVersionString());
		strutil::kbe_replace(filebody, "@{KBE_SERVER_PROTO_MD5}", Network::MessageHandlers::getDigestStr());
		strutil::kbe_replace(filebody, "@{KBE_SERVER_ENTITYDEF_MD5}", EntityDef::md5().getDigestStr());
		strutil::kbe_replace(filebody, "@{KBE_USE_ALIAS_ENTITYID}", g_kbeSrvConfig.getCellApp().aliasEntityID ? "true" : "false");
		strutil::kbe_replace(filebody, "@{KBE_UPDATEHZ}", fmt::format("{}", g_kbeSrvConfig.gameUpdateHertz()));
		strutil::kbe_replace(filebody, "@{KBE_LOGIN_PORT}", fmt::format("{}", g_kbeSrvConfig.getLoginApp().externalTcpPorts_min));
		strutil::kbe_replace(filebody, "@{KBE_SERVER_EXTERNAL_TIMEOUT}", fmt::format("{}", (int)g_kbeSrvConfig.channelExternalTimeout()));
		output << filebody;

		output.close();
		input.close();
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeServerErrorDescrsModule()
{
	std::map<uint16, std::pair< std::string, std::string> > errsDescrs;

	{
		TiXmlNode *rootNode = NULL;
		SmartPointer<XML> xml(new XML(Resmgr::getSingleton().matchRes("server/server_errors_defaults.xml").c_str()));

		if (!xml->isGood())
		{
			ERROR_MSG(fmt::format("ClientSDK::writeServerErrorDescrsModule: load {} is failed!\n",
				"server/server_errors_defaults.xml"));

			return false;
		}

		rootNode = xml->getRootNode();
		if (rootNode)
		{
			XML_FOR_BEGIN(rootNode)
			{
				TiXmlNode* node = xml->enterNode(rootNode->FirstChild(), "id");
				TiXmlNode* node1 = xml->enterNode(rootNode->FirstChild(), "descr");
				errsDescrs[xml->getValInt(node)] = std::make_pair< std::string, std::string>(xml->getKey(rootNode), xml->getVal(node1));
			}
			XML_FOR_END(rootNode);
		}
	}

	{
		TiXmlNode *rootNode = NULL;

		FILE* f = Resmgr::getSingleton().openRes("server/server_errors.xml");

		if (f)
		{
			fclose(f);
			SmartPointer<XML> xml(new XML(Resmgr::getSingleton().matchRes("server/server_errors.xml").c_str()));

			if (xml->isGood())
			{
				rootNode = xml->getRootNode();
				if (rootNode)
				{
					XML_FOR_BEGIN(rootNode)
					{
						TiXmlNode* node = xml->enterNode(rootNode->FirstChild(), "id");
						TiXmlNode* node1 = xml->enterNode(rootNode->FirstChild(), "descr");
						errsDescrs[xml->getValInt(node)] = std::make_pair< std::string, std::string>(xml->getKey(rootNode), xml->getVal(node1));
					}
					XML_FOR_END(rootNode);
				}
			}
		}
	}

	sourcefileName_ = sourcefileBody_ = "";
	headerfileName_ = headerfileBody_ = "";

	onCreateServerErrorDescrsModuleFileName();

	DEBUG_MSG(fmt::format("ClientSDK::writeServerErrorDescrsModule(): {}/{}\n",
		basepath_, sourcefileName_));

	if (!writeServerErrorDescrsModuleBegin())
		return false;

	std::map<uint16, std::pair< std::string, std::string> >::iterator iter = errsDescrs.begin();

	for (; iter != errsDescrs.end(); ++iter)
	{
		if (!writeServerErrorDescrsModuleErrDescr(iter->first, iter->second.first, iter->second.second))
			return false;
	}

	if (!writeServerErrorDescrsModuleEnd())
		return false;

	return saveFile();
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeServerErrorDescrsModuleBegin()
{
	ERROR_MSG(fmt::format("ClientSDK::writeServerErrorDescrsModuleBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeServerErrorDescrsModuleErrDescr(int errorID, const std::string& errname, const std::string& errdescr)
{
	ERROR_MSG(fmt::format("ClientSDK::writeServerErrorDescrsModuleErrDescr: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeServerErrorDescrsModuleEnd()
{
	ERROR_MSG(fmt::format("ClientSDK::writeServerErrorDescrsModuleEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
void ClientSDK::onCreateEngineMessagesModuleFileName()
{
	sourcefileName_ = "Messages.unknown";
	headerfileName_ = "";
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEngineMessagesModule()
{
	sourcefileName_ = sourcefileBody_ = "";
	headerfileName_ = headerfileBody_ = "";

	onCreateEngineMessagesModuleFileName();

	DEBUG_MSG(fmt::format("ClientSDK::writeEngineMessagesModule(): {}/{}\n",
		basepath_, sourcefileName_));

	if (!writeEngineMessagesModuleBegin())
		return false;

	std::map< Network::MessageID, Network::ExposedMessageInfo > clientMessages;
	{
		const Network::MessageHandlers::MessageHandlerMap& msgHandlers = ClientInterface::messageHandlers.msgHandlers();
		Network::MessageHandlers::MessageHandlerMap::const_iterator iter = msgHandlers.begin();
		for (; iter != msgHandlers.end(); ++iter)
		{
			Network::MessageHandler* pMessageHandler = iter->second;

			Network::ExposedMessageInfo& info = clientMessages[iter->first];
			info.id = iter->first;
			info.name = pMessageHandler->name;
			info.msgLen = pMessageHandler->msgLen;
			info.argsType = (int8)pMessageHandler->pArgs->type();

			KBEngine::strutil::kbe_replace(info.name, "::", "_");
			std::vector<std::string>::iterator iter1 = pMessageHandler->pArgs->strArgsTypes.begin();
			for (; iter1 != pMessageHandler->pArgs->strArgsTypes.end(); ++iter1)
			{
				info.argsTypes.push_back((uint8)datatype2id((*iter1)));
			}

			if (!writeEngineMessagesModuleMessage(info, CLIENT_TYPE))
				return false;
		}
	}

	std::map< Network::MessageID, Network::ExposedMessageInfo > messages;
	{
		const Network::MessageHandlers::MessageHandlerMap& msgHandlers = LoginappInterface::messageHandlers.msgHandlers();
		Network::MessageHandlers::MessageHandlerMap::const_iterator iter = msgHandlers.begin();
		for (; iter != msgHandlers.end(); ++iter)
		{
			Network::MessageHandler* pMessageHandler = iter->second;
			if (!iter->second->exposed)
				continue;

			Network::ExposedMessageInfo& info = messages[iter->first];
			info.id = iter->first;
			info.name = pMessageHandler->name;
			info.msgLen = pMessageHandler->msgLen;

			KBEngine::strutil::kbe_replace(info.name, "::", "_");
			std::vector<std::string>::iterator iter1 = pMessageHandler->pArgs->strArgsTypes.begin();
			for (; iter1 != pMessageHandler->pArgs->strArgsTypes.end(); ++iter1)
			{
				info.argsTypes.push_back((uint8)datatype2id((*iter1)));
			}

			if (!writeEngineMessagesModuleMessage(info, LOGINAPP_TYPE))
				return false;
		}
	}

	{
		const Network::MessageHandlers::MessageHandlerMap& msgHandlers = BaseappInterface::messageHandlers.msgHandlers();
		Network::MessageHandlers::MessageHandlerMap::const_iterator iter = msgHandlers.begin();
		for (; iter != msgHandlers.end(); ++iter)
		{
			Network::MessageHandler* pMessageHandler = iter->second;
			if (!iter->second->exposed)
				continue;

			Network::ExposedMessageInfo& info = messages[iter->first];
			info.id = iter->first;
			info.name = pMessageHandler->name;
			info.msgLen = pMessageHandler->msgLen;
			info.argsType = (int8)pMessageHandler->pArgs->type();

			KBEngine::strutil::kbe_replace(info.name, "::", "_");
			std::vector<std::string>::iterator iter1 = pMessageHandler->pArgs->strArgsTypes.begin();
			for (; iter1 != pMessageHandler->pArgs->strArgsTypes.end(); ++iter1)
			{
				info.argsTypes.push_back((uint8)datatype2id((*iter1)));
			}

			if (!writeEngineMessagesModuleMessage(info, BASEAPP_TYPE))
				return false;
		}
	}

	if (!writeEngineMessagesModuleEnd())
		return false;

	return saveFile();
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEngineMessagesModuleBegin()
{
	ERROR_MSG(fmt::format("ClientSDK::writeEngineMessagesModuleBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEngineMessagesModuleMessage(Network::ExposedMessageInfo& messageInfos, COMPONENT_TYPE componentType)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEngineMessagesModuleMessage: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEngineMessagesModuleEnd()
{
	ERROR_MSG(fmt::format("ClientSDK::writeEngineMessagesModuleEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
void ClientSDK::onCreateEntityDefsModuleFileName()
{
	sourcefileName_ = "EntityDef.unknown";
	headerfileName_ = "";
}

//-------------------------------------------------------------------------------------
void ClientSDK::onCreateDefsCustomTypesModuleFileName()
{
	sourcefileName_ = "CustomDataTypes.unknown";
	headerfileName_ = "";
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModule()
{
	sourcefileName_ = sourcefileBody_ = "";
	headerfileName_ = headerfileBody_ = "";

	onCreateEntityDefsModuleFileName();

	DEBUG_MSG(fmt::format("ClientSDK::writeEntityDefsModule(): {}/{}\n",
		basepath_, sourcefileName_));

	if (!writeEntityDefsModuleBegin())
		return false;

	if (!writeEntityDefsModuleInitScriptBegin())
		return false;

	const EntityDef::SCRIPT_MODULES& modules = EntityDef::getScriptModules();
	EntityDef::SCRIPT_MODULES::const_iterator iter = modules.begin();
	for (; iter != modules.end(); ++iter)
	{
		if (!iter->get()->hasClient())
			continue;

		if (!writeEntityDefsModuleInitScript(iter->get()))
			return false;
	}

	if (!writeEntityDefsModuleInitScriptEnd())
		return false;

	if(!writeEntityDefsModuleInitDefTypes())
		return false;

	onCreateEntityDefsModuleFileName();

	if (!writeEntityDefsModuleEnd())
		return false;

	return saveFile();
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleInitScriptBegin()
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityDefsModuleInitScriptBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleInitScriptEnd()
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityDefsModuleInitScriptEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleBegin()
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityDefsModuleBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleEnd()
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityDefsModuleEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleInitScript(ScriptDefModule* pScriptDefModule)
{
	if (!writeEntityDefsModuleInitScript_ScriptModule(pScriptDefModule))
		return false;

	const ScriptDefModule::PROPERTYDESCRIPTION_MAP& propers = pScriptDefModule->getClientPropertyDescriptions();
	const ScriptDefModule::METHODDESCRIPTION_MAP& methods = pScriptDefModule->getClientMethodDescriptions();
	const ScriptDefModule::METHODDESCRIPTION_MAP& methods1 = pScriptDefModule->getBaseExposedMethodDescriptions();
	const ScriptDefModule::METHODDESCRIPTION_MAP& methods2 = pScriptDefModule->getCellExposedMethodDescriptions();

	//(*pBundleImportEntityDefDatas_) << iter->get()->getName() << iter->get()->getUType() << size << size1 << size2 << size3;

	ENTITY_PROPERTY_UID posuid = 0;
	if (posuid == 0)
	{
		posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
		Network::FixedMessages::MSGInfo* msgInfo =
			Network::FixedMessages::getSingleton().isFixed("Property::position");

		if (msgInfo != NULL)
			posuid = msgInfo->msgid;
	}

	PropertyDescription positionDescription(posuid, "VECTOR3", "position", ED_FLAG_ALL_CLIENTS, true, DataTypes::getDataType("VECTOR3"), false, "", 0, "", DETAIL_LEVEL_FAR);
	if (pScriptDefModule->usePropertyDescrAlias() && positionDescription.aliasID() == -1)
		positionDescription.aliasID(ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ);

	if (!writeEntityDefsModuleInitScript_PropertyDescr(pScriptDefModule, &positionDescription))
		return false;


	ENTITY_PROPERTY_UID diruid = 0;
	if (diruid == 0)
	{
		diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;
		Network::FixedMessages::MSGInfo* msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::direction");
		if (msgInfo != NULL)
			diruid = msgInfo->msgid;
	}

	PropertyDescription directionDescription(diruid, "VECTOR3", "direction", ED_FLAG_ALL_CLIENTS, true, DataTypes::getDataType("VECTOR3"), false, "", 0, "", DETAIL_LEVEL_FAR);
	if (pScriptDefModule->usePropertyDescrAlias() && directionDescription.aliasID() == -1)
		directionDescription.aliasID(ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW);

	if (!writeEntityDefsModuleInitScript_PropertyDescr(pScriptDefModule, &directionDescription))
		return false;

	ENTITY_PROPERTY_UID spaceuid = 0;
	if (spaceuid == 0)
	{
		spaceuid = ENTITY_BASE_PROPERTY_UTYPE_SPACEID;
		Network::FixedMessages::MSGInfo* msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::spaceID");
		if (msgInfo != NULL)
			spaceuid = msgInfo->msgid;
	}

	PropertyDescription spaceDescription(spaceuid, "UINT32", "spaceID", ED_FLAG_OWN_CLIENT, true, DataTypes::getDataType("UINT32"), false, "", 0, "", DETAIL_LEVEL_FAR);
	if (pScriptDefModule->usePropertyDescrAlias() && spaceDescription.aliasID() == -1)
		spaceDescription.aliasID(ENTITY_BASE_PROPERTY_ALIASID_SPACEID);

	if (!writeEntityDefsModuleInitScript_PropertyDescr(pScriptDefModule, &spaceDescription))
		return false;

	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator piter = propers.begin();
	for (; piter != propers.end(); ++piter)
	{
		if (!writeEntityDefsModuleInitScript_PropertyDescr(pScriptDefModule, piter->second))
			return false;
	}

	ScriptDefModule::METHODDESCRIPTION_MAP::const_iterator miter = methods.begin();
	if (methods.size() > 0)
	{
		for (; miter != methods.end(); ++miter)
		{
			if (!writeEntityDefsModuleInitScript_MethodDescr(pScriptDefModule, miter->second, CLIENT_TYPE))
				return false;
		}
	}
	else
	{
		if (!writeEntityDefsModuleInitScript_MethodDescr(pScriptDefModule, NULL, CLIENT_TYPE))
			return false;
	}

	miter = methods1.begin();
	for (; miter != methods1.end(); ++miter)
	{
		if (!writeEntityDefsModuleInitScript_MethodDescr(pScriptDefModule, miter->second, BASEAPP_TYPE))
			return false;
	}

	miter = methods2.begin();
	for (; miter != methods2.end(); ++miter)
	{
		if (!writeEntityDefsModuleInitScript_MethodDescr(pScriptDefModule, miter->second, CELLAPP_TYPE))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleInitScript_ScriptModule(ScriptDefModule* pScriptDefModule)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityDefsModuleInitScript_ScriptModule: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleInitScript_MethodDescr(ScriptDefModule* pScriptDefModule, MethodDescription* pDescr, COMPONENT_TYPE componentType)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityDefsModuleInitScript_MethodDescr: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleInitScript_PropertyDescr(ScriptDefModule* pScriptDefModule, PropertyDescription* pDescr)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityDefsModuleInitScript_PropertyDescr: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleInitDefTypesBegin()
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityDefsModuleInitDefTypesBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleInitDefTypesEnd()
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityDefsModuleInitDefTypesEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleInitDefType(const DataType* pDataType)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityDefsModuleInitDefType: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityDefsModuleInitDefTypes()
{
	if (!writeEntityDefsModuleInitDefTypesBegin())
		return false;

	const DataTypes::UID_DATATYPE_MAP& dataTypes = DataTypes::uid_dataTypes();
	DataTypes::UID_DATATYPE_MAP::const_iterator dtiter = dataTypes.begin();
	for (; dtiter != dataTypes.end(); ++dtiter)
	{
		const DataType* datatype = dtiter->second;

		if (datatype->aliasName()[0] == '_')
			continue;

		if (!writeEntityDefsModuleInitDefType(datatype))
			return false;
	}

	return writeEntityDefsModuleInitDefTypesEnd();
}

//-------------------------------------------------------------------------------------
void ClientSDK::onEntityCallModuleFileName(const std::string& moduleName)
{
	sourcefileName_ = std::string("EntityCall") + moduleName + ".unknown";
	headerfileName_ = "";
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityCall(ScriptDefModule* pScriptDefModule)
{
	sourcefileName_ = sourcefileBody_ = "";
	headerfileName_ = headerfileBody_ = "";

	onEntityCallModuleFileName(pScriptDefModule->getName());

	if (!writeEntityCallBegin(pScriptDefModule))
		return false;

	std::string newModuleName;

	// ÏÈÐ´BaseEntityCall
	if(!writeBaseEntityCallBegin(pScriptDefModule))
		return false;

	{
		ScriptDefModule::METHODDESCRIPTION_MAP& scriptMethods = pScriptDefModule->getBaseMethodDescriptions();
		ScriptDefModule::METHODDESCRIPTION_MAP::iterator methodIter = scriptMethods.begin();
		for (; methodIter != scriptMethods.end(); ++methodIter)
		{
			MethodDescription* pMethodDescription = methodIter->second;

			if (!pMethodDescription->isExposed())
				continue;

			if (!writeEntityCallMethodBegin(pScriptDefModule, pMethodDescription, "#REPLACE_FILLARGS1#", "#REPLACE_FILLARGS2#", BASEAPP_TYPE))
				return false;

			std::string::size_type fHeaderPos = headerfileBody_.find("#REPLACE_FILLARGS1#");
			std::string::size_type fSourcePos = sourcefileBody_.find("#REPLACE_FILLARGS1#");
			KBE_ASSERT((fHeaderPos != std::string::npos) || (fSourcePos != std::string::npos));

			std::string argsBody1 = "";
			std::string argsBody2 = "";

			std::vector<DataType*>& argTypes = pMethodDescription->getArgTypes();
			std::vector<DataType*>::iterator iter = argTypes.begin();

			int i = 1;

			for (; iter != argTypes.end(); ++iter)
			{
				DataType* pDataType = (*iter);

				argsBody2 += fmt::format("arg{}, ", i);

				if (pDataType->type() == DATA_TYPE_FIXEDARRAY)
				{
					FixedArrayType* pFixedArrayType = static_cast<FixedArrayType*>(pDataType);

					std::string argsTypeBody;
					if (!writeEntityMethodArgs_ARRAY(pFixedArrayType, argsTypeBody, pFixedArrayType->aliasName()))
					{
						return false;
					}

					argsBody1 += fmt::format("{} arg{}, ", argsTypeBody, i++);
				}
				else if (pDataType->type() == DATA_TYPE_FIXEDDICT)
				{
					FixedDictType* pFixedDictType = static_cast<FixedDictType*>(pDataType);

					std::string argsTypeBody = typeToType(pFixedDictType->aliasName());
					if (!writeEntityMethodArgs_Const_Ref(pDataType, argsTypeBody))
					{
						return false;
					}

					argsBody1 += fmt::format("{} arg{}, ", argsTypeBody, i++);
				}
				else if (pDataType->type() != DATA_TYPE_DIGIT)
				{
					std::string argsTypeBody = typeToType(pDataType->getName());
					if (!writeEntityMethodArgs_Const_Ref(pDataType, argsTypeBody))
					{
						return false;
					}

					argsBody1 += fmt::format("{} arg{}, ", argsTypeBody, i++);
				}
				else
				{
					argsBody1 += fmt::format("{} arg{}, ", typeToType(pDataType->getName()), i++);
				}
			}

			if (argsBody1.size() > 0)
			{
				argsBody1.erase(argsBody1.size() - 2, 2);
				argsBody2.erase(argsBody2.size() - 2, 2);

				argsBody2 = std::string(", ") + argsBody2;
			}

			strutil::kbe_replace(headerfileBody_, "#REPLACE_FILLARGS1#", argsBody1);
			strutil::kbe_replace(headerfileBody_, "#REPLACE_FILLARGS2#", argsBody2);
			strutil::kbe_replace(sourcefileBody_, "#REPLACE_FILLARGS1#", argsBody1);
			strutil::kbe_replace(sourcefileBody_, "#REPLACE_FILLARGS2#", argsBody2);

			if (!writeEntityCallMethodEnd(pScriptDefModule, pMethodDescription))
				return false;
		}
	}

	if (!writeBaseEntityCallEnd(pScriptDefModule))
		return false;

	headerfileBody_ += fmt::format("\n");
	sourcefileBody_ += fmt::format("\n");

	// ÔÙÐ´CellEntityCall
	if (!writeCellEntityCallBegin(pScriptDefModule))
		return false;

	{
		ScriptDefModule::METHODDESCRIPTION_MAP& scriptMethods = pScriptDefModule->getCellMethodDescriptions();
		ScriptDefModule::METHODDESCRIPTION_MAP::iterator methodIter = scriptMethods.begin();
		for (; methodIter != scriptMethods.end(); ++methodIter)
		{
			MethodDescription* pMethodDescription = methodIter->second;

			if (!pMethodDescription->isExposed())
				continue;

			if (!writeEntityCallMethodBegin(pScriptDefModule, pMethodDescription, "#REPLACE_FILLARGS1#", "#REPLACE_FILLARGS2#", CELLAPP_TYPE))
				return false;

			std::string::size_type fHeaderPos = headerfileBody_.find("#REPLACE_FILLARGS1#");
			std::string::size_type fSourcePos = sourcefileBody_.find("#REPLACE_FILLARGS1#");
			KBE_ASSERT((fHeaderPos != std::string::npos) || (fSourcePos != std::string::npos));

			std::string argsBody1 = "";
			std::string argsBody2 = "";

			std::vector<DataType*>& argTypes = pMethodDescription->getArgTypes();
			std::vector<DataType*>::iterator iter = argTypes.begin();

			int i = 1;

			for (; iter != argTypes.end(); ++iter)
			{
				DataType* pDataType = (*iter);

				argsBody2 += fmt::format("arg{}, ", i);

				if (pDataType->type() == DATA_TYPE_FIXEDARRAY)
				{
					FixedArrayType* pFixedArrayType = static_cast<FixedArrayType*>(pDataType);

					std::string argsTypeBody;
					if (!writeEntityMethodArgs_ARRAY(pFixedArrayType, argsTypeBody, pFixedArrayType->aliasName()))
					{
						return false;
					}

					argsBody1 += fmt::format("{} arg{}, ", argsTypeBody, i++);
				}
				else if (pDataType->type() == DATA_TYPE_FIXEDDICT)
				{
					FixedDictType* pFixedDictType = static_cast<FixedDictType*>(pDataType);

					std::string argsTypeBody = typeToType(pFixedDictType->aliasName());
					if (!writeEntityMethodArgs_Const_Ref(pDataType, argsTypeBody))
					{
						return false;
					}

					argsBody1 += fmt::format("{} arg{}, ", argsTypeBody, i++);
				}
				else if (pDataType->type() != DATA_TYPE_DIGIT)
				{
					std::string argsTypeBody = typeToType(pDataType->getName());
					if (!writeEntityMethodArgs_Const_Ref(pDataType, argsTypeBody))
					{
						return false;
					}

					argsBody1 += fmt::format("{} arg{}, ", argsTypeBody, i++);
				}
				else
				{
					argsBody1 += fmt::format("{} arg{}, ", typeToType(pDataType->getName()), i++);
				}
			}

			if (argsBody1.size() > 0)
			{
				argsBody1.erase(argsBody1.size() - 2, 2);
				argsBody2.erase(argsBody2.size() - 2, 2);

				argsBody2 = std::string(", ") + argsBody2;
			}

			strutil::kbe_replace(headerfileBody_, "#REPLACE_FILLARGS1#", argsBody1);
			strutil::kbe_replace(headerfileBody_, "#REPLACE_FILLARGS2#", argsBody2);
			strutil::kbe_replace(sourcefileBody_, "#REPLACE_FILLARGS1#", argsBody1);
			strutil::kbe_replace(sourcefileBody_, "#REPLACE_FILLARGS2#", argsBody2);

			if (!writeEntityCallMethodEnd(pScriptDefModule, pMethodDescription))
				return false;
		}
	}

	if (!writeCellEntityCallEnd(pScriptDefModule))
		return false;

	if (!writeEntityCallEnd(pScriptDefModule))
		return false;

	return saveFile();
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityCallMethodBegin(ScriptDefModule* pScriptDefModule, MethodDescription* pMethodDescription, const char* fillString1, const char* fillString2, COMPONENT_TYPE componentType)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityCallMethodBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityCallMethodEnd(ScriptDefModule* pScriptDefModule, MethodDescription* pMethodDescription)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityCallMethodEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityCallBegin(ScriptDefModule* pScriptDefModule)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityCallBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityCallEnd(ScriptDefModule* pScriptDefModule)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityCallEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeBaseEntityCallBegin(ScriptDefModule* pScriptDefModule)
{
	ERROR_MSG(fmt::format("ClientSDK::writeBaseEntityCallBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeBaseEntityCallEnd(ScriptDefModule* pScriptDefModule)
{
	ERROR_MSG(fmt::format("ClientSDK::writeBaseEntityCallEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeCellEntityCallBegin(ScriptDefModule* pScriptDefModule)
{
	ERROR_MSG(fmt::format("ClientSDK::writeCellEntityCallBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeCellEntityCallEnd(ScriptDefModule* pScriptDefModule)
{
	ERROR_MSG(fmt::format("ClientSDK::writeCellEntityCallEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeCustomDataTypesBegin()
{
	ERROR_MSG(fmt::format("ClientSDK::writeCustomDataTypesBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeCustomDataTypesEnd()
{
	ERROR_MSG(fmt::format("ClientSDK::writeCustomDataTypesEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeCustomDataTypes()
{
	sourcefileName_ = sourcefileBody_ = "";
	headerfileName_ = headerfileBody_ = "";

	onCreateDefsCustomTypesModuleFileName();

	if (!writeCustomDataTypesBegin())
		return false;

	const DataTypes::UID_DATATYPE_MAP& dataTypes = DataTypes::uid_dataTypes();
	DataTypes::UID_DATATYPE_MAP::const_iterator dtiter = dataTypes.begin();
	for (; dtiter != dataTypes.end(); ++dtiter)
	{
		const DataType* datatype = dtiter->second;

		if (datatype->aliasName()[0] == '_')
			continue;

		if (!writeCustomDataType(datatype))
			return false;
	}

	onCreateDefsCustomTypesModuleFileName();
	return writeCustomDataTypesEnd() && saveFile();
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeCustomDataType(const DataType* pDataType)
{
	ERROR_MSG(fmt::format("ClientSDK::writeCustomDataType: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeTypes()
{
	sourcefileName_ = sourcefileBody_ = "";
	headerfileName_ = headerfileBody_ = "";

	onCreateTypeFileName();

	if (!writeTypesBegin())
		return false;

	const DataTypes::DATATYPE_MAP& dataTypes = DataTypes::dataTypes();
	const DataTypes::DATATYPE_ORDERS& dataTypesOrders = DataTypes::dataTypesOrders();
	DataTypes::DATATYPE_ORDERS::const_iterator oiter = dataTypesOrders.begin();

	for (; oiter != dataTypesOrders.end(); ++oiter)
	{
		DataTypes::DATATYPE_MAP::const_iterator iter = dataTypes.find((*oiter));

		std::string typeName = iter->first;

		if (typeName[0] == '_')
			continue;

		DataType* pDataType = iter->second.get();

		if (pDataType->type() == DATA_TYPE_FIXEDDICT)
		{
			FixedDictType* pFixedDictType = static_cast<FixedDictType*>(pDataType);

			if (!writeTypeBegin(typeName, pFixedDictType))
				return false;

			FixedDictType::FIXEDDICT_KEYTYPE_MAP& keyTypes = pFixedDictType->getKeyTypes();
			FixedDictType::FIXEDDICT_KEYTYPE_MAP::iterator itemIter = keyTypes.begin();
			for(; itemIter != keyTypes.end(); ++itemIter)
			{
				std::string type = itemIter->second->dataType->getName();
				std::string itemTypeName = itemIter->first;
				std::string itemTypeAliasName = itemIter->second->dataType->aliasName();

				if (type == "INT8")
				{
					if (!writeTypeItemType_INT8(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "INT16")
				{
					if (!writeTypeItemType_INT16(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "INT32")
				{
					if (!writeTypeItemType_INT32(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "INT64")
				{
					if (!writeTypeItemType_INT64(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "UINT8")
				{
					if (!writeTypeItemType_UINT8(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "UINT16")
				{
					if (!writeTypeItemType_UINT16(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "UINT32")
				{
					if (!writeTypeItemType_UINT32(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "UINT64")
				{
					if (!writeTypeItemType_UINT64(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "FLOAT")
				{
					if (!writeTypeItemType_FLOAT(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "DOUBLE")
				{
					if (!writeTypeItemType_DOUBLE(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "STRING")
				{
					if (!writeTypeItemType_STRING(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "UNICODE")
				{
					if (!writeTypeItemType_UNICODE(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "PYTHON")
				{
					if (!writeTypeItemType_PYTHON(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "PY_DICT")
				{
					if (!writeTypeItemType_PY_DICT(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "PY_TUPLE")
				{
					if (!writeTypeItemType_PY_TUPLE(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "PY_LIST")
				{
					if (!writeTypeItemType_PY_LIST(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "BLOB")
				{
					if (!writeTypeItemType_BLOB(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "ARRAY")
				{
					if (!writeTypeItemType_ARRAY(itemTypeName, itemTypeAliasName, itemIter->second->dataType))
						return false;
				}
				else if (type == "FIXED_DICT")
				{
					if (!writeTypeItemType_FIXED_DICT(itemTypeName, itemTypeAliasName, itemIter->second->dataType))
						return false;
				}
#ifdef CLIENT_NO_FLOAT
				else if (type == "VECTOR2")
				{
					if (!writeTypeItemType_VECTOR2(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "VECTOR3")
				{
					if (!writeTypeItemType_VECTOR3(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "VECTOR4")
				{
					if (!writeTypeItemType_VECTOR4(itemTypeName, itemTypeAliasName))
						return false;
				}
#else
				else if (type == "VECTOR2")
				{
					if (!writeTypeItemType_VECTOR2(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "VECTOR3")
				{
					if (!writeTypeItemType_VECTOR3(itemTypeName, itemTypeAliasName))
						return false;
				}
				else if (type == "VECTOR4")
				{
					if (!writeTypeItemType_VECTOR4(itemTypeName, itemTypeAliasName))
						return false;
				}
#endif
				else if (type == "ENTITYCALL")
				{
					if (!writeTypeItemType_ENTITYCALL(itemTypeName, itemTypeAliasName))
						return false;
				}
			}

			if (!writeTypeEnd(typeName, pFixedDictType))
				return false;
		}
		else if (pDataType->type() == DATA_TYPE_FIXEDARRAY)
		{
			FixedArrayType* pFixedArrayType = static_cast<FixedArrayType*>(pDataType);

			if (!writeTypeBegin(typeName, pFixedArrayType, fmt::format("{}<#REPLACE#>", typeToType("ARRAY"))))
				return false;

			std::string type = pFixedArrayType->getDataType()->getName();
			std::string itemTypeAliasName = pFixedArrayType->getDataType()->aliasName();

			if (type == "ARRAY")
			{
				std::string newType;
				getArrayType(pFixedArrayType->getDataType(), newType);
				strutil::kbe_replace(headerfileBody_, "#REPLACE#", newType);
				strutil::kbe_replace(sourcefileBody_, "#REPLACE#", newType);
			}
			else if (type == "FIXED_DICT")
			{
				strutil::kbe_replace(headerfileBody_, "#REPLACE#", itemTypeAliasName);
				strutil::kbe_replace(sourcefileBody_, "#REPLACE#", itemTypeAliasName);
			}
			else
			{
				std::string newType = typeToType(type);
				strutil::kbe_replace(headerfileBody_, "#REPLACE#", newType);
				strutil::kbe_replace(sourcefileBody_, "#REPLACE#", newType);
			}

			std::string::size_type fHeaderPos = headerfileBody_.find("#REPLACE#");
			std::string::size_type fSourcePos = sourcefileBody_.find("#REPLACE#");
			KBE_ASSERT((fHeaderPos == std::string::npos) || (fSourcePos == std::string::npos));

			if (!writeTypeEnd(typeName, pFixedArrayType))
				return false;
		}
		else
		{
			if (!writeTypeBegin(typeName, pDataType))
				return false;

			if(!writeTypeItemType_AliasName(typeName, pDataType->getName()))
				return false;

			if (!writeTypeEnd(typeName, pDataType))
				return false;
		}
	}

	if (!writeTypesEnd())
		return false; 

	return saveFile();
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeTypesBegin()
{
	ERROR_MSG(fmt::format("ClientSDK::writeTypesBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeTypesEnd()
{
	ERROR_MSG(fmt::format("ClientSDK::writeTypesEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityModule(ScriptDefModule* pEntityScriptDefModule)
{
	DEBUG_MSG(fmt::format("ClientSDK::writeEntityModule(): {}/{}\n",
		currSourcePath_, pEntityScriptDefModule->getName()));

	sourcefileName_ = sourcefileBody_ = "";
	headerfileName_ = headerfileBody_ = "";

	onCreateEntityModuleFileName(pEntityScriptDefModule->getName());

	if (!writeEntityModuleBegin(pEntityScriptDefModule))
		return false;

	if (!writeEntityPropertys(pEntityScriptDefModule, pEntityScriptDefModule))
		return false;

	if (!writeEntityMethods(pEntityScriptDefModule, pEntityScriptDefModule))
		return false;

	if (!writeEntityProcessMessagesMethod(pEntityScriptDefModule))
		return false;
	
	if (!writeEntityModuleEnd(pEntityScriptDefModule))
		return false;

	return saveFile();
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityModuleBegin(ScriptDefModule* pEntityScriptDefModule)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityModuleBegin: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityModuleEnd(ScriptDefModule* pEntityScriptDefModule)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityModuleEnd: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityPropertys(ScriptDefModule* pEntityScriptDefModule,
	ScriptDefModule* pCurrScriptDefModule)
{
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& clientPropertys = pCurrScriptDefModule->getClientPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator propIter = clientPropertys.begin();
	for (; propIter != clientPropertys.end(); ++propIter)
	{
		PropertyDescription* pPropertyDescription = propIter->second;

		if (pPropertyDescription->getDataType()->type() == DATA_TYPE_ENTITY_COMPONENT)
		{
			if (!writeEntityPropertyComponent(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription))
				return false;
		}
		else
		{
			if (!writeEntityProperty(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription))
				return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityProperty(ScriptDefModule* pEntityScriptDefModule,
	ScriptDefModule* pCurrScriptDefModule, PropertyDescription* pPropertyDescription)
{
	std::string type = pPropertyDescription->getDataType()->getName();

	if (type == "INT8")
	{
		return writeEntityProperty_INT8(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "INT16")
	{
		return writeEntityProperty_INT16(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "INT32")
	{
		return writeEntityProperty_INT32(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "INT64")
	{
		return writeEntityProperty_INT64(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "UINT8")
	{
		return writeEntityProperty_UINT8(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "UINT16")
	{
		return writeEntityProperty_UINT16(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "UINT32")
	{
		return writeEntityProperty_UINT32(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "UINT64")
	{
		return writeEntityProperty_UINT64(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "FLOAT")
	{
		return writeEntityProperty_FLOAT(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "DOUBLE")
	{
		return writeEntityProperty_DOUBLE(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "STRING")
	{
		return writeEntityProperty_STRING(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "UNICODE")
	{
		return writeEntityProperty_UNICODE(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "PYTHON")
	{
		return writeEntityProperty_PYTHON(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "PY_DICT")
	{
		return writeEntityProperty_PY_DICT(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "PY_TUPLE")
	{
		return writeEntityProperty_PY_TUPLE(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "PY_LIST")
	{
		return writeEntityProperty_PY_LIST(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "BLOB")
	{
		return writeEntityProperty_BLOB(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "ARRAY")
	{
		return writeEntityProperty_ARRAY(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "FIXED_DICT")
	{
		return writeEntityProperty_FIXED_DICT(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
#ifdef CLIENT_NO_FLOAT
	else if (type == "VECTOR2")
	{
		return writeEntityProperty_VECTOR2(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "VECTOR3")
	{
		return writeEntityProperty_VECTOR3(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "VECTOR4")
	{
		return writeEntityProperty_VECTOR4(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
#else
	else if (type == "VECTOR2")
	{
		return writeEntityProperty_VECTOR2(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "VECTOR3")
	{
		return writeEntityProperty_VECTOR3(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
	else if (type == "VECTOR4")
	{
		return writeEntityProperty_VECTOR4(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}
#endif
	else if (type == "ENTITYCALL")
	{
		return writeEntityProperty_ENTITYCALL(pEntityScriptDefModule, pCurrScriptDefModule, pPropertyDescription);
	}

	assert(false);
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityMethods(ScriptDefModule* pEntityScriptDefModule,
	ScriptDefModule* pCurrScriptDefModule)
{
	sourcefileBody_ += "\n";
	headerfileBody_ += "\n";

	ScriptDefModule::METHODDESCRIPTION_MAP& clientMethods = pCurrScriptDefModule->getClientMethodDescriptions();
	ScriptDefModule::METHODDESCRIPTION_MAP::iterator methodIter = clientMethods.begin();
	for (; methodIter != clientMethods.end(); ++methodIter)
	{
		MethodDescription* pMethodDescription = methodIter->second;
		if (!writeEntityMethod(pEntityScriptDefModule, pCurrScriptDefModule, pMethodDescription, "#REPLACE#"))
			return false;

		std::string::size_type fHeaderPos = headerfileBody_.find("#REPLACE#");
		std::string::size_type fSourcePos = sourcefileBody_.find("#REPLACE#");
		KBE_ASSERT((fHeaderPos != std::string::npos) || (fSourcePos != std::string::npos));

		std::string argsBody = "";

		std::vector<DataType*>& argTypes = pMethodDescription->getArgTypes();
		std::vector<DataType*>::iterator iter = argTypes.begin();

		int i = 1;

		for (; iter != argTypes.end(); ++iter)
		{
			DataType* pDataType = (*iter);

			if (pDataType->type() == DATA_TYPE_FIXEDARRAY)
			{
				FixedArrayType* pFixedArrayType = static_cast<FixedArrayType*>(pDataType);
				
				std::string argsTypeBody;
				if (!writeEntityMethodArgs_ARRAY(pFixedArrayType, argsTypeBody, pFixedArrayType->aliasName()))
				{
					return false;
				}

				argsBody += fmt::format("{} arg{}, ", argsTypeBody, i++);
			}
			else if (pDataType->type() == DATA_TYPE_FIXEDDICT)
			{
				FixedDictType* pFixedDictType = static_cast<FixedDictType*>(pDataType);

				std::string argsTypeBody = typeToType(pFixedDictType->aliasName());
				if (!writeEntityMethodArgs_Const_Ref(pDataType, argsTypeBody))
				{
					return false;
				}

				argsBody += fmt::format("{} arg{}, ", argsTypeBody, i++);
			}
			else if (pDataType->type() != DATA_TYPE_DIGIT)
			{
				std::string argsTypeBody = typeToType(pDataType->getName());
				if (!writeEntityMethodArgs_Const_Ref(pDataType, argsTypeBody))
				{
					return false;
				}

				argsBody += fmt::format("{} arg{}, ", argsTypeBody, i++);
			}
			else
			{
				argsBody += fmt::format("{} arg{}, ", typeToType(pDataType->getName()), i++);
			}
		}

		if (argsBody.size() > 0)
		{
			argsBody.erase(argsBody.size() - 2, 2);
		}

		strutil::kbe_replace(headerfileBody_, "#REPLACE#", argsBody);
		strutil::kbe_replace(sourcefileBody_, "#REPLACE#", argsBody);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityMethodArgs_ARRAY(FixedArrayType* pFixedArrayType, std::string& stackArgsTypeBody, const std::string& childItemName)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityMethodArgs_ARRAY: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityMethodArgs_Const_Ref(DataType* pDataType, std::string& stackArgsTypeBody)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityMethodArgs_Const_Ref: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityMethod(ScriptDefModule* pEntityScriptDefModule,
	ScriptDefModule* pCurrScriptDefModule, MethodDescription* pMethodDescription, const char* fillString)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityMethod: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
bool ClientSDK::writeEntityProcessMessagesMethod(ScriptDefModule* pEntityScriptDefModule)
{
	ERROR_MSG(fmt::format("ClientSDK::writeEntityProcessMessagesMethod: Not Implemented!\n"));
	return false;
}

//-------------------------------------------------------------------------------------
}
