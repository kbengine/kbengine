#include "serverconfig.hpp"
namespace KBEngine{
template<> ServerConfig* Singleton<ServerConfig>::m_singleton_ = 0;

//-------------------------------------------------------------------------------------
ServerConfig::ServerConfig()
{
	loadConfig("../../res/server/KBEngineDefault.xml");
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
	_m_cellAppInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");	
	strncpy((char*)&_m_cellAppInfo.ip, xml->getValStr(node).c_str(), 50);
	node = xml->enterNode(rootNode, "entryScriptFile");	
	strncpy((char*)&_m_cellAppInfo.entryScriptFile, xml->getValStr(node).c_str(), 255);
	TiXmlNode* aoiNode = xml->enterNode(rootNode, "defaultAoIRadius");
	node = xml->enterNode(aoiNode, "radius");
	_m_cellAppInfo.defaultAoIRadius = float(xml->getValFloat(node));
	node = xml->enterNode(aoiNode, "hysteresisArea");
	_m_cellAppInfo.defaultAoIHysteresisArea = float(xml->getValFloat(node));
	
	rootNode = xml->getRootNode("baseapp");
	node = xml->enterNode(rootNode, "port");	
	_m_baseAppInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_m_baseAppInfo.ip, xml->getValStr(node).c_str(), 50);
	node = xml->enterNode(rootNode, "entryScriptFile");	
	strncpy((char*)&_m_baseAppInfo.entryScriptFile, xml->getValStr(node).c_str(), 255);
	
	rootNode = xml->getRootNode("dbmgr");
	node = xml->enterNode(rootNode, "port");	
	_m_dbmgrInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_m_dbmgrInfo.ip, xml->getValStr(node).c_str(), 50);
	node = xml->enterNode(rootNode, "dbAccountEntityScriptType");	
	strncpy((char*)&_m_dbmgrInfo.dbAccountEntityScriptType, xml->getValStr(node).c_str(), 255);
	
	rootNode = xml->getRootNode("loginapp");
	node = xml->enterNode(rootNode, "port");
	_m_loginAppInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_m_loginAppInfo.ip, xml->getValStr(node).c_str(), 50);
	
	rootNode = xml->getRootNode("cellappmgr");
	node = xml->enterNode(rootNode, "port");
	_m_cellAppMgrInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_m_cellAppMgrInfo.ip, xml->getValStr(node).c_str(), 50);
	
	rootNode = xml->getRootNode("baseappmgr");
	node = xml->enterNode(rootNode, "port");
	_m_baseAppMgrInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_m_baseAppMgrInfo.ip, xml->getValStr(node).c_str(), 50);

	rootNode = xml->getRootNode("kbmachine");
	node = xml->enterNode(rootNode, "port");
	_m_kbMachineInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_m_kbMachineInfo.ip, xml->getValStr(node).c_str(), 50);

	rootNode = xml->getRootNode("kbcenter");
	node = xml->enterNode(rootNode, "port");
	_m_kbCenterInfo.port = xml->getValInt(node);
	node = xml->enterNode(rootNode, "ip");
	strncpy((char*)&_m_kbCenterInfo.ip, xml->getValStr(node).c_str(), 50);
	
	SAFE_RELEASE(xml);
	return true;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getCellApp(void)
{
	return _m_cellAppInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getBaseApp(void)
{
	return _m_baseAppInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getDBMgr(void)
{
	return _m_dbmgrInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getLoginApp(void)
{
	return _m_loginAppInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getCellAppMgr(void)
{
	return _m_cellAppMgrInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getBaseAppMgr(void)
{
	return _m_baseAppMgrInfo;
}

//-------------------------------------------------------------------------------------		
ENGINE_COMPONENT_INFO& ServerConfig::getKBMachine(void)
{
	return _m_kbMachineInfo;
}

//-------------------------------------------------------------------------------------		
ENGINE_COMPONENT_INFO& ServerConfig::getKBCenter(void)
{
	return _m_kbCenterInfo;
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
	};

	return getBaseApp();	
}

//-------------------------------------------------------------------------------------		
}
