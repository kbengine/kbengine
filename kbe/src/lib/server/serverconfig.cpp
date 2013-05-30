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
gameUpdateHertz_(10),
billingSystemAddr_(),
billingSystem_type_(""),
billingSystem_thirdpartyAccountServiceAddr_(""),
billingSystem_thirdpartyAccountServicePort_(80),
billingSystem_thirdpartyChargeServiceAddr_(""),
billingSystem_thirdpartyChargeServicePort_(80),
billingSystem_thirdpartyServiceCBPort_(0),
shutdown_time_(1)
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
	XmlPlus* xml = new XmlPlus(Resmgr::getSingleton().matchRes(fileName).c_str());

	if(!xml->isGood())
	{
		ERROR_MSG(boost::format("ServerConfig::loadConfig: load %1% is failed!\n") %
			fileName.c_str());

		SAFE_RELEASE(xml);
		return false;
	}
	
	rootNode = xml->getRootNode("packetAlwaysContainLength");
	if(rootNode != NULL){
		Mercury::g_packetAlwaysContainLength = xml->getValInt(rootNode) != 0;
	}

	rootNode = xml->getRootNode("trace_packet");
	if(rootNode != NULL)
	{
		TiXmlNode* childnode = xml->enterNode(rootNode, "debug_type");
		if(childnode)
			Mercury::g_trace_packet = xml->getValInt(childnode);

		if(Mercury::g_trace_packet > 3)
			Mercury::g_trace_packet = 0;

		childnode = xml->enterNode(rootNode, "use_logfile");
		if(childnode)
			Mercury::g_trace_packet_use_logfile = (xml->getValStr(childnode) == "true");

		childnode = xml->enterNode(rootNode, "disables");
		if(childnode)
		{
			do																				
			{	
				if(childnode->FirstChild() != NULL)
				{
					std::string c = childnode->FirstChild()->Value();
					c = strutil::kbe_trim(c);
					if(c.size() > 0)
					{
						Mercury::g_trace_packet_disables.push_back(c);
					}
				}
			}while((childnode = childnode->NextSibling()));												
		}
	}

	rootNode = xml->getRootNode("debugEntity");
	if(rootNode != NULL)
	{
		g_debugEntity = xml->getValInt(rootNode) > 0;
	}

	rootNode = xml->getRootNode("app_publish");
	if(rootNode != NULL)
	{
		g_appPublish = xml->getValInt(rootNode);
	}

	rootNode = xml->getRootNode("shutdown_time");
	if(rootNode != NULL)
	{
		shutdown_time_ = xml->getValInt(rootNode);
	}
	
	rootNode = xml->getRootNode("channelCommon");
	if(rootNode != NULL)
	{
		TiXmlNode* childnode = xml->enterNode(rootNode, "timeout");
		if(childnode)
		{
			TiXmlNode* childnode1 = xml->enterNode(childnode, "internal");
			if(childnode1)
			{
				channelCommon_.channelInternalTimeout = KBE_MAX(1.f, float(xml->getValFloat(childnode1)));
				Mercury::g_channelInternalTimeout = channelCommon_.channelInternalTimeout;
			}

			childnode1 = xml->enterNode(childnode, "external");
			if(childnode)
			{
				channelCommon_.channelExternalTimeout = KBE_MAX(1.f, float(xml->getValFloat(childnode1)));
				Mercury::g_channelExternalTimeout = channelCommon_.channelExternalTimeout;
			}
		}

		childnode = xml->enterNode(rootNode, "readBufferSize");
		if(childnode)
		{
			TiXmlNode* childnode1 = xml->enterNode(childnode, "internal");
			if(childnode1)
				channelCommon_.intReadBufferSize = KBE_MAX(0, xml->getValInt(childnode1));

			childnode1 = xml->enterNode(childnode, "external");
			if(childnode1)
				channelCommon_.extReadBufferSize = KBE_MAX(0, xml->getValInt(childnode1));
		}

		childnode = xml->enterNode(rootNode, "writeBufferSize");
		if(childnode)
		{
			TiXmlNode* childnode1 = xml->enterNode(childnode, "internal");
			if(childnode1)
				channelCommon_.intWriteBufferSize = KBE_MAX(0, xml->getValInt(childnode1));

			childnode1 = xml->enterNode(childnode, "external");
			if(childnode1)
				channelCommon_.extWriteBufferSize = KBE_MAX(0, xml->getValInt(childnode1));
		}

		childnode = xml->enterNode(rootNode, "receiveWindowOverflow");
		if(childnode)
		{
			TiXmlNode* childnode1 = xml->enterNode(childnode, "messages");
			if(childnode1)
			{
				childnode1 = xml->enterNode(childnode, "internal");
				if(childnode1)
					Mercury::g_intReceiveWindowMessagesOverflow = KBE_MAX(16, xml->getValInt(childnode1));

				childnode1 = xml->enterNode(childnode, "external");
				if(childnode1)
					Mercury::g_extReceiveWindowMessagesOverflow = KBE_MAX(16, xml->getValInt(childnode1));
			}

			childnode1 = xml->enterNode(childnode, "bytes");
			if(childnode1)
			{
				childnode1 = xml->enterNode(childnode, "internal");
				if(childnode1)
					Mercury::g_intReceiveWindowBytesOverflow = KBE_MAX(16, xml->getValInt(childnode1));

				childnode1 = xml->enterNode(childnode, "external");
				if(childnode1)
					Mercury::g_extReceiveWindowBytesOverflow = KBE_MAX(16, xml->getValInt(childnode1));
			}
		};

		childnode = xml->enterNode(rootNode, "encrypt_type");
		if(childnode)
		{
			Mercury::g_channelExternalEncryptType = xml->getValInt(childnode);
		}
	}

	rootNode = xml->getRootNode("gameUpdateHertz");
	if(rootNode != NULL){
		gameUpdateHertz_ = xml->getValInt(rootNode);
	}

	rootNode = xml->getRootNode("bitsPerSecondToClient");
	if(rootNode != NULL){
		bitsPerSecondToClient_ = xml->getValInt(rootNode);
	}

	rootNode = xml->getRootNode("billingSystem");
	if(rootNode != NULL)
	{
		TiXmlNode* childnode = xml->enterNode(rootNode, "type");
		if(childnode)
		{
			billingSystem_type_ = xml->getValStr(childnode);
			if(billingSystem_type_.size() == 0)
				billingSystem_type_ = "normal";
		}

		std::string ip = "";
		childnode = xml->enterNode(rootNode, "host");
		if(childnode)
		{
			ip = xml->getValStr(childnode);
			if(ip.size() == 0)
				ip = "localhost";

			Mercury::Address addr(ip, ntohs(billingSystemAddr_.port));
			billingSystemAddr_ = addr;
		}

		uint16 port = 0;
		childnode = xml->enterNode(rootNode, "port");
		if(childnode)
		{
			port = xml->getValInt(childnode);

			if(port <= 0)
				port = KBE_BILLING_TCP_PORT;

			Mercury::Address addr(inet_ntoa((struct in_addr&)billingSystemAddr_.ip), port);
			billingSystemAddr_ = addr;
		}

		childnode = xml->enterNode(rootNode, "thirdpartyAccountService_addr");
		if(childnode)
		{
			billingSystem_thirdpartyAccountServiceAddr_ = xml->getValStr(childnode);
		}

		childnode = xml->enterNode(rootNode, "thirdpartyAccountService_port");
		if(childnode)
		{
			billingSystem_thirdpartyAccountServicePort_ = xml->getValInt(childnode);
		}
		
		childnode = xml->enterNode(rootNode, "thirdpartyChargeService_addr");
		if(childnode)
		{
			billingSystem_thirdpartyChargeServiceAddr_ = xml->getValStr(childnode);
		}

		childnode = xml->enterNode(rootNode, "thirdpartyChargeService_port");
		if(childnode)
		{
			billingSystem_thirdpartyChargeServicePort_ = xml->getValInt(childnode);
		}

		childnode = xml->enterNode(rootNode, "thirdpartyService_cbport");
		if(childnode)
		{
			billingSystem_thirdpartyServiceCBPort_ = xml->getValInt(childnode);
		}

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_billingInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "orders_timeout");
		if(node != NULL){
			billingSystem_orders_timeout_ = xml->getValInt(node);
		}
	}

	rootNode = xml->getRootNode("cellapp");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_cellAppInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_cellAppInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME);
		
		TiXmlNode* aoiNode = xml->enterNode(rootNode, "defaultAoIRadius");
		if(aoiNode != NULL)
		{
			node = NULL;
			node = xml->enterNode(aoiNode, "radius");
			if(node != NULL)
				_cellAppInfo.defaultAoIRadius = float(xml->getValFloat(node));
					
			node = xml->enterNode(aoiNode, "hysteresisArea");
			if(node != NULL)
				_cellAppInfo.defaultAoIHysteresisArea = float(xml->getValFloat(node));
		}
			
		node = xml->enterNode(rootNode, "ids");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "criticallyLowSize");
			if(childnode)
			{
				_cellAppInfo.criticallyLowSize = xml->getValInt(childnode);
				if(_cellAppInfo.criticallyLowSize < 100)
					_cellAppInfo.criticallyLowSize = 100;
			}
		}
		
		node = xml->enterNode(rootNode, "profiles");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "cprofile");
			if(childnode)
			{
				_cellAppInfo.profiles.open_cprofile = (xml->getValStr(childnode) == "true");
			}

			childnode = xml->enterNode(node, "pyprofile");
			if(childnode)
			{
				_cellAppInfo.profiles.open_pyprofile = (xml->getValStr(childnode) == "true");
			}

			childnode = xml->enterNode(node, "eventprofile");
			if(childnode)
			{
				_cellAppInfo.profiles.open_eventprofile = (xml->getValStr(childnode) == "true");
			}

			childnode = xml->enterNode(node, "mercuryprofile");
			if(childnode)
			{
				_cellAppInfo.profiles.open_mercuryprofile = (xml->getValStr(childnode) == "true");
			}
		}

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_cellAppInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "ghostDistance");
		if(node != NULL){
			_cellAppInfo.ghostDistance = (float)xml->getValFloat(node);
		}

		node = xml->enterNode(rootNode, "ghostingMaxPerCheck");
		if(node != NULL){
			_cellAppInfo.ghostingMaxPerCheck = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "ghostUpdateHertz");
		if(node != NULL){
			_cellAppInfo.ghostUpdateHertz = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "coordinate_system");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "enable");
			if(childnode)
			{
				_cellAppInfo.use_coordinate_system = (xml->getValStr(childnode) == "true");
			}
			
			childnode = xml->enterNode(node, "rangemgr_y");
			if(childnode)
			{
				_cellAppInfo.rangelist_hasY = (xml->getValStr(childnode) == "true");
			}
		}

		node = xml->enterNode(rootNode, "telnet_service");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "port");
			if(childnode)
			{
				_cellAppInfo.telnet_port = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "password");
			if(childnode)
			{
				_cellAppInfo.telnet_passwd = xml->getValStr(childnode);
			}

			childnode = xml->enterNode(node, "default_layer");
			if(childnode)
			{
				_cellAppInfo.telnet_deflayer = xml->getValStr(childnode);
			}
		}
	}
	
	rootNode = xml->getRootNode("baseapp");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "externalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.externalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "externalPorts_min");
		if(node != NULL)	
			_baseAppInfo.externalPorts_min = xml->getValInt(node);

		node = xml->enterNode(rootNode, "externalPorts_max");
		if(node != NULL)	
			_baseAppInfo.externalPorts_max = xml->getValInt(node);

		if(_baseAppInfo.externalPorts_min < 0)
			_baseAppInfo.externalPorts_min = 0;
		if(_baseAppInfo.externalPorts_max < _baseAppInfo.externalPorts_min)
			_baseAppInfo.externalPorts_max = _baseAppInfo.externalPorts_min;

		node = xml->enterNode(rootNode, "archivePeriod");
		if(node != NULL)
			_baseAppInfo.archivePeriod = float(xml->getValFloat(node));
				
		node = xml->enterNode(rootNode, "backupPeriod");
		if(node != NULL)
			_baseAppInfo.backupPeriod = float(xml->getValFloat(node));
		
		node = xml->enterNode(rootNode, "backUpUndefinedProperties");
		if(node != NULL)
			_baseAppInfo.backUpUndefinedProperties = xml->getValInt(node) > 0;
			
		node = xml->enterNode(rootNode, "loadSmoothingBias");
		if(node != NULL)
			_baseAppInfo.loadSmoothingBias = float(xml->getValFloat(node));
		
		node = xml->enterNode(rootNode, "ids");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "criticallyLowSize");
			if(childnode)
			{
				_baseAppInfo.criticallyLowSize = xml->getValInt(childnode);
				if(_baseAppInfo.criticallyLowSize < 100)
					_baseAppInfo.criticallyLowSize = 100;
			}
		}
		
		node = xml->enterNode(rootNode, "downloadStreaming");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "bitsPerSecondTotal");
			if(childnode)
				_baseAppInfo.downloadBitsPerSecondTotal = xml->getValInt(childnode);

			childnode = xml->enterNode(node, "bitsPerSecondPerClient");
			if(childnode)
				_baseAppInfo.downloadBitsPerSecondPerClient = xml->getValInt(childnode);
		}
	
		node = xml->enterNode(rootNode, "profiles");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "cprofile");
			if(childnode)
			{
				_baseAppInfo.profiles.open_cprofile = (xml->getValStr(childnode) == "true");
			}

			childnode = xml->enterNode(node, "pyprofile");
			if(childnode)
			{
				_baseAppInfo.profiles.open_pyprofile = (xml->getValStr(childnode) == "true");
			}

			childnode = xml->enterNode(node, "eventprofile");
			if(childnode)
			{
				_baseAppInfo.profiles.open_eventprofile = (xml->getValStr(childnode) == "true");
			}

			childnode = xml->enterNode(node, "mercuryprofile");
			if(childnode)
			{
				_baseAppInfo.profiles.open_mercuryprofile = (xml->getValStr(childnode) == "true");
			}
		}

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_baseAppInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "entityRestoreSize");
		if(node != NULL){
			_baseAppInfo.entityRestoreSize = xml->getValInt(node);
		}
		
		if(_baseAppInfo.entityRestoreSize <= 0)
			_baseAppInfo.entityRestoreSize = 32;

		node = xml->enterNode(rootNode, "telnet_service");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "port");
			if(childnode)
			{
				_baseAppInfo.telnet_port = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "password");
			if(childnode)
			{
				_baseAppInfo.telnet_passwd = xml->getValStr(childnode);
			}

			childnode = xml->enterNode(node, "default_layer");
			if(childnode)
			{
				_baseAppInfo.telnet_deflayer = xml->getValStr(childnode);
			}
		}
	}

	rootNode = xml->getRootNode("dbmgr");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "dbAccountEntityScriptType");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.dbAccountEntityScriptType, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "type");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.db_type, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "host");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.db_ip, xml->getValStr(node).c_str(), MAX_IP);

		node = xml->enterNode(rootNode, "port");	
		if(node != NULL)
			_dbmgrInfo.db_port = xml->getValInt(node);	

		node = xml->enterNode(rootNode, "auth");	
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "password");
			if(childnode)
			{
				strncpy((char*)&_dbmgrInfo.db_password, xml->getValStr(childnode).c_str(), MAX_BUF * 10);
			}

			childnode = xml->enterNode(node, "username");
			if(childnode)
			{
				strncpy((char*)&_dbmgrInfo.db_username, xml->getValStr(childnode).c_str(), MAX_NAME);
			}

			childnode = xml->enterNode(node, "encrypt");
			if(childnode)
			{
				_dbmgrInfo.db_passwordEncrypt = xml->getValStr(childnode) == "true";
			}
		}
		
		node = xml->enterNode(rootNode, "databaseName");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.db_name, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "numConnections");	
		if(node != NULL)
			_dbmgrInfo.db_numConnections = xml->getValInt(node);
		
		node = xml->enterNode(rootNode, "unicodeString");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "characterSet");
			if(childnode)
			{
				_dbmgrInfo.db_unicodeString_characterSet = xml->getValStr(childnode);
			}

			childnode = xml->enterNode(node, "collation");
			if(childnode)
			{
				_dbmgrInfo.db_unicodeString_collation = xml->getValStr(childnode);
			}
		}

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_dbmgrInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "notFoundAccountAutoCreate");
		if(node != NULL){
			_dbmgrInfo.notFoundAccountAutoCreate = (xml->getValStr(node) == "true");
		}
		
		node = xml->enterNode(rootNode, "allowEmptyDigest");
		if(node != NULL){
			_dbmgrInfo.allowEmptyDigest = (xml->getValStr(node) == "true");
		}
	}

	if(_dbmgrInfo.db_unicodeString_characterSet.size() == 0)
		_dbmgrInfo.db_unicodeString_characterSet = "utf8";

	if(_dbmgrInfo.db_unicodeString_collation.size() == 0)
		_dbmgrInfo.db_unicodeString_collation = "utf8_bin";

	rootNode = xml->getRootNode("loginapp");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "externalInterface");	
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.externalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "externalPorts_min");
		if(node != NULL)	
			_loginAppInfo.externalPorts_min = xml->getValInt(node);

		node = xml->enterNode(rootNode, "externalPorts_max");
		if(node != NULL)	
			_loginAppInfo.externalPorts_max = xml->getValInt(node);

		if(_loginAppInfo.externalPorts_min < 0)
			_loginAppInfo.externalPorts_min = 0;
		if(_loginAppInfo.externalPorts_max < _loginAppInfo.externalPorts_min)
			_loginAppInfo.externalPorts_max = _loginAppInfo.externalPorts_min;

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_loginAppInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "encrypt_login");
		if(node != NULL){
			_loginAppInfo.encrypt_login = xml->getValInt(node);
		}
	}
	
	rootNode = xml->getRootNode("cellappmgr");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_cellAppMgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_cellAppMgrInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}
	}
	
	rootNode = xml->getRootNode("baseappmgr");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppMgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_baseAppMgrInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}
	}
	
	rootNode = xml->getRootNode("kbmachine");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_kbMachineInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "externalInterface");	
		if(node != NULL)
			strncpy((char*)&_kbMachineInfo.externalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "externalPorts_min");
		if(node != NULL)	
			_kbMachineInfo.externalPorts_min = xml->getValInt(node);

		node = xml->enterNode(rootNode, "externalPorts_max");
		if(node != NULL)	
			_kbMachineInfo.externalPorts_max = xml->getValInt(node);

		if(_kbMachineInfo.externalPorts_min < 0)
			_kbMachineInfo.externalPorts_min = 0;
		if(_kbMachineInfo.externalPorts_max < _kbMachineInfo.externalPorts_min)
			_kbMachineInfo.externalPorts_max = _kbMachineInfo.externalPorts_min;

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_kbMachineInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}
	}

	rootNode = xml->getRootNode("kbcenter");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_kbCenterInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_kbCenterInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}
	}
	
	rootNode = xml->getRootNode("bots");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "bots");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "host");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.login_ip, xml->getValStr(node).c_str(), MAX_IP);

		node = xml->enterNode(rootNode, "port");	
		if(node != NULL)
			_botsInfo.login_port = xml->getValInt(node);

		node = xml->enterNode(rootNode, "defaultAddBots");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "totalcount");
			if(childnode)
			{
				_botsInfo.defaultAddBots_totalCount = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "tickcount");
			if(childnode)
			{
				_botsInfo.defaultAddBots_tickCount = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "ticktime");
			if(childnode)
			{
				_botsInfo.defaultAddBots_tickTime = (float)xml->getValFloat(childnode);
			}
		}

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_botsInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}
	}

	rootNode = xml->getRootNode("messagelog");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "messagelog");	
		if(node != NULL)
			strncpy((char*)&_messagelogInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_messagelogInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}
	}

	rootNode = xml->getRootNode("resourcemgr");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "resourcemgr");	
		if(node != NULL)
			strncpy((char*)&_resourcemgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_resourcemgrInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}
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
ENGINE_COMPONENT_INFO& ServerConfig::getResourcemgr(void)
{
	return _resourcemgrInfo;
}

//-------------------------------------------------------------------------------------		
ENGINE_COMPONENT_INFO& ServerConfig::getMessagelog(void)
{
	return _messagelogInfo;
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
	case RESOURCEMGR_TYPE:
		return getResourcemgr();
	case MESSAGELOG_TYPE:
		return getMessagelog();
	default:
		return getCellApp();
	};

	return getBaseApp();	
}

//-------------------------------------------------------------------------------------	
uint32 ServerConfig::tcp_SOMAXCONN(COMPONENT_TYPE componentType)
{
	ENGINE_COMPONENT_INFO& cinfo = getComponent(componentType);
	return cinfo.tcp_SOMAXCONN;
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
			INFO_MSG(boost::format("\tgameUpdateHertz : %1%\n") % gameUpdateHertz());
			INFO_MSG(boost::format("\tdefaultAoIRadius : %1%\n") % info.defaultAoIRadius);
			INFO_MSG(boost::format("\tdefaultAoIHysteresisArea : %1%\n") % info.defaultAoIHysteresisArea);
			INFO_MSG(boost::format("\tentryScriptFile : %1%\n") % info.entryScriptFile);
			INFO_MSG(boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str());
			//INFO_MSG(boost::format("\texternalAddr : %1%\n") % externalAddr.c_str());
			INFO_MSG(boost::format("\tcomponentID : %1%\n") % info.componentID);
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
			INFO_MSG(boost::format("\tgameUpdateHertz : %1%\n") % gameUpdateHertz());
			INFO_MSG(boost::format("\tdefaultAoIRadius : %1%\n") % info.defaultAoIRadius);
			INFO_MSG(boost::format("\tdefaultAoIHysteresisArea : %1%\n") % info.defaultAoIHysteresisArea);
			INFO_MSG(boost::format("\tentryScriptFile : %1%\n") % info.entryScriptFile);
			INFO_MSG(boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str());
			INFO_MSG(boost::format("\texternalAddr : %1%\n") % externalAddr.c_str());
			INFO_MSG(boost::format("\tcomponentID : %1%\n") % info.componentID);
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
			INFO_MSG(boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str());
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG(boost::format("\tcomponentID : %1%\n") % info.componentID);
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
			INFO_MSG(boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str());
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG(boost::format("\tcomponentID : %1%\n") % info.componentID);
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
			INFO_MSG(boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str());
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG(boost::format("\tcomponentID : %1%\n") % info.componentID);
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
			INFO_MSG(boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str());
			INFO_MSG(boost::format("\texternalAddr : %1%\n") % externalAddr.c_str());
			INFO_MSG(boost::format("\tcomponentID : %1%\n") % info.componentID);
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
			INFO_MSG(boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str());
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG(boost::format("\tcomponentID : %1%\n") % info.componentID);
		}
	}
}

//-------------------------------------------------------------------------------------		
}
