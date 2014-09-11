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
#include "cstdkbe/kbekey.hpp"
#include "cstdkbe/kbeversion.hpp"

#ifndef CODE_INLINE
#include "serverconfig.ipp"
#endif

namespace KBEngine{
KBE_SINGLETON_INIT(ServerConfig);

//-------------------------------------------------------------------------------------
ServerConfig::ServerConfig():
gameUpdateHertz_(10),
tick_max_buffered_logs_(4096),
tick_max_sync_logs_(32),
billingSystemAddr_(),
billingSystem_accountType_(""),
billingSystem_chargeType_(""),
billingSystem_thirdpartyAccountServiceAddr_(""),
billingSystem_thirdpartyAccountServicePort_(80),
billingSystem_thirdpartyChargeServiceAddr_(""),
billingSystem_thirdpartyChargeServicePort_(80),
billingSystem_thirdpartyServiceCBPort_(0),
shutdown_time_(1.f),
shutdown_waitTickTime_(1.f),
callback_timeout_(180.f),
thread_timeout_(300.f),
thread_init_create_(1),
thread_pre_create_(2),
thread_max_create_(8)
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
	
	std::string email_service_config;
	rootNode = xml->getRootNode("email_service_config");
	if(rootNode != NULL)
	{
		email_service_config = xml->getValStr(rootNode);
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

	rootNode = xml->getRootNode("publish");
	if(rootNode != NULL)
	{
		TiXmlNode* childnode = xml->enterNode(rootNode, "state");
		if(childnode)
		{
			g_appPublish = xml->getValInt(childnode);
		}

		childnode = xml->enterNode(rootNode, "script_version");
		if(childnode)
		{
			KBEVersion::setScriptVersion(xml->getValStr(childnode));
		}
	}

	rootNode = xml->getRootNode("shutdown_time");
	if(rootNode != NULL)
	{
		shutdown_time_ = float(xml->getValFloat(rootNode));
	}
	
	rootNode = xml->getRootNode("shutdown_waittick");
	if(rootNode != NULL)
	{
		shutdown_waitTickTime_ = float(xml->getValFloat(rootNode));
	}

	rootNode = xml->getRootNode("callback_timeout");
	if(rootNode != NULL)
	{
		callback_timeout_ = float(xml->getValFloat(rootNode));
		if(callback_timeout_ < 5.f)
			callback_timeout_ = 5.f;
	}
	
	rootNode = xml->getRootNode("thread_pool");
	if(rootNode != NULL)
	{
		TiXmlNode* childnode = xml->enterNode(rootNode, "timeout");
		if(childnode)
		{
			thread_timeout_ = float(KBE_MAX(1.0, xml->getValFloat(childnode)));
		}

		childnode = xml->enterNode(rootNode, "init_create");
		if(childnode)
		{
			thread_init_create_ = KBE_MAX(1, xml->getValInt(childnode));
		}

		childnode = xml->enterNode(rootNode, "pre_create");
		if(childnode)
		{
			thread_pre_create_ = KBE_MAX(1, xml->getValInt(childnode));
		}

		childnode = xml->enterNode(rootNode, "max_create");
		if(childnode)
		{
			thread_max_create_ = KBE_MAX(1, xml->getValInt(childnode));
		}
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

		childnode = xml->enterNode(rootNode, "resend");
		if(childnode)
		{
			TiXmlNode* childnode1 = xml->enterNode(childnode, "internal");
			if(childnode1)
			{
				TiXmlNode* childnode2 = xml->enterNode(childnode1, "interval");
				if(childnode2)
				{
					Mercury::g_intReSendInterval = uint32(xml->getValInt(childnode2));
				}

				childnode2 = xml->enterNode(childnode1, "retries");
				if(childnode2)
				{
					Mercury::g_intReSendRetries = uint32(xml->getValInt(childnode2));
				}
			}

			childnode1 = xml->enterNode(childnode, "external");
			if(childnode)
			{
				TiXmlNode* childnode2 = xml->enterNode(childnode1, "interval");
				if(childnode2)
				{
					Mercury::g_extReSendInterval = uint32(xml->getValInt(childnode2));
				}

				childnode2 = xml->enterNode(childnode1, "retries");
				if(childnode2)
				{
					Mercury::g_extReSendRetries = uint32(xml->getValInt(childnode2));
				}
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
				TiXmlNode* childnode2 = xml->enterNode(childnode1, "internal");
				if(childnode2)
					Mercury::g_intReceiveWindowMessagesOverflow = KBE_MAX(0, xml->getValInt(childnode2));

				childnode2 = xml->enterNode(childnode1, "external");
				if(childnode2)
					Mercury::g_extReceiveWindowMessagesOverflow = KBE_MAX(0, xml->getValInt(childnode2));

				childnode2 = xml->enterNode(childnode1, "critical");
				if(childnode2)
					Mercury::g_receiveWindowMessagesOverflowCritical = KBE_MAX(0, xml->getValInt(childnode2));
			}

			childnode1 = xml->enterNode(childnode, "bytes");
			if(childnode1)
			{
				TiXmlNode* childnode2 = xml->enterNode(childnode1, "internal");
				if(childnode2)
					Mercury::g_intReceiveWindowBytesOverflow = KBE_MAX(0, xml->getValInt(childnode2));
				
				childnode2 = xml->enterNode(childnode1, "external");
				if(childnode2)
					Mercury::g_extReceiveWindowBytesOverflow = KBE_MAX(0, xml->getValInt(childnode2));
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
		TiXmlNode* childnode = xml->enterNode(rootNode, "accountType");
		if(childnode)
		{
			billingSystem_accountType_ = xml->getValStr(childnode);
			if(billingSystem_accountType_.size() == 0)
				billingSystem_accountType_ = "normal";
		}

		childnode = xml->enterNode(rootNode, "chargeType");
		if(childnode)
		{
			billingSystem_chargeType_ = xml->getValStr(childnode);
			if(billingSystem_chargeType_.size() == 0)
				billingSystem_chargeType_ = "normal";
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

		node = xml->enterNode(rootNode, "aliasEntityID");
		if(node != NULL){
			_cellAppInfo.aliasEntityID = (xml->getValStr(node) == "true");
		}

		node = xml->enterNode(rootNode, "entitydefAliasID");
		if(node != NULL){
			_cellAppInfo.entitydefAliasID = (xml->getValStr(node) == "true");
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
				_cellAppInfo.coordinateSystem_hasY = (xml->getValStr(childnode) == "true");
			}

			childnode = xml->enterNode(node, "entity_posdir_additional_updates");
			if(childnode)
			{
				_cellAppInfo.entity_posdir_additional_updates = xml->getValInt(childnode);
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

		node = xml->enterNode(rootNode, "shutdown");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "perSecsDestroyEntitySize");
			if(childnode)
			{
				_cellAppInfo.perSecsDestroyEntitySize = uint32(xml->getValInt(childnode));
			}
		}

		node = xml->enterNode(rootNode, "witness");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "timeout");
			if(childnode)
			{
				_cellAppInfo.witness_timeout = uint16(xml->getValInt(childnode));
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

		node = xml->enterNode(rootNode, "externalAddress");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.externalAddress, xml->getValStr(node).c_str(), MAX_NAME);

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

		node = xml->enterNode(rootNode, "shutdown");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "perSecsDestroyEntitySize");
			if(childnode)
			{
				_baseAppInfo.perSecsDestroyEntitySize = uint32(xml->getValInt(childnode));
			}
		}

		node = xml->enterNode(rootNode, "respool");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "buffer_size");
			if(childnode)
				_baseAppInfo.respool_buffersize = xml->getValInt(childnode);

			childnode = xml->enterNode(node, "timeout");
			if(childnode)
				_baseAppInfo.respool_timeout = uint64(xml->getValInt(childnode));

			childnode = xml->enterNode(node, "checktick");
			if(childnode)
				Resmgr::respool_checktick = xml->getValInt(childnode);

			Resmgr::respool_timeout = _baseAppInfo.respool_timeout;
			Resmgr::respool_buffersize = _baseAppInfo.respool_buffersize;
		}
	}

	rootNode = xml->getRootNode("dbmgr");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

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
		
		node = xml->enterNode(rootNode, "debug");
		if(node != NULL){
			_dbmgrInfo.debugDBMgr = (xml->getValStr(node) == "true");
		}

		node = xml->enterNode(rootNode, "allowEmptyDigest");
		if(node != NULL){
			_dbmgrInfo.allowEmptyDigest = (xml->getValStr(node) == "true");
		}

		node = xml->enterNode(rootNode, "account_system");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "accountDefaultFlags");
			if(childnode)
			{
				_dbmgrInfo.accountDefaultFlags = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "accountDefaultDeadline");	
			if(childnode != NULL)
			{
				_dbmgrInfo.accountDefaultDeadline = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "accountEntityScriptType");	
			if(childnode != NULL)
			{
				strncpy((char*)&_dbmgrInfo.dbAccountEntityScriptType, xml->getValStr(childnode).c_str(), MAX_NAME);
			}

			childnode = xml->enterNode(node, "account_registration");	
			if(childnode != NULL)
			{
				TiXmlNode* childchildnode = xml->enterNode(childnode, "enable");
				if(childchildnode)
				{
					_dbmgrInfo.account_registration_enable = (xml->getValStr(childchildnode) == "true");
				}

				childchildnode = xml->enterNode(childnode, "loginAutoCreate");
				if(childchildnode != NULL){
					_dbmgrInfo.notFoundAccountAutoCreate = (xml->getValStr(childchildnode) == "true");
				}
			} 
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

		node = xml->enterNode(rootNode, "externalAddress");	
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.externalAddress, xml->getValStr(node).c_str(), MAX_NAME);

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

		node = xml->enterNode(rootNode, "account_type");
		if(node != NULL){
			_loginAppInfo.account_type = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "http_cbhost");
		if(node)
			_loginAppInfo.http_cbhost = xml->getValStr(node);

		node = xml->enterNode(rootNode, "http_cbport");
		if(node)
			_loginAppInfo.http_cbport = xml->getValInt(node);
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

		node = xml->enterNode(rootNode, "account_infos");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "account_name_prefix");
			if(childnode)
			{
				_botsInfo.bots_account_name_prefix = xml->getValStr(childnode);
			}

			childnode = xml->enterNode(node, "account_name_suffix_inc");
			if(childnode)
			{
				_botsInfo.bots_account_name_suffix_inc = xml->getValInt(childnode);
			}
		}

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_botsInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "telnet_service");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "port");
			if(childnode)
			{
				_botsInfo.telnet_port = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "password");
			if(childnode)
			{
				_botsInfo.telnet_passwd = xml->getValStr(childnode);
			}

			childnode = xml->enterNode(node, "default_layer");
			if(childnode)
			{
				_botsInfo.telnet_deflayer = xml->getValStr(childnode);
			}
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

		node = xml->enterNode(rootNode, "tick_max_buffered_logs");
		if(node != NULL){
			tick_max_buffered_logs_ = (uint32)xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "tick_max_sync_logs");
		if(node != NULL){
			tick_max_sync_logs_ = (uint32)xml->getValInt(node);
		}
	}

	SAFE_RELEASE(xml);

	if(email_service_config.size() > 0)
	{
		xml = new XmlPlus(Resmgr::getSingleton().matchRes(email_service_config).c_str());

		if(!xml->isGood())
		{
			ERROR_MSG(boost::format("ServerConfig::loadConfig: load %1% is failed!\n") %
				email_service_config.c_str());

			SAFE_RELEASE(xml);
			return false;
		}

		TiXmlNode* childnode = xml->getRootNode("smtp_server");
		if(childnode)
			emailServerInfo_.smtp_server = xml->getValStr(childnode);

		childnode = xml->getRootNode("smtp_port");
		if(childnode)
			emailServerInfo_.smtp_port = xml->getValInt(childnode);

		childnode = xml->getRootNode("username");
		if(childnode)
			emailServerInfo_.username = xml->getValStr(childnode);

		childnode = xml->getRootNode("password");
		if(childnode)
		{
			emailServerInfo_.password = xml->getValStr(childnode);
		}

		childnode = xml->getRootNode("smtp_auth");
		if(childnode)
			emailServerInfo_.smtp_auth = xml->getValInt(childnode);

		TiXmlNode* rootNode1 = xml->getRootNode("email_activation");
		if(rootNode1 != NULL)
		{
			TiXmlNode* childnode1 = xml->enterNode(rootNode1, "subject");
			if(childnode1)
				emailAtivationInfo_.subject = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "message");
			if(childnode1)
				emailAtivationInfo_.message = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "deadline");
			if(childnode1)
				emailAtivationInfo_.deadline = xml->getValInt(childnode1);

			childnode1 = xml->enterNode(rootNode1, "backlink_success_message");
			if(childnode1)
				emailAtivationInfo_.backlink_success_message = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "backlink_fail_message");
			if(childnode1)
				emailAtivationInfo_.backlink_fail_message = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "backlink_hello_message");
			if(childnode1)
				emailAtivationInfo_.backlink_hello_message = childnode1->ToText()->Value();
		}

		rootNode1 = xml->getRootNode("email_resetpassword");
		if(rootNode1 != NULL)
		{
			TiXmlNode* childnode1 = xml->enterNode(rootNode1, "subject");
			if(childnode1)
				emailResetPasswordInfo_.subject = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "message");
			if(childnode1)
				emailResetPasswordInfo_.message = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "deadline");
			if(childnode1)
				emailResetPasswordInfo_.deadline = xml->getValInt(childnode1);

			childnode1 = xml->enterNode(rootNode1, "backlink_success_message");
			if(childnode1)
				emailResetPasswordInfo_.backlink_success_message = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "backlink_fail_message");
			if(childnode1)
				emailResetPasswordInfo_.backlink_fail_message = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "backlink_hello_message");
			if(childnode1)
				emailResetPasswordInfo_.backlink_hello_message = childnode1->ToText()->Value();
		}

		rootNode1 = xml->getRootNode("email_bind");
		if(rootNode1 != NULL)
		{
			TiXmlNode* childnode1 = xml->enterNode(rootNode1, "subject");
			if(childnode1)
				emailBindInfo_.subject = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "message");
			if(childnode1)
				emailBindInfo_.message = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "deadline");
			if(childnode1)
				emailBindInfo_.deadline = xml->getValInt(childnode1);

			childnode1 = xml->enterNode(rootNode1, "backlink_success_message");
			if(childnode1)
				emailBindInfo_.backlink_success_message = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "backlink_fail_message");
			if(childnode1)
				emailBindInfo_.backlink_fail_message = childnode1->ToText()->Value();

			childnode1 = xml->enterNode(rootNode1, "backlink_hello_message");
			if(childnode1)
				emailBindInfo_.backlink_hello_message = childnode1->ToText()->Value();
		}
	}

	SAFE_RELEASE(xml);
	return true;
}

//-------------------------------------------------------------------------------------	
uint32 ServerConfig::tcp_SOMAXCONN(COMPONENT_TYPE componentType)
{
	ENGINE_COMPONENT_INFO& cinfo = getComponent(componentType);
	return cinfo.tcp_SOMAXCONN;
}

//-------------------------------------------------------------------------------------	
void ServerConfig::_updateEmailInfos()
{
#ifdef USE_OPENSSL
	// 如果小于64则表示目前还是明文密码
	if(emailServerInfo_.password.size() < 64)
	{
		WARNING_MSG(boost::format("ServerConfig::loadConfig: email password(email_service.xml) is not encrypted!\nplease use password(rsa):\n%1%\n") 
			% KBEKey::getSingleton().encrypt(emailServerInfo_.password));
	}
	else
	{
		std::string out = KBEKey::getSingleton().decrypt(emailServerInfo_.password);
		if(out.size() == 0)
		{
			ERROR_MSG("ServerConfig::loadConfig: email password(email_service.xml) encrypt is error!\n");
		}
		else
		{
			emailServerInfo_.password = out;
		}
	}
#endif
}

//-------------------------------------------------------------------------------------	
void ServerConfig::updateInfos(bool isPrint, COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
							   const Mercury::Address& internalAddr, const Mercury::Address& externalAddr)
{
	std::string infostr = "";

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

			infostr += "server-configs:\n";
			infostr += (boost::format("\tgameUpdateHertz : %1%\n") % gameUpdateHertz()).str();
			infostr += (boost::format("\tdefaultAoIRadius : %1%\n") % info.defaultAoIRadius).str();
			infostr += (boost::format("\tdefaultAoIHysteresisArea : %1%\n") % info.defaultAoIHysteresisArea).str();
			infostr += (boost::format("\tentryScriptFile : %1%\n") % info.entryScriptFile).str();
			infostr += (boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str()).str();
			//infostr += (boost::format("\texternalAddr : %1%\n") % externalAddr.c_str()).str();
			infostr += (boost::format("\tcomponentID : %1%\n") % info.componentID).str();
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
			INFO_MSG(boost::format("\tentryScriptFile : %1%\n") % info.entryScriptFile);
			INFO_MSG(boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str());
			INFO_MSG(boost::format("\texternalAddr : %1%\n") % externalAddr.c_str());
			INFO_MSG(boost::format("\tcomponentID : %1%\n") % info.componentID);

			infostr += "server-configs:\n";
			infostr += (boost::format("\tgameUpdateHertz : %1%\n") % gameUpdateHertz()).str();
			infostr += (boost::format("\tentryScriptFile : %1%\n") % info.entryScriptFile).str();
			infostr += (boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str()).str();
			infostr += (boost::format("\texternalAddr : %1%\n") % externalAddr.c_str()).str();
			infostr += (boost::format("\tcomponentID : %1%\n") % info.componentID).str();
		}

		_updateEmailInfos();
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

			infostr += "server-configs:\n";
			infostr += (boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str()).str();
			infostr += (boost::format("\tcomponentID : %1%\n") % info.componentID).str();
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

			infostr += "server-configs:\n";
			infostr += (boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str()).str();
			infostr += (boost::format("\tcomponentID : %1%\n") % info.componentID).str();
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

			infostr += "server-configs:\n";
			infostr += (boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str()).str();
			infostr += (boost::format("\tcomponentID : %1%\n") % info.componentID).str();
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

			infostr += "server-configs:\n";
			infostr += (boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str()).str();
			infostr += (boost::format("\texternalAddr : %1%\n") % externalAddr.c_str()).str();
			infostr += (boost::format("\tcomponentID : %1%\n") % info.componentID).str();
		}

		_updateEmailInfos();
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

			infostr += "server-configs:\n";
			infostr += (boost::format("\tinternalAddr : %1%\n") % internalAddr.c_str()).str();
			infostr += (boost::format("\tcomponentID : %1%\n") % info.componentID).str();
		}
	}

#if KBE_PLATFORM == PLATFORM_WIN32
	if(infostr.size() > 0)
	{
		infostr += "\n";
		printf("%s", infostr.c_str());
	}
#endif
}

//-------------------------------------------------------------------------------------		
}
