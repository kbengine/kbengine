/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "serverconfig.h"
#include "network/common.h"
#include "network/address.h"
#include "resmgr/resmgr.h"
#include "common/kbekey.h"
#include "common/kbeversion.h"

#ifndef CODE_INLINE
#include "serverconfig.inl"
#endif

namespace KBEngine{
KBE_SINGLETON_INIT(ServerConfig);

//-------------------------------------------------------------------------------------
ServerConfig::ServerConfig():
gameUpdateHertz_(10),
tick_max_buffered_logs_(4096),
tick_max_sync_logs_(32),
interfacesAddr_(),
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
	SmartPointer<XML> xml(new XML(Resmgr::getSingleton().matchRes(fileName).c_str()));

	if(!xml->isGood())
	{
		ERROR_MSG(fmt::format("ServerConfig::loadConfig: load {} is failed!\n",
			fileName.c_str()));

		return false;
	}
	
	if(xml->getRootNode() == NULL)
	{
		// root�ڵ���û���ӽڵ���
		return true;
	}

	std::string email_service_config;
	rootNode = xml->getRootNode("email_service_config");
	if(rootNode != NULL)
	{
		email_service_config = xml->getValStr(rootNode);
	}

	rootNode = xml->getRootNode("packetAlwaysContainLength");
	if(rootNode != NULL){
		Network::g_packetAlwaysContainLength = xml->getValInt(rootNode) != 0;
	}

	rootNode = xml->getRootNode("trace_packet");
	if(rootNode != NULL)
	{
		TiXmlNode* childnode = xml->enterNode(rootNode, "debug_type");
		if(childnode)
			Network::g_trace_packet = xml->getValInt(childnode);

		if(Network::g_trace_packet > 3)
			Network::g_trace_packet = 0;

		childnode = xml->enterNode(rootNode, "use_logfile");
		if(childnode)
			Network::g_trace_packet_use_logfile = (xml->getValStr(childnode) == "true");

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
						Network::g_trace_packet_disables.push_back(c);
						
						// ��debug���ܰ�
						if(c == "Encrypted::packets")
							Network::g_trace_encrypted_packet = false;
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
				channelCommon_.channelInternalTimeout = KBE_MAX(0.f, float(xml->getValFloat(childnode1)));
				Network::g_channelInternalTimeout = channelCommon_.channelInternalTimeout;
			}

			childnode1 = xml->enterNode(childnode, "external");
			if(childnode)
			{
				channelCommon_.channelExternalTimeout = KBE_MAX(0.f, float(xml->getValFloat(childnode1)));
				Network::g_channelExternalTimeout = channelCommon_.channelExternalTimeout;
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
					Network::g_intReSendInterval = uint32(xml->getValInt(childnode2));
				}

				childnode2 = xml->enterNode(childnode1, "retries");
				if(childnode2)
				{
					Network::g_intReSendRetries = uint32(xml->getValInt(childnode2));
				}
			}

			childnode1 = xml->enterNode(childnode, "external");
			if(childnode)
			{
				TiXmlNode* childnode2 = xml->enterNode(childnode1, "interval");
				if(childnode2)
				{
					Network::g_extReSendInterval = uint32(xml->getValInt(childnode2));
				}

				childnode2 = xml->enterNode(childnode1, "retries");
				if(childnode2)
				{
					Network::g_extReSendRetries = uint32(xml->getValInt(childnode2));
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

		childnode = xml->enterNode(rootNode, "windowOverflow");
		if(childnode)
		{
			TiXmlNode* sendNode = xml->enterNode(childnode, "send");
			if(sendNode)
			{
				TiXmlNode* childnode1 = xml->enterNode(sendNode, "messages");
				if(childnode1)
				{
					TiXmlNode* childnode2 = xml->enterNode(childnode1, "internal");
					if(childnode2)
						Network::g_intSendWindowMessagesOverflow = KBE_MAX(0, xml->getValInt(childnode2));

					childnode2 = xml->enterNode(childnode1, "external");
					if(childnode2)
						Network::g_extSendWindowMessagesOverflow = KBE_MAX(0, xml->getValInt(childnode2));

					childnode2 = xml->enterNode(childnode1, "critical");
					if(childnode2)
						Network::g_sendWindowMessagesOverflowCritical = KBE_MAX(0, xml->getValInt(childnode2));
				}

				childnode1 = xml->enterNode(sendNode, "bytes");
				if(childnode1)
				{
					TiXmlNode* childnode2 = xml->enterNode(childnode1, "internal");
					if(childnode2)
						Network::g_intSendWindowBytesOverflow = KBE_MAX(0, xml->getValInt(childnode2));
				
					childnode2 = xml->enterNode(childnode1, "external");
					if(childnode2)
						Network::g_extSendWindowBytesOverflow = KBE_MAX(0, xml->getValInt(childnode2));
				}
			}

			TiXmlNode* recvNode = xml->enterNode(childnode, "receive");
			if(recvNode)
			{
				TiXmlNode* childnode1 = xml->enterNode(recvNode, "messages");
				if(childnode1)
				{
					TiXmlNode* childnode2 = xml->enterNode(childnode1, "internal");
					if(childnode2)
						Network::g_intReceiveWindowMessagesOverflow = KBE_MAX(0, xml->getValInt(childnode2));

					childnode2 = xml->enterNode(childnode1, "external");
					if(childnode2)
						Network::g_extReceiveWindowMessagesOverflow = KBE_MAX(0, xml->getValInt(childnode2));

					childnode2 = xml->enterNode(childnode1, "critical");
					if(childnode2)
						Network::g_receiveWindowMessagesOverflowCritical = KBE_MAX(0, xml->getValInt(childnode2));
				}

				childnode1 = xml->enterNode(recvNode, "bytes");
				if(childnode1)
				{
					TiXmlNode* childnode2 = xml->enterNode(childnode1, "internal");
					if(childnode2)
						Network::g_intReceiveWindowBytesOverflow = KBE_MAX(0, xml->getValInt(childnode2));
				
					childnode2 = xml->enterNode(childnode1, "external");
					if(childnode2)
						Network::g_extReceiveWindowBytesOverflow = KBE_MAX(0, xml->getValInt(childnode2));
				}
			}
		};

		childnode = xml->enterNode(rootNode, "encrypt_type");
		if(childnode)
		{
			Network::g_channelExternalEncryptType = xml->getValInt(childnode);
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

	rootNode = xml->getRootNode("interfaces");
	if(rootNode != NULL)
	{
		TiXmlNode* childnode = xml->enterNode(rootNode, "entryScriptFile");	
		if(childnode != NULL)
			strncpy((char*)&_interfacesInfo.entryScriptFile, xml->getValStr(childnode).c_str(), MAX_NAME);

		childnode = xml->enterNode(rootNode, "host");
		if(childnode)
		{
			std::string ip = xml->getValStr(childnode);
			Network::Address addr(ip, ntohs(interfacesAddr_.port));
			interfacesAddr_ = addr;
		}

		uint16 port = 0;
		childnode = xml->enterNode(rootNode, "port");
		if(childnode)
		{
			port = xml->getValInt(childnode);

			if(port <= 0)
				port = KBE_INTERFACES_TCP_PORT;

			Network::Address addr(inet_ntoa((struct in_addr&)interfacesAddr_.ip), port);
			interfacesAddr_ = addr;
		}

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_interfacesInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "orders_timeout");
		if(node != NULL){
			interfaces_orders_timeout_ = xml->getValInt(node);
		}
	
		node = xml->enterNode(rootNode, "telnet_service");
		if (node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "port");
			if (childnode)
			{
				_interfacesInfo.telnet_port = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "password");
			if (childnode)
			{
				_interfacesInfo.telnet_passwd = xml->getValStr(childnode);
			}

			childnode = xml->enterNode(node, "default_layer");
			if (childnode)
			{
				_interfacesInfo.telnet_deflayer = xml->getValStr(childnode);
			}
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

			childnode = xml->enterNode(node, "networkprofile");
			if(childnode)
			{
				_cellAppInfo.profiles.open_networkprofile = (xml->getValStr(childnode) == "true");
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

		node = xml->enterNode(rootNode, "loadSmoothingBias");
		if(node != NULL)
			_cellAppInfo.loadSmoothingBias = float(xml->getValFloat(node));

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

			childnode = xml->enterNode(node, "networkprofile");
			if(childnode)
			{
				_baseAppInfo.profiles.open_networkprofile = (xml->getValStr(childnode) == "true");
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
		node = xml->enterNode(rootNode, "entryScriptFile");
		if (node != NULL)
			strncpy((char*)&_dbmgrInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME);
		
		node = xml->enterNode(rootNode, "telnet_service");
		if (node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "port");
			if (childnode)
			{
				_dbmgrInfo.telnet_port = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "password");
			if (childnode)
			{
				_dbmgrInfo.telnet_passwd = xml->getValStr(childnode);
			}

			childnode = xml->enterNode(node, "default_layer");
			if (childnode)
			{
				_dbmgrInfo.telnet_deflayer = xml->getValStr(childnode);
			}
		}

		node = xml->enterNode(rootNode, "InterfacesServiceAddr");
		if (node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "host");
			if (childnode)
			{
				std::string ip = xml->getValStr(childnode);
				Network::Address addr(ip, ntohs(interfacesAddr_.port));
				interfacesAddr_ = addr;
			}

			uint16 port = 0;
			childnode = xml->enterNode(node, "port");
			if (childnode)
			{
				port = xml->getValInt(childnode);

				if (port <= 0)
					port = KBE_INTERFACES_TCP_PORT;

				Network::Address addr(inet_ntoa((struct in_addr&)interfacesAddr_.ip), port);
				interfacesAddr_ = addr;
			}

			childnode = xml->enterNode(node, "enable");
			if (childnode)
			{
				if(xml->getValStr(childnode) != "true")
					interfacesAddr_ = Network::Address::NONE;
			}
		}

		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		TiXmlNode* databaseInterfacesNode = xml->enterNode(rootNode, "databaseInterfaces");	
		if(databaseInterfacesNode != NULL)
		{
			if (databaseInterfacesNode->FirstChild() != NULL)
			{
				do
				{
					std::string name = databaseInterfacesNode->Value();

					DBInterfaceInfo dbinfo;
					DBInterfaceInfo* pDBInfo = dbInterface(name);
					if (!pDBInfo)
						pDBInfo = &dbinfo;
					
					strncpy((char*)&pDBInfo->name, name.c_str(), MAX_NAME);

					TiXmlNode* interfaceNode = databaseInterfacesNode->FirstChild();
					
					node = xml->enterNode(interfaceNode, "pure");
					if (node)
						pDBInfo->isPure = xml->getValStr(node) == "true";

					// Ĭ�Ͽⲻ�����Ǵ����⣬������Ҫ����ʵ���
					if (name == "default")
						pDBInfo->isPure = false;

					node = xml->enterNode(interfaceNode, "type");
					if(node != NULL)
						strncpy((char*)&pDBInfo->db_type, xml->getValStr(node).c_str(), MAX_NAME);
					
					node = xml->enterNode(interfaceNode, "host");
					if(node != NULL)
						strncpy((char*)&pDBInfo->db_ip, xml->getValStr(node).c_str(), MAX_IP);

					node = xml->enterNode(interfaceNode, "port");
					if(node != NULL)
						pDBInfo->db_port = xml->getValInt(node);

					node = xml->enterNode(interfaceNode, "auth");
					if(node != NULL)
					{
						TiXmlNode* childnode = xml->enterNode(node, "password");
						if(childnode)
						{
							strncpy((char*)&pDBInfo->db_password, xml->getValStr(childnode).c_str(), MAX_BUF * 10);
						}

						childnode = xml->enterNode(node, "username");
						if(childnode)
						{
							strncpy((char*)&pDBInfo->db_username, xml->getValStr(childnode).c_str(), MAX_NAME);
						}

						childnode = xml->enterNode(node, "encrypt");
						if(childnode)
						{
							pDBInfo->db_passwordEncrypt = xml->getValStr(childnode) == "true";
						}
					}
						
					node = xml->enterNode(interfaceNode, "databaseName");
					if(node != NULL)
						strncpy((char*)&pDBInfo->db_name, xml->getValStr(node).c_str(), MAX_NAME);

					node = xml->enterNode(interfaceNode, "numConnections");
					if(node != NULL)
						pDBInfo->db_numConnections = xml->getValInt(node);
						
					node = xml->enterNode(interfaceNode, "unicodeString");
					if(node != NULL)
					{
						TiXmlNode* childnode = xml->enterNode(node, "characterSet");
						if(childnode)
						{
							pDBInfo->db_unicodeString_characterSet = xml->getValStr(childnode);
						}

						childnode = xml->enterNode(node, "collation");
						if(childnode)
						{
							pDBInfo->db_unicodeString_collation = xml->getValStr(childnode);
						}
					}

					if (pDBInfo->db_unicodeString_characterSet.size() == 0)
						pDBInfo->db_unicodeString_characterSet = "utf8";

					if (pDBInfo->db_unicodeString_collation.size() == 0)
						pDBInfo->db_unicodeString_collation = "utf8_bin";
	
					if (pDBInfo == &dbinfo)
					{
						// ��鲻���ڲ�ͬ�Ľӿ���ʹ����ͬ�����ݿ�����ͬ�ı�
						std::vector<DBInterfaceInfo>::iterator dbinfo_iter = _dbmgrInfo.dbInterfaceInfos.begin();
						for (; dbinfo_iter != _dbmgrInfo.dbInterfaceInfos.end(); ++dbinfo_iter)
						{
							if (kbe_stricmp((*dbinfo_iter).db_ip, dbinfo.db_ip) == 0 && 
								kbe_stricmp((*dbinfo_iter).db_type, dbinfo.db_type) == 0 &&
								(*dbinfo_iter).db_port == dbinfo.db_port &&
								strcmp(dbinfo.db_name, (*dbinfo_iter).db_name) == 0)
							{
								ERROR_MSG(fmt::format("ServerConfig::loadConfig: databaseInterfaces, Conflict between \"{}\" and \"{}\", file={}!\n",
									(*dbinfo_iter).name, dbinfo.name, fileName.c_str()));

								return false;
							}
						}

						_dbmgrInfo.dbInterfaceInfos.push_back(dbinfo);
					}

				} while ((databaseInterfacesNode = databaseInterfacesNode->NextSibling()));
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

	rootNode = xml->getRootNode("loginapp");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "entryScriptFile");
		if (node != NULL)
			strncpy((char*)&_loginAppInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME);
		
		node = xml->enterNode(rootNode, "telnet_service");
		if (node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "port");
			if (childnode)
			{
				_loginAppInfo.telnet_port = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "password");
			if (childnode)
			{
				_loginAppInfo.telnet_passwd = xml->getValStr(childnode);
			}

			childnode = xml->enterNode(node, "default_layer");
			if (childnode)
			{
				_loginAppInfo.telnet_deflayer = xml->getValStr(childnode);
			}
		}

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
	
	rootNode = xml->getRootNode("machine");
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
		
		node = xml->enterNode(rootNode, "addresses");
		if(node)
		{
			do
			{
				if(node->FirstChild() != NULL)
				{
					std::string c = node->FirstChild()->Value();
					c = strutil::kbe_trim(c);
					if(c.size() > 0)
					{
						_kbMachineInfo.machine_addresses.push_back(c);
					}
				}
			} while((node = node->NextSibling()));
		}
	}
	
	rootNode = xml->getRootNode("bots");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "host");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.login_ip, xml->getValStr(node).c_str(), MAX_IP);

		node = xml->enterNode(rootNode, "port");	
		if(node != NULL)
			_botsInfo.login_port = xml->getValInt(node);

		node = xml->enterNode(rootNode, "isOnInitCallPropertysSetMethods");
		if (node != NULL)
			_botsInfo.isOnInitCallPropertysSetMethods = xml->getValInt(node);

		node = xml->enterNode(rootNode, "defaultAddBots");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "totalCount");
			if(childnode)
			{
				_botsInfo.defaultAddBots_totalCount = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "tickCount");
			if(childnode)
			{
				_botsInfo.defaultAddBots_tickCount = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "tickTime");
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

	rootNode = xml->getRootNode("logger");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_loggerInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "entryScriptFile");
		if (node != NULL)
			strncpy((char*)&_loggerInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME);

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_loggerInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "tick_max_buffered_logs");
		if(node != NULL){
			tick_max_buffered_logs_ = (uint32)xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "tick_sync_logs");
		if(node != NULL){
			tick_max_sync_logs_ = (uint32)xml->getValInt(node);
		}
	
		node = xml->enterNode(rootNode, "telnet_service");
		if (node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "port");
			if (childnode)
			{
				_loggerInfo.telnet_port = xml->getValInt(childnode);
			}

			childnode = xml->enterNode(node, "password");
			if (childnode)
			{
				_loggerInfo.telnet_passwd = xml->getValStr(childnode);
			}

			childnode = xml->enterNode(node, "default_layer");
			if (childnode)
			{
				_loggerInfo.telnet_deflayer = xml->getValStr(childnode);
			}
		}
	}

	if(email_service_config.size() > 0)
	{
		SmartPointer<XML> emailxml(new XML(Resmgr::getSingleton().matchRes(email_service_config).c_str()));

		if(!emailxml->isGood())
		{
			ERROR_MSG(fmt::format("ServerConfig::loadConfig: load {} is failed!\n",
				email_service_config.c_str()));

			return false;
		}

		TiXmlNode* childnode = emailxml->getRootNode("smtp_server");
		if(childnode)
			emailServerInfo_.smtp_server = emailxml->getValStr(childnode);

		childnode = emailxml->getRootNode("smtp_port");
		if(childnode)
			emailServerInfo_.smtp_port = emailxml->getValInt(childnode);

		childnode = emailxml->getRootNode("username");
		if(childnode)
			emailServerInfo_.username = emailxml->getValStr(childnode);

		childnode = emailxml->getRootNode("password");
		if(childnode)
		{
			emailServerInfo_.password = emailxml->getValStr(childnode);
		}

		childnode = emailxml->getRootNode("smtp_auth");
		if(childnode)
			emailServerInfo_.smtp_auth = emailxml->getValInt(childnode);

		TiXmlNode* rootNode1 = emailxml->getRootNode("email_activation");
		if(rootNode1 != NULL)
		{
			TiXmlNode* childnode1 = emailxml->enterNode(rootNode1, "subject");
			if(childnode1)
				emailAtivationInfo_.subject = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "message");
			if(childnode1)
				emailAtivationInfo_.message = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "deadline");
			if(childnode1)
				emailAtivationInfo_.deadline = emailxml->getValInt(childnode1);

			childnode1 = emailxml->enterNode(rootNode1, "backlink_success_message");
			if(childnode1)
				emailAtivationInfo_.backlink_success_message = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "backlink_fail_message");
			if(childnode1)
				emailAtivationInfo_.backlink_fail_message = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "backlink_hello_message");
			if(childnode1)
				emailAtivationInfo_.backlink_hello_message = childnode1->ToText()->Value();
		}

		rootNode1 = emailxml->getRootNode("email_resetpassword");
		if(rootNode1 != NULL)
		{
			TiXmlNode* childnode1 = emailxml->enterNode(rootNode1, "subject");
			if(childnode1)
				emailResetPasswordInfo_.subject = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "message");
			if(childnode1)
				emailResetPasswordInfo_.message = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "deadline");
			if(childnode1)
				emailResetPasswordInfo_.deadline = emailxml->getValInt(childnode1);

			childnode1 = emailxml->enterNode(rootNode1, "backlink_success_message");
			if(childnode1)
				emailResetPasswordInfo_.backlink_success_message = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "backlink_fail_message");
			if(childnode1)
				emailResetPasswordInfo_.backlink_fail_message = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "backlink_hello_message");
			if(childnode1)
				emailResetPasswordInfo_.backlink_hello_message = childnode1->ToText()->Value();
		}

		rootNode1 = emailxml->getRootNode("email_bind");
		if(rootNode1 != NULL)
		{
			TiXmlNode* childnode1 = emailxml->enterNode(rootNode1, "subject");
			if(childnode1)
				emailBindInfo_.subject = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "message");
			if(childnode1)
				emailBindInfo_.message = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "deadline");
			if(childnode1)
				emailBindInfo_.deadline = emailxml->getValInt(childnode1);

			childnode1 = emailxml->enterNode(rootNode1, "backlink_success_message");
			if(childnode1)
				emailBindInfo_.backlink_success_message = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "backlink_fail_message");
			if(childnode1)
				emailBindInfo_.backlink_fail_message = childnode1->ToText()->Value();

			childnode1 = emailxml->enterNode(rootNode1, "backlink_hello_message");
			if(childnode1)
				emailBindInfo_.backlink_hello_message = childnode1->ToText()->Value();
		}
	}

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
	// ���С��64���ʾĿǰ������������
	if(emailServerInfo_.password.size() < 64)
	{
		WARNING_MSG(fmt::format("ServerConfig::loadConfig: email password(email_service.xml) is not encrypted!\nplease use password(rsa):\n{}\n"
			, KBEKey::getSingleton().encrypt(emailServerInfo_.password)));
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
}

//-------------------------------------------------------------------------------------	
void ServerConfig::updateExternalAddress(char* buf)
{
	if(strlen(buf) > 0)
	{
		unsigned int inaddr = 0; 
		if((inaddr = inet_addr(buf)) == INADDR_NONE)  
		{
			struct hostent *host;
			host = gethostbyname(buf);
			if(host)
			{
				strncpy(buf, inet_ntoa(*(struct in_addr*)host->h_addr_list[0]), MAX_BUF);
			}	
		}
	}
}

//-------------------------------------------------------------------------------------	
void ServerConfig::updateInfos(bool isPrint, COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
							   const Network::Address& internalAddr, const Network::Address& externalAddr)
{
	std::string infostr = "";

	for (size_t i = 0; i < _dbmgrInfo.dbInterfaceInfos.size(); ++i)
		_dbmgrInfo.dbInterfaceInfos[i].index = i;

	//updateExternalAddress(getBaseApp().externalAddress);
	//updateExternalAddress(getLoginApp().externalAddress);

	if(componentType == CELLAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getCellApp();
		info.internalAddr = &internalAddr;
		info.externalAddr = &externalAddr;
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			INFO_MSG(fmt::format("\tdefaultAoIRadius : {}\n", info.defaultAoIRadius));
			INFO_MSG(fmt::format("\tdefaultAoIHysteresisArea : {}\n", info.defaultAoIHysteresisArea));
			INFO_MSG(fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			INFO_MSG(fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			//INFO_MSG(fmt::format("\texternalAddr : {}\n", externalAddr.c_str()));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			infostr += (fmt::format("\tdefaultAoIRadius : {}\n", info.defaultAoIRadius));
			infostr += (fmt::format("\tdefaultAoIHysteresisArea : {}\n", info.defaultAoIHysteresisArea));
			infostr += (fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			infostr += (fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			//infostr += (fmt::format("\texternalAddr : {}\n", externalAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == BASEAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getBaseApp();
		info.internalAddr = const_cast<Network::Address*>(&internalAddr);
		info.externalAddr = const_cast<Network::Address*>(&externalAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			INFO_MSG(fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			INFO_MSG(fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			INFO_MSG(fmt::format("\texternalAddr : {}\n", externalAddr.c_str()));

			if(strlen(info.externalAddress) > 0)
			{
				INFO_MSG(fmt::format("\texternalCustomAddr : {}\n", info.externalAddress));
			}

			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			infostr += (fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			infostr += (fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			infostr += (fmt::format("\texternalAddr : {}\n", externalAddr.c_str()));

			if(strlen(info.externalAddress) > 0)
			{
				infostr +=  (fmt::format("\texternalCustomAddr : {}\n", info.externalAddress));
			}

			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}

		_updateEmailInfos();
	}
	else if (componentType == BASEAPPMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getBaseAppMgr();
		info.internalAddr = const_cast<Network::Address*>(&internalAddr);
		info.externalAddr = const_cast<Network::Address*>(&externalAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str()));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == CELLAPPMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getCellAppMgr();
		info.internalAddr = const_cast<Network::Address*>(&internalAddr);
		info.externalAddr = const_cast<Network::Address*>(&externalAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str());
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == DBMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getDBMgr();
		info.internalAddr = const_cast<Network::Address*>(&internalAddr);
		info.externalAddr = const_cast<Network::Address*>(&externalAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str()));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == LOGINAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getLoginApp();
		info.internalAddr = const_cast<Network::Address*>(&internalAddr);
		info.externalAddr = const_cast<Network::Address*>(&externalAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			INFO_MSG(fmt::format("\texternalAddr : {}\n", externalAddr.c_str()));
			if(strlen(info.externalAddress) > 0)
			{
				INFO_MSG(fmt::format("\texternalCustomAddr : {}\n", info.externalAddress));
			}

			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			infostr += (fmt::format("\texternalAddr : {}\n", externalAddr.c_str()));

			if(strlen(info.externalAddress) > 0)
			{
				infostr +=  (fmt::format("\texternalCustomAddr : {}\n", info.externalAddress));
			}

			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}

		_updateEmailInfos();
	}
	else if (componentType == MACHINE_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getKBMachine();
		info.internalAddr = const_cast<Network::Address*>(&internalAddr);
		info.externalAddr = const_cast<Network::Address*>(&externalAddr);
		info.componentID = componentID;
		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			//INFO_MSG("\texternalAddr : %s\n", externalAddr.c_str()));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalAddr : {}\n", internalAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
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
