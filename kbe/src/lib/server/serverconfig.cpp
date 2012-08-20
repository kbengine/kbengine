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


#include "serverconfig.hpp"
#include "network/common.hpp"
#include "network/address.hpp"
#include "resmgr/resmgr.hpp"

namespace KBEngine{
KBE_SINGLETON_INIT(ServerConfig);

//-------------------------------------------------------------------------------------
ServerConfig::ServerConfig():
gameUpdateHertz_(10)
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
	XmlPlus* xml = new XmlPlus(Resmgr::matchRes(fileName).c_str());

	if(!xml->isGood())
	{
		ERROR_MSG(" ServerConfig::loadConfig: load %s is failed!\n", fileName.c_str());
		SAFE_RELEASE(xml);
		return false;
	}
	
	rootNode = xml->getRootNode("packetAlwaysContainLength");
	if(rootNode != NULL){
		Mercury::g_packetAlwaysContainLength = xml->getValInt(rootNode) != 0;
		rootNode = NULL;
	}

	rootNode = xml->getRootNode("trace_packet");
	if(rootNode != NULL)
	{
		Mercury::g_trace_packet = xml->getValInt(rootNode);
		rootNode = NULL;

		if(Mercury::g_trace_packet > 3)
			Mercury::g_trace_packet = 0;
	}

	rootNode = xml->getRootNode("gameUpdateHertz");
	if(rootNode != NULL){
		gameUpdateHertz_ = xml->getValInt(rootNode);
		rootNode = NULL;
	}

	rootNode = xml->getRootNode("cellapp");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_cellAppInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_cellAppInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME);
		
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
		node = xml->enterNode(rootNode, "entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "externalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.externalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "externalPorts_min");
		if(node != NULL)	
			_baseAppInfo.externalPorts_min = xml->getValInt(node);

		node = NULL;
		node = xml->enterNode(rootNode, "externalPorts_max");
		if(node != NULL)	
			_baseAppInfo.externalPorts_max = xml->getValInt(node);

		if(_baseAppInfo.externalPorts_min < 0)
			_baseAppInfo.externalPorts_min = 0;
		if(_baseAppInfo.externalPorts_max < _baseAppInfo.externalPorts_min)
			_baseAppInfo.externalPorts_max = _baseAppInfo.externalPorts_min;
	}

	rootNode = NULL;
	rootNode = xml->getRootNode("dbmgr");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "dbAccountEntityScriptType");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.dbAccountEntityScriptType, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "type");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.db_type, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "host");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.db_ip, xml->getValStr(node).c_str(), MAX_IP);

		node = NULL;
		node = xml->enterNode(rootNode, "port");	
		if(node != NULL)
			_dbmgrInfo.db_port = xml->getValInt(node);

		node = NULL;
		node = xml->enterNode(rootNode, "username");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.db_username, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "password");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.db_password, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "databaseName");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.db_name, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "numConnections");	
		if(node != NULL)
			_dbmgrInfo.db_numConnections = xml->getValInt(node);
	}

	rootNode = NULL;
	rootNode = xml->getRootNode("loginapp");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "externalInterface");	
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.externalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = NULL;
		node = xml->enterNode(rootNode, "externalPorts_min");
		if(node != NULL)	
			_loginAppInfo.externalPorts_min = xml->getValInt(node);

		node = NULL;
		node = xml->enterNode(rootNode, "externalPorts_max");
		if(node != NULL)	
			_loginAppInfo.externalPorts_max = xml->getValInt(node);

		if(_loginAppInfo.externalPorts_min < 0)
			_loginAppInfo.externalPorts_min = 0;
		if(_loginAppInfo.externalPorts_max < _loginAppInfo.externalPorts_min)
			_loginAppInfo.externalPorts_max = _loginAppInfo.externalPorts_min;
	}
	
	rootNode = NULL;
	rootNode = xml->getRootNode("cellappmgr");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_cellAppMgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);
	}
	
	rootNode = NULL;
	rootNode = xml->getRootNode("baseappmgr");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppMgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);
	}
	
	rootNode = NULL;
	rootNode = xml->getRootNode("kbmachine");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_kbMachineInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);
	}

	rootNode = NULL;
	rootNode = xml->getRootNode("kbcenter");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_kbCenterInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);
	}
	
	rootNode = NULL;
	rootNode = xml->getRootNode("bots");
	if(rootNode != NULL)
	{
		node = NULL;
		node = xml->enterNode(rootNode, "bots");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);
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
ENGINE_COMPONENT_INFO& ServerConfig::getBots(void)
{
	return _botsInfo;
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
void ServerConfig::updateInfos(bool isPrint, COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
							   const Mercury::Address& internalAddr, const Mercury::Address& externalAddr)
{
	if(componentType == CELLAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getCellApp();
		info.internalAddr = &internalAddr;
		info.externalAddr = &externalAddr;
		info.componentID = componentID;
		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG("\tgameUpdateHertz : %d\n", gameUpdateHertz());
			INFO_MSG("\tdefaultAoIRadius : %f\n", info.defaultAoIRadius);
			INFO_MSG("\tdefaultAoIHysteresisArea : %f\n", info.defaultAoIHysteresisArea);
			INFO_MSG("\tentryScriptFile : %s\n", info.entryScriptFile);
			INFO_MSG("\tinternalAddr : %s\n", internalAddr.c_str());
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG("\tcomponentID : %"PRAppID"\n", info.componentID);
		}
	}
	else if (componentType == BASEAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getBaseApp();
		info.internalAddr = const_cast<Mercury::Address*>(&internalAddr);
		info.externalAddr = const_cast<Mercury::Address*>(&externalAddr);
		info.componentID = componentID;
		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG("\tgameUpdateHertz : %d\n", gameUpdateHertz());
			INFO_MSG("\tdefaultAoIRadius : %f\n", info.defaultAoIRadius);
			INFO_MSG("\tdefaultAoIHysteresisArea : %f\n", info.defaultAoIHysteresisArea);
			INFO_MSG("\tentryScriptFile : %s\n", info.entryScriptFile);
			INFO_MSG("\tinternalAddr : %s\n", internalAddr.c_str());
			INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG("\tcomponentID : %"PRAppID"\n", info.componentID);
		}
	}
	else if (componentType == BASEAPPMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getBaseAppMgr();
		info.internalAddr = const_cast<Mercury::Address*>(&internalAddr);
		info.externalAddr = const_cast<Mercury::Address*>(&externalAddr);
		info.componentID = componentID;
		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG("\tinternalAddr : %s\n", internalAddr.c_str());
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG("\tcomponentID : %"PRAppID"\n", info.componentID);
		}
	}
	else if (componentType == CELLAPPMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getCellAppMgr();
		info.internalAddr = const_cast<Mercury::Address*>(&internalAddr);
		info.externalAddr = const_cast<Mercury::Address*>(&externalAddr);
		info.componentID = componentID;
		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG("\tinternalAddr : %s\n", internalAddr.c_str());
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG("\tcomponentID : %"PRAppID"\n", info.componentID);
		}
	}
	else if (componentType == DBMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getDBMgr();
		info.internalAddr = const_cast<Mercury::Address*>(&internalAddr);
		info.externalAddr = const_cast<Mercury::Address*>(&externalAddr);
		info.componentID = componentID;
		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG("\tinternalAddr : %s\n", internalAddr.c_str());
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG("\tcomponentID : %"PRAppID"\n", info.componentID);
		}
	}
	else if (componentType == LOGINAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getLoginApp();
		info.internalAddr = const_cast<Mercury::Address*>(&internalAddr);
		info.externalAddr = const_cast<Mercury::Address*>(&externalAddr);
		info.componentID = componentID;
		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG("\tinternalAddr : %s\n", internalAddr.c_str());
			INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG("\tcomponentID : %"PRAppID"\n", info.componentID);
		}
	}
	else if (componentType == MACHINE_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getKBMachine();
		info.internalAddr = const_cast<Mercury::Address*>(&internalAddr);
		info.externalAddr = const_cast<Mercury::Address*>(&externalAddr);
		info.componentID = componentID;
		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG("\tinternalAddr : %s\n", internalAddr.c_str());
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG("\tcomponentID : %"PRAppID"\n", info.componentID);
		}
	}
}

//-------------------------------------------------------------------------------------		
}
