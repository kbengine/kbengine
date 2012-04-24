#include "serverconfig.hpp"
namespace KBEngine{
KBE_SINGLETON_INIT(ServerConfig);

//-------------------------------------------------------------------------------------
ServerConfig::ServerConfig()
{
}

//-------------------------------------------------------------------------------------
ServerConfig::~ServerConfig()
{
}

//-------------------------------------------------------------------------------------
bool ServerConfig::loadConfig(std::string fileName)
{
	TiXmlNode* node = NULL, *rootNode = NULL;
	XmlPlus* xml = new XmlPlus(fileName.c_str());

	rootNode = xml->getRootNode("gameUpdateHertz");
	if(rootNode != NULL){
		gameUpdateHertz_ = xml->getValInt(rootNode);
		rootNode = NULL;
	}

	rootNode = xml->getRootNode("cellapp");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "port");
		if(node != NULL)
			_cellAppInfo.port = xml->getValInt(node);
		
		node = NULL;
		node = xml->enterNode(rootNode, "ip");	
		if(node != NULL)
			strncpy((char*)&_cellAppInfo.ip, xml->getValStr(node).c_str(), 50);
		
		node = NULL;
		node = xml->enterNode(rootNode, "entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_cellAppInfo.entryScriptFile, xml->getValStr(node).c_str(), 255);
		
		node = NULL;
		TiXmlNode* aoiNode = xml->enterNode(rootNode, "defaultAoIRadius");
		if(aoiNode != NULL)
		{
			node = NULL;
			node = xml->enterNode(aoiNode, "radius");
			if(node != NULL)
				_cellAppInfo.defaultAoIRadius = float(xml->getValFloat(node));
			
			node = NULL;			
			node = xml->enterNode(aoiNode, "hysteresisArea");
			if(node != NULL)
				_cellAppInfo.defaultAoIHysteresisArea = float(xml->getValFloat(node));
		}
	}
	
	rootNode = NULL;
	rootNode = xml->getRootNode("baseapp");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "port");
		if(node != NULL)	
			_baseAppInfo.port = xml->getValInt(node);
		
		node = NULL;
		node = xml->enterNode(rootNode, "ip");
		if(node != NULL)	
			strncpy((char*)&_baseAppInfo.ip, xml->getValStr(node).c_str(), 50);
		
		node = NULL;
		node = xml->enterNode(rootNode, "entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.entryScriptFile, xml->getValStr(node).c_str(), 255);
	}

	rootNode = NULL;
	rootNode = xml->getRootNode("dbmgr");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "port");	
		if(node != NULL)
			_dbmgrInfo.port = xml->getValInt(node);
		
		node = NULL;
		node = xml->enterNode(rootNode, "ip");
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.ip, xml->getValStr(node).c_str(), 50);
		
		node = NULL;
		node = xml->enterNode(rootNode, "dbAccountEntityScriptType");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.dbAccountEntityScriptType, xml->getValStr(node).c_str(), 255);
	}

	rootNode = NULL;
	rootNode = xml->getRootNode("loginapp");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "port");
		if(node != NULL)
			_loginAppInfo.port = xml->getValInt(node);
		
		node = NULL;
		node = xml->enterNode(rootNode, "ip");
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.ip, xml->getValStr(node).c_str(), 50);
	}
	
	rootNode = NULL;
	rootNode = xml->getRootNode("cellappmgr");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "port");
		if(node != NULL)
			_cellAppMgrInfo.port = xml->getValInt(node);
		
		node = NULL;
		node = xml->enterNode(rootNode, "ip");
		if(node != NULL)
			strncpy((char*)&_cellAppMgrInfo.ip, xml->getValStr(node).c_str(), 50);
	}
	
	rootNode = NULL;
	rootNode = xml->getRootNode("baseappmgr");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "port");
		if(node != NULL)
			_baseAppMgrInfo.port = xml->getValInt(node);
		
		node = NULL;
		node = xml->enterNode(rootNode, "ip");
		if(node != NULL)
			strncpy((char*)&_baseAppMgrInfo.ip, xml->getValStr(node).c_str(), 50);
	}
	
	rootNode = NULL;
	rootNode = xml->getRootNode("kbmachine");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "port");
		if(node != NULL)
			_kbMachineInfo.port = xml->getValInt(node);
		
		node = NULL;
		node = xml->enterNode(rootNode, "ip");
		if(node != NULL)
			strncpy((char*)&_kbMachineInfo.ip, xml->getValStr(node).c_str(), 50);
	}

	rootNode = NULL;
	rootNode = xml->getRootNode("kbcenter");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "port");
		if(node != NULL)
			_kbCenterInfo.port = xml->getValInt(node);
		
		node = NULL;
		node = xml->enterNode(rootNode, "ip");
		if(node != NULL)
			strncpy((char*)&_kbCenterInfo.ip, xml->getValStr(node).c_str(), 50);
	}
	
	SAFE_RELEASE(xml);
	return true;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getCellApp(void)
{
	return _cellAppInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getBaseApp(void)
{
	return _baseAppInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getDBMgr(void)
{
	return _dbmgrInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getLoginApp(void)
{
	return _loginAppInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getCellAppMgr(void)
{
	return _cellAppMgrInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getBaseAppMgr(void)
{
	return _baseAppMgrInfo;
}

//-------------------------------------------------------------------------------------		
ENGINE_COMPONENT_INFO& ServerConfig::getKBMachine(void)
{
	return _kbMachineInfo;
}

//-------------------------------------------------------------------------------------		
ENGINE_COMPONENT_INFO& ServerConfig::getKBCenter(void)
{
	return _kbCenterInfo;
}

//-------------------------------------------------------------------------------------	
ENGINE_COMPONENT_INFO& ServerConfig::getComponent(COMPONENT_TYPE componentType)
{
	switch(componentType)
	{
	case DBMGR_TYPE:
		return getDBMgr();
	case LOGINAPP_TYPE:
		return getLoginApp();
	case BASEAPPMGR_TYPE:
		return getBaseAppMgr();
	case CELLAPPMGR_TYPE:
		return getCellAppMgr();
	case CELLAPP_TYPE:
		return getCellApp();
	case BASEAPP_TYPE:
		return getBaseApp();
	case MACHINE_TYPE:
		return getKBMachine();
	case CENTER_TYPE:
		return getKBCenter();
	default:
		return getCellApp();
	};

	return getBaseApp();	
}

//-------------------------------------------------------------------------------------	
void ServerConfig::printCellapp(void)
{
	ENGINE_COMPONENT_INFO info = getCellApp();
	INFO_MSG("server-configs:\n");
	INFO_MSG("\tgameUpdateHertz : %d\n", gameUpdateHertz());
	INFO_MSG("\tdefaultAoIRadius : %f\n", info.defaultAoIRadius);
	INFO_MSG("\tdefaultAoIHysteresisArea : %f\n", info.defaultAoIHysteresisArea);
	INFO_MSG("\tentryScriptFile : %s\n", info.entryScriptFile);
}

//-------------------------------------------------------------------------------------		
}
