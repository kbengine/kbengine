#include "serverconfig.hpp"
namespace KBEngine{
template<> ServerConfig* Singleton<ServerConfig>::singleton_ = 0;

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

	rootNode = xml->getRootNode("cellapp");
	node = xml->enterNode(rootNode, "port");	
	_cellAppInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");	
	strncpy((char*)&_cellAppInfo.ip, xml->getValStr(node).c_str(), 50);
	node = xml->enterNode(rootNode, "entryScriptFile");	
	strncpy((char*)&_cellAppInfo.entryScriptFile, xml->getValStr(node).c_str(), 255);
	TiXmlNode* aoiNode = xml->enterNode(rootNode, "defaultAoIRadius");
	node = xml->enterNode(aoiNode, "radius");
	_cellAppInfo.defaultAoIRadius = float(xml->getValFloat(node));
	node = xml->enterNode(aoiNode, "hysteresisArea");
	_cellAppInfo.defaultAoIHysteresisArea = float(xml->getValFloat(node));
	
	rootNode = xml->getRootNode("baseapp");
	node = xml->enterNode(rootNode, "port");	
	_baseAppInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_baseAppInfo.ip, xml->getValStr(node).c_str(), 50);
	node = xml->enterNode(rootNode, "entryScriptFile");	
	strncpy((char*)&_baseAppInfo.entryScriptFile, xml->getValStr(node).c_str(), 255);
	
	rootNode = xml->getRootNode("dbmgr");
	node = xml->enterNode(rootNode, "port");	
	_dbmgrInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_dbmgrInfo.ip, xml->getValStr(node).c_str(), 50);
	node = xml->enterNode(rootNode, "dbAccountEntityScriptType");	
	strncpy((char*)&_dbmgrInfo.dbAccountEntityScriptType, xml->getValStr(node).c_str(), 255);
	
	rootNode = xml->getRootNode("loginapp");
	node = xml->enterNode(rootNode, "port");
	_loginAppInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_loginAppInfo.ip, xml->getValStr(node).c_str(), 50);
	
	rootNode = xml->getRootNode("cellappmgr");
	node = xml->enterNode(rootNode, "port");
	_cellAppMgrInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_cellAppMgrInfo.ip, xml->getValStr(node).c_str(), 50);
	
	rootNode = xml->getRootNode("baseappmgr");
	node = xml->enterNode(rootNode, "port");
	_baseAppMgrInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_baseAppMgrInfo.ip, xml->getValStr(node).c_str(), 50);

	rootNode = xml->getRootNode("kbmachine");
	node = xml->enterNode(rootNode, "port");
	_kbMachineInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_kbMachineInfo.ip, xml->getValStr(node).c_str(), 50);

	rootNode = xml->getRootNode("kbcenter");
	node = xml->enterNode(rootNode, "port");
	_kbCenterInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_kbCenterInfo.ip, xml->getValStr(node).c_str(), 50);
	
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
}
