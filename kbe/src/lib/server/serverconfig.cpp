// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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

static bool g_dbmgr_addDefaultAddress = true;

//-------------------------------------------------------------------------------------
ServerConfig::ServerConfig():
	gameUpdateHertz_(10),
	tick_max_buffered_logs_(4096),
	tick_max_sync_logs_(32),
	channelCommon_(),
	bitsPerSecondToClient_(0),
	interfacesAddress_(),
	interfacesPort_min_(0),
	interfacesPort_max_(0),
	interfacesAddrs_(),
	interfaces_orders_timeout_(0),
	shutdown_time_(1.f),
	shutdown_waitTickTime_(1.f),
	callback_timeout_(180.f),
	thread_timeout_(300.f),
	thread_init_create_(1),
	thread_pre_create_(2),
	thread_max_create_(8),
	emailServerInfo_(),
	emailAtivationInfo_(),
	emailResetPasswordInfo_(),
	emailBindInfo_()
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
		// root节点下没有子节点了
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
						
						// 不debug加密包
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

				childnode1 = xml->enterNode(sendNode, "tickSentBytes");
				if (childnode1)
				{
					TiXmlNode* childnode2 = xml->enterNode(childnode1, "internal");
					if (childnode2)
						Network::g_intSentWindowBytesOverflow = KBE_MAX(0, xml->getValInt(childnode2));

					childnode2 = xml->enterNode(childnode1, "external");
					if (childnode2)
						Network::g_extSentWindowBytesOverflow = KBE_MAX(0, xml->getValInt(childnode2));
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
		}

		childnode = xml->enterNode(rootNode, "encrypt_type");
		if(childnode)
		{
			Network::g_channelExternalEncryptType = xml->getValInt(childnode);
		}

		childnode = xml->enterNode(rootNode, "sslCertificate");
		if (childnode)
		{
			Network::g_sslCertificate = xml->getValStr(childnode);
		}

		childnode = xml->enterNode(rootNode, "sslPrivateKey");
		if (childnode)
		{
			Network::g_sslPrivateKey = xml->getValStr(childnode);
		}

		TiXmlNode* rudpChildnode = xml->enterNode(rootNode, "reliableUDP");
		if(rudpChildnode)
		{
			childnode = xml->enterNode(rudpChildnode, "readPacketsQueueSize");
			if (childnode)
			{
				TiXmlNode* childnode1 = xml->enterNode(childnode, "internal");
				if (childnode1)
					Network::g_rudp_intReadPacketsQueueSize = KBE_MAX(0, xml->getValInt(childnode1));

				childnode1 = xml->enterNode(childnode, "external");
				if (childnode1)
					Network::g_rudp_extReadPacketsQueueSize = KBE_MAX(0, xml->getValInt(childnode1));
			}

			childnode = xml->enterNode(rudpChildnode, "writePacketsQueueSize");
			if (childnode)
			{
				TiXmlNode* childnode1 = xml->enterNode(childnode, "internal");
				if (childnode1)
					Network::g_rudp_intWritePacketsQueueSize = KBE_MAX(0, xml->getValInt(childnode1));

				childnode1 = xml->enterNode(childnode, "external");
				if (childnode1)
					Network::g_rudp_extWritePacketsQueueSize = KBE_MAX(0, xml->getValInt(childnode1));
			}

			childnode = xml->enterNode(rudpChildnode, "tickInterval");
			if (childnode)
			{
				Network::g_rudp_tickInterval = KBE_MAX(0, xml->getValInt(childnode));
			}

			childnode = xml->enterNode(rudpChildnode, "minRTO");
			if (childnode)
			{
				Network::g_rudp_minRTO = KBE_MAX(0, xml->getValInt(childnode));
			}

			childnode = xml->enterNode(rudpChildnode, "mtu");
			if (childnode)
			{
				Network::g_rudp_mtu = KBE_MAX(0, xml->getValInt(childnode));
			}

			childnode = xml->enterNode(rudpChildnode, "missAcksResend");
			if (childnode)
			{
				Network::g_rudp_missAcksResend = KBE_MAX(0, xml->getValInt(childnode));
			}

			childnode = xml->enterNode(rudpChildnode, "congestionControl");
			if (childnode)
			{
				Network::g_rudp_congestionControl = (xml->getValStr(childnode) == "true");
			}

			childnode = xml->enterNode(rudpChildnode, "nodelay");
			if (childnode)
			{
				Network::g_rudp_nodelay = (xml->getValStr(childnode) == "true");
			}
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
			strncpy((char*)&_interfacesInfo.entryScriptFile, xml->getValStr(childnode).c_str(), MAX_NAME - 1);

		childnode = xml->enterNode(rootNode, "host");
		if(childnode)
		{
			interfacesAddress_ = xml->getValStr(childnode);
		}

		childnode = xml->enterNode(rootNode, "port_min");
		if(childnode)
		{
			interfacesPort_min_ = xml->getValInt(childnode);

			if(interfacesPort_min_ <= 0)
				interfacesPort_min_ = KBE_INTERFACES_TCP_PORT;
		}

		childnode = xml->enterNode(rootNode, "port_max");
		if (childnode)
		{
			interfacesPort_max_ = xml->getValInt(childnode);

			if (interfacesPort_max_ <= 0)
				interfacesPort_max_ = interfacesPort_min_;
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
			strncpy((char*)&_cellAppInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "entryScriptFile");	
		if(node != NULL)
			strncpy((char*)&_cellAppInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME - 1);
		
		TiXmlNode* viewNode = xml->enterNode(rootNode, "defaultViewRadius");
		if(viewNode != NULL)
		{
			node = NULL;
			node = xml->enterNode(viewNode, "radius");
			if(node != NULL)
				_cellAppInfo.defaultViewRadius = float(xml->getValFloat(node));
					
			node = xml->enterNode(viewNode, "hysteresisArea");
			if(node != NULL)
				_cellAppInfo.defaultViewHysteresisArea = float(xml->getValFloat(node));
		}
			
		node = xml->enterNode(rootNode, "ids");
		if(node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "criticallyLowSize");
			if(childnode)
			{
				_cellAppInfo.ids_criticallyLowSize = xml->getValInt(childnode);
				if (_cellAppInfo.ids_criticallyLowSize < 100)
					_cellAppInfo.ids_criticallyLowSize = 100;
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

			childnode = xml->enterNode(node, "entity_posdir_updates");
			if (childnode)
			{
				TiXmlNode* node = xml->enterNode(childnode, "type");
				if (node)
					_cellAppInfo.entity_posdir_updates_type = xml->getValInt(node);

				node = xml->enterNode(childnode, "smartThreshold");
				if (node)
					_cellAppInfo.entity_posdir_updates_smart_threshold = xml->getValInt(node);
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
			strncpy((char*)&_baseAppInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "externalInterface");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.externalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "externalAddress");	
		if(node != NULL)
			strncpy((char*)&_baseAppInfo.externalAddress, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "externalTcpPorts_min");
		if(node != NULL)	
			_baseAppInfo.externalTcpPorts_min = xml->getValInt(node);

		node = xml->enterNode(rootNode, "externalTcpPorts_max");
		if(node != NULL)	
			_baseAppInfo.externalTcpPorts_max = xml->getValInt(node);

		if(_baseAppInfo.externalTcpPorts_min < 0)
			_baseAppInfo.externalTcpPorts_min = -1;
		if(_baseAppInfo.externalTcpPorts_max < _baseAppInfo.externalTcpPorts_min)
			_baseAppInfo.externalTcpPorts_max = _baseAppInfo.externalTcpPorts_min;

		node = xml->enterNode(rootNode, "externalUdpPorts_min");
		if (node != NULL)
			_baseAppInfo.externalUdpPorts_min = xml->getValInt(node);

		node = xml->enterNode(rootNode, "externalUdpPorts_max");
		if (node != NULL)
			_baseAppInfo.externalUdpPorts_max = xml->getValInt(node);

		if (_baseAppInfo.externalUdpPorts_min < 0)
			_baseAppInfo.externalUdpPorts_min = -1;
		if (_baseAppInfo.externalUdpPorts_max < _baseAppInfo.externalUdpPorts_min)
			_baseAppInfo.externalUdpPorts_max = _baseAppInfo.externalUdpPorts_min;

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
				_baseAppInfo.ids_criticallyLowSize = xml->getValInt(childnode);
				if (_baseAppInfo.ids_criticallyLowSize < 100)
					_baseAppInfo.ids_criticallyLowSize = 100;
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
			strncpy((char*)&_dbmgrInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME - 1);
		
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

		node = xml->enterNode(rootNode, "ids");
		if (node != NULL)
		{
			TiXmlNode* childnode = xml->enterNode(node, "increasing_range");
			if (childnode)
			{
				_dbmgrInfo.ids_increasing_range = xml->getValInt(childnode);
			}
		}

		node = xml->enterNode(rootNode, "InterfacesServiceAddr");
		if (node != NULL)
		{
			TiXmlNode* loopNode = node;

			do
			{
				if (TiXmlNode::TINYXML_COMMENT == loopNode->Type())
					continue;

				std::string name = loopNode->Value();
				name = strutil::kbe_trim(name);

				if (name == "item")
				{
					if (loopNode->FirstChild() != NULL)
					{
						TiXmlNode* host_node = xml->enterNode(loopNode->FirstChild(), "host");
						TiXmlNode* port_node = xml->enterNode(loopNode->FirstChild(), "port");
						if (host_node && port_node)
						{
							std::string ip = xml->getValStr(host_node);
							int port = xml->getValInt(port_node);

							if (port <= 0)
								port = KBE_INTERFACES_TCP_PORT;

							Network::Address addr(ip, port);
							interfacesAddrs_.push_back(addr);
						}
					}
				}
			} while ((loopNode = loopNode->NextSibling()));

			TiXmlNode* childnode = xml->enterNode(node, "addDefaultAddress");
			if (childnode)
			{
				g_dbmgr_addDefaultAddress = xml->getValStr(childnode) == "true";
			}

			childnode = xml->enterNode(node, "enable");
			if (childnode)
			{
				if (xml->getValStr(childnode) != "true")
				{
					interfacesAddrs_.clear();
					g_dbmgr_addDefaultAddress = false;
				}
			}
		}

		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_dbmgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

		TiXmlNode* databaseInterfacesNode = xml->enterNode(rootNode, "databaseInterfaces");	
		if(databaseInterfacesNode != NULL)
		{
			if (databaseInterfacesNode->FirstChild() != NULL)
			{
				do
				{
					if (TiXmlNode::TINYXML_COMMENT == databaseInterfacesNode->Type())
						continue;
					
					std::vector<std::string> missingFields;
					missingFields.clear();

					std::string name = databaseInterfacesNode->Value();

					DBInterfaceInfo dbinfo;
					DBInterfaceInfo* pDBInfo = dbInterface(name);
					if (!pDBInfo)
						pDBInfo = &dbinfo;
					
					strncpy((char*)&pDBInfo->name, name.c_str(), MAX_NAME - 1);

					TiXmlNode* interfaceNode = databaseInterfacesNode->FirstChild();
					
					node = xml->enterNode(interfaceNode, "pure");
					if (node)
						pDBInfo->isPure = xml->getValStr(node) == "true";
					else
						missingFields.push_back("pure");

					// 默认库不允许是纯净库，引擎需要创建实体表
					if (name == "default")
						pDBInfo->isPure = false;

					node = xml->enterNode(interfaceNode, "type");
					if(node != NULL)
						strncpy((char*)&pDBInfo->db_type, xml->getValStr(node).c_str(), MAX_NAME - 1);
					else
						missingFields.push_back("type");

					
					node = xml->enterNode(interfaceNode, "host");
					if(node != NULL)
						strncpy((char*)&pDBInfo->db_ip, xml->getValStr(node).c_str(), MAX_IP - 1);
					else
						missingFields.push_back("host");

					node = xml->enterNode(interfaceNode, "port");
					if(node != NULL)
						pDBInfo->db_port = xml->getValInt(node);
					else
						missingFields.push_back("port");

					node = xml->enterNode(interfaceNode, "auth");
					if(node != NULL)
					{
						TiXmlNode* childnode = xml->enterNode(node, "password");
						if(childnode)
						{
							strncpy((char*)&pDBInfo->db_password, xml->getValStr(childnode).c_str(), MAX_BUF * 10 - 1);
						}
						else
						{
							missingFields.push_back("auth->password");
						}

						childnode = xml->enterNode(node, "username");
						if(childnode)
						{
							strncpy((char*)&pDBInfo->db_username, xml->getValStr(childnode).c_str(), MAX_NAME - 1);
						}
						else
						{
							missingFields.push_back("auth->username");
						}

						childnode = xml->enterNode(node, "encrypt");
						if(childnode)
						{
							pDBInfo->db_passwordEncrypt = xml->getValStr(childnode) == "true";
						}
						else
						{
							missingFields.push_back("auth->encrypt");
						}
					}
					else
					{
						missingFields.push_back("auth");
					}
						
					node = xml->enterNode(interfaceNode, "databaseName");
					if(node != NULL)
						strncpy((char*)&pDBInfo->db_name, xml->getValStr(node).c_str(), MAX_NAME - 1);
					else
						missingFields.push_back("databaseName");

					node = xml->enterNode(interfaceNode, "numConnections");
					if(node != NULL)
						pDBInfo->db_numConnections = xml->getValInt(node);
					else
						missingFields.push_back("numConnections");
						
					node = xml->enterNode(interfaceNode, "unicodeString");
					if(node != NULL)
					{
						TiXmlNode* childnode = xml->enterNode(node, "characterSet");
						if(childnode)
						{
							pDBInfo->db_unicodeString_characterSet = xml->getValStr(childnode);
						}
						else
						{
							missingFields.push_back("unicodeString->characterSet");
						}

						childnode = xml->enterNode(node, "collation");
						if(childnode)
						{
							pDBInfo->db_unicodeString_collation = xml->getValStr(childnode);
						}
						else
						{
							missingFields.push_back("unicodeString->collation");
						}
					}
					else
					{
						missingFields.push_back("unicodeString");
					}

					if (pDBInfo->db_unicodeString_characterSet.size() == 0)
						pDBInfo->db_unicodeString_characterSet = "utf8";

					if (pDBInfo->db_unicodeString_collation.size() == 0)
						pDBInfo->db_unicodeString_collation = "utf8_bin";
	
					if (pDBInfo == &dbinfo)
					{
						// 检查不能在不同的接口中使用相同的数据库与相同的表
						std::vector<DBInterfaceInfo>::iterator dbinfo_iter = _dbmgrInfo.dbInterfaceInfos.begin();
						for (; dbinfo_iter != _dbmgrInfo.dbInterfaceInfos.end(); ++dbinfo_iter)
						{
							if (kbe_stricmp((*dbinfo_iter).db_ip, dbinfo.db_ip) == 0 && 
								kbe_stricmp((*dbinfo_iter).db_type, dbinfo.db_type) == 0 &&
								(*dbinfo_iter).db_port == dbinfo.db_port &&
								strcmp(dbinfo.db_name, (*dbinfo_iter).db_name) == 0)
							{
								ERROR_MSG(fmt::format("ServerConfig::loadConfig: databaseInterfaces, Conflict between \"{}=(databaseName={})\" and \"{}=(databaseName={})\", file={}!\n",
									(*dbinfo_iter).name, (*dbinfo_iter).db_name, dbinfo.name, dbinfo.db_name, fileName.c_str()));

								return false;
							}
						}

						if (fileName == "server/kbengine_defaults.xml" && !missingFields.empty())
						{
							std::vector<std::string>::const_iterator iter = missingFields.begin();
							for (; iter != missingFields.end(); iter++)
							{
								ERROR_MSG(fmt::format("ServerConfig::loadConfig: kbengine_defaults.xml error, databaseInterface({}) missing filed:{}.\n", name, *iter));
							}

							return false;
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

		node = xml->enterNode(rootNode, "shareDB");
		if (node != NULL) {
			_dbmgrInfo.isShareDB = (xml->getValStr(node) == "true");
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
				strncpy((char*)&_dbmgrInfo.dbAccountEntityScriptType, xml->getValStr(childnode).c_str(), MAX_NAME - 1);
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

			childnode = xml->enterNode(node, "account_resetPassword");
			if (childnode != NULL)
			{
				TiXmlNode* childchildnode = xml->enterNode(childnode, "enable");
				if (childchildnode)
				{
					_dbmgrInfo.account_reset_password_enable = (xml->getValStr(childchildnode) == "true");
				}
			}
		}
	}

	rootNode = xml->getRootNode("loginapp");
	if(rootNode != NULL)
	{
		node = xml->enterNode(rootNode, "entryScriptFile");
		if (node != NULL)
			strncpy((char*)&_loginAppInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME - 1);
		
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
			strncpy((char*)&_loginAppInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "externalInterface");	
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.externalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "externalAddress");	
		if(node != NULL)
			strncpy((char*)&_loginAppInfo.externalAddress, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "externalTcpPorts_min");
		if(node != NULL)	
			_loginAppInfo.externalTcpPorts_min = xml->getValInt(node);

		node = xml->enterNode(rootNode, "externalTcpPorts_max");
		if(node != NULL)	
			_loginAppInfo.externalTcpPorts_max = xml->getValInt(node);

		if(_loginAppInfo.externalTcpPorts_min < 0)
			_loginAppInfo.externalTcpPorts_min = -1;
		if(_loginAppInfo.externalTcpPorts_max < _loginAppInfo.externalTcpPorts_min)
			_loginAppInfo.externalTcpPorts_max = _loginAppInfo.externalTcpPorts_min;

		node = xml->enterNode(rootNode, "externalUdpPorts_min");
		if (node != NULL)
			_loginAppInfo.externalUdpPorts_min = xml->getValInt(node);

		node = xml->enterNode(rootNode, "externalUdpPorts_max");
		if (node != NULL)
			_loginAppInfo.externalUdpPorts_max = xml->getValInt(node);

		if (_loginAppInfo.externalUdpPorts_min < 0)
			_loginAppInfo.externalUdpPorts_min = -1;
		if (_loginAppInfo.externalUdpPorts_max < _loginAppInfo.externalUdpPorts_min)
			_loginAppInfo.externalUdpPorts_max = _loginAppInfo.externalUdpPorts_min;

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
			strncpy((char*)&_cellAppMgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

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
			strncpy((char*)&_baseAppMgrInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

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
			strncpy((char*)&_kbMachineInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "externalInterface");	
		if(node != NULL)
			strncpy((char*)&_kbMachineInfo.externalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "externalTcpPorts_min");
		if(node != NULL)	
			_kbMachineInfo.externalTcpPorts_min = xml->getValInt(node);

		node = xml->enterNode(rootNode, "externalTcpPorts_max");
		if(node != NULL)	
			_kbMachineInfo.externalTcpPorts_max = xml->getValInt(node);

		if(_kbMachineInfo.externalTcpPorts_min < 0)
			_kbMachineInfo.externalTcpPorts_min = 0;
		if(_kbMachineInfo.externalTcpPorts_max < _kbMachineInfo.externalTcpPorts_min)
			_kbMachineInfo.externalTcpPorts_max = _kbMachineInfo.externalTcpPorts_min;

		node = xml->enterNode(rootNode, "externalUdpPorts_min");
		if (node != NULL)
			_kbMachineInfo.externalUdpPorts_min = xml->getValInt(node);

		node = xml->enterNode(rootNode, "externalUdpPorts_max");
		if (node != NULL)
			_kbMachineInfo.externalUdpPorts_max = xml->getValInt(node);

		if (_kbMachineInfo.externalUdpPorts_min < 0)
			_kbMachineInfo.externalUdpPorts_min = 0;
		if (_kbMachineInfo.externalUdpPorts_max < _kbMachineInfo.externalUdpPorts_min)
			_kbMachineInfo.externalUdpPorts_max = _kbMachineInfo.externalUdpPorts_min;

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_kbMachineInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}
		
		node = xml->enterNode(rootNode, "addresses");
		if(node)
		{
			do
			{
				if (TiXmlNode::TINYXML_COMMENT == node->Type())
					continue;

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
			strncpy((char*)&_botsInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "internalInterface");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "host");	
		if(node != NULL)
			strncpy((char*)&_botsInfo.login_ip, xml->getValStr(node).c_str(), MAX_IP - 1);

		node = xml->enterNode(rootNode, "port_min");	
		if(node != NULL)
			_botsInfo.login_port_min = xml->getValInt(node);

		node = xml->enterNode(rootNode, "port_max");
		if (node != NULL)
			_botsInfo.login_port_max = xml->getValInt(node);

		if (_botsInfo.login_port_min < 0)
			_botsInfo.login_port_min = 0;
			
		if (_botsInfo.login_port_max < _botsInfo.login_port_min)
			_botsInfo.login_port_max = _botsInfo.login_port_min;
		
		_botsInfo.login_port = _botsInfo.login_port_min;

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

			childnode = xml->enterNode(node, "account_password");
			if (childnode)
			{
				_botsInfo.bots_account_passwd = xml->getValStr(childnode);
			}
		}

		node = xml->enterNode(rootNode, "SOMAXCONN");
		if(node != NULL){
			_botsInfo.tcp_SOMAXCONN = xml->getValInt(node);
		}

		node = xml->enterNode(rootNode, "forceInternalLogin");
		if (node != NULL){
			_botsInfo.forceInternalLogin = (xml->getValStr(node) == "true");
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
			strncpy((char*)&_loggerInfo.internalInterface, xml->getValStr(node).c_str(), MAX_NAME - 1);

		node = xml->enterNode(rootNode, "entryScriptFile");
		if (node != NULL)
			strncpy((char*)&_loggerInfo.entryScriptFile, xml->getValStr(node).c_str(), MAX_NAME - 1);

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
	// 如果小于64则表示目前还是明文密码
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
			ERROR_MSG("ServerConfig::loadConfig: email password(email_service.xml) encrypt error!\n");
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
				strncpy(buf, inet_ntoa(*(struct in_addr*)host->h_addr_list[0]), MAX_BUF - 1);
			}	
		}
	}
}

//-------------------------------------------------------------------------------------	
void ServerConfig::updateInfos(bool isPrint, COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
							   const Network::Address& internalTcpAddr, const Network::Address& externalTcpAddr, const Network::Address& externalUdpAddr)
{
	std::string infostr = "";

	for (size_t i = 0; i < _dbmgrInfo.dbInterfaceInfos.size(); ++i)
		_dbmgrInfo.dbInterfaceInfos[i].index = i;

	if (g_dbmgr_addDefaultAddress)
	{
		Network::Address interfacesAddr(interfacesAddress_, interfacesPort_min_);
		interfacesAddrs_.insert(interfacesAddrs_.begin(), interfacesAddr);
	}

	//updateExternalAddress(getBaseApp().externalTcpAddr);
	//updateExternalAddress(getLoginApp().externalTcpAddr);

	if(componentType == CELLAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getCellApp();
		info.internalTcpAddr = &internalTcpAddr;
		info.externalTcpAddr = &externalTcpAddr;
		info.externalUdpAddr = &externalUdpAddr;
		info.componentID = componentID;

		if (info.ids_criticallyLowSize > getDBMgr().ids_increasing_range / 2)
		{
			info.ids_criticallyLowSize = getDBMgr().ids_increasing_range / 2;
			ERROR_MSG(fmt::format("kbengine[_defs].xml->cellapp->ids->criticallyLowSize > dbmgr->ids->increasing_range / 2, Force adjustment to criticallyLowSize({})\n", 
				info.ids_criticallyLowSize));
		}

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			INFO_MSG(fmt::format("\tdefaultViewRadius : {}\n", info.defaultViewRadius));
			INFO_MSG(fmt::format("\tdefaultViewHysteresisArea : {}\n", info.defaultViewHysteresisArea));
			INFO_MSG(fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//INFO_MSG(fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			infostr += (fmt::format("\tdefaultViewRadius : {}\n", info.defaultViewRadius));
			infostr += (fmt::format("\tdefaultViewHysteresisArea : {}\n", info.defaultViewHysteresisArea));
			infostr += (fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//infostr += (fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == BASEAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getBaseApp();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.externalUdpAddr = const_cast<Network::Address*>(&externalUdpAddr);
		info.componentID = componentID;

		if (info.ids_criticallyLowSize > getDBMgr().ids_increasing_range / 2)
		{
			info.ids_criticallyLowSize = getDBMgr().ids_increasing_range / 2;
			ERROR_MSG(fmt::format("kbengine[_defs].xml->baseapp->ids->criticallyLowSize > dbmgr->ids->increasing_range / 2, Force adjustment to criticallyLowSize({})\n",
				info.ids_criticallyLowSize));
		}

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			INFO_MSG(fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			INFO_MSG(fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));
			INFO_MSG(fmt::format("\texternalUdpAddr : {}\n", externalUdpAddr.c_str()));

			if(strlen(info.externalAddress) > 0)
			{
				INFO_MSG(fmt::format("\texternalCustomAddr : {}\n", info.externalAddress));
			}

			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tgameUpdateHertz : {}\n", gameUpdateHertz()));
			infostr += (fmt::format("\tentryScriptFile : {}\n", info.entryScriptFile));
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));
			infostr += (fmt::format("\texternalUdpAddr : {}\n", externalUdpAddr.c_str()));

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
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//INFO_MSG((fmt::format("\texternalTcpAddr : %s\n", externalTcpAddr.c_str())));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == CELLAPPMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getCellAppMgr();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//INFO_MSG((fmt::format("\texternalTcpAddr : %s\n", externalTcpAddr.c_str())));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == DBMGR_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getDBMgr();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;

		if (info.ids_increasing_range < 500)
		{
			info.ids_increasing_range = 500;
			ERROR_MSG(fmt::format("kbengine[_defs].xml-> dbmgr->ids->increasing_range too small, Force adjustment to ids_increasing_range({})\n",
				info.ids_increasing_range));
		}

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//INFO_MSG((fmt::format("\texternalTcpAddr : %s\n", externalTcpAddr.c_str())));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == LOGINAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getLoginApp();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;

		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			INFO_MSG(fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));
			if(strlen(info.externalAddress) > 0)
			{
				INFO_MSG(fmt::format("\texternalCustomAddr : {}\n", info.externalAddress));
			}

			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\texternalTcpAddr : {}\n", externalTcpAddr.c_str()));

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
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;
		if(isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			//INFO_MSG((fmt::format("\texternalTcpAddr : %s\n", externalTcpAddr.c_str())));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			infostr += (fmt::format("\tcomponentID : {}\n", info.componentID));
		}
	}
	else if (componentType == INTERFACES_TYPE)
	{
		ENGINE_COMPONENT_INFO info = getInterfaces();
		info.internalTcpAddr = const_cast<Network::Address*>(&internalTcpAddr);
		info.externalTcpAddr = const_cast<Network::Address*>(&externalTcpAddr);
		info.componentID = componentID;
		if (isPrint)
		{
			INFO_MSG("server-configs:\n");
			INFO_MSG(fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
			INFO_MSG((fmt::format("\texternalTcpAddr : %s\n", externalTcpAddr.c_str())));
			INFO_MSG(fmt::format("\tcomponentID : {}\n", info.componentID));

			infostr += "server-configs:\n";
			infostr += (fmt::format("\tinternalTcpAddr : {}\n", internalTcpAddr.c_str()));
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
