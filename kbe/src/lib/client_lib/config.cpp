// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "config.h"
#include "network/common.h"
#include "network/address.h"
#include "resmgr/resmgr.h"
#include "entitydef/entitydef.h"
#include "server/serverconfig.h"
#include "common/kbeversion.h"

namespace KBEngine{
KBE_SINGLETON_INIT(Config);

ServerConfig g_ServerConfig;

//-------------------------------------------------------------------------------------
Config::Config():
gameUpdateHertz_(10),
tcp_SOMAXCONN_(5),
port_(0),
channelInternalTimeout_(60.0f),
channelExternalTimeout_(60.0f),
encrypt_login_(0),
fileName_(),
useLastAccountName_(false),
telnet_port(0),
telnet_passwd(),
telnet_deflayer(),
isOnInitCallPropertysSetMethods_(true)
{
}

//-------------------------------------------------------------------------------------
Config::~Config()
{
}

//-------------------------------------------------------------------------------------
bool Config::loadConfig(std::string fileName)
{
	fileName_ = fileName;
	TiXmlNode* rootNode = NULL;
	SmartPointer<XML> xml(new XML(Resmgr::getSingleton().matchRes(fileName_).c_str()));

	if(!xml->isGood())
	{
		ERROR_MSG(fmt::format("Config::loadConfig: load {} is failed!\n",
			fileName.c_str()));

		return false;
	}
	
	if(xml->getRootNode() == NULL)
	{
		// root节点下没有子节点了
		return true;
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

	rootNode = xml->getRootNode("channelCommon");
	if(rootNode != NULL)
	{
		TiXmlNode* childnode = xml->enterNode(rootNode, "timeout");
		if(childnode)
		{
			TiXmlNode* childnode1 = xml->enterNode(childnode, "internal");
			if(childnode1)
			{
				channelInternalTimeout_ = KBE_MAX(0.f, float(xml->getValFloat(childnode1)));
				Network::g_channelInternalTimeout = channelInternalTimeout_;
			}

			childnode1 = xml->enterNode(childnode, "external");
			if(childnode)
			{
				channelExternalTimeout_ = KBE_MAX(0.f, float(xml->getValFloat(childnode1)));
				Network::g_channelExternalTimeout = channelExternalTimeout_;
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

		TiXmlNode* rudpChildnode = xml->enterNode(rootNode, "reliableUDP");
		if (rudpChildnode)
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

	rootNode = xml->getRootNode("telnet_service");
	if(rootNode != NULL)
	{
		TiXmlNode* childnode = xml->enterNode(rootNode, "port");
		if(childnode)
		{
			telnet_port = xml->getValInt(childnode);
		}

		childnode = xml->enterNode(rootNode, "password");
		if(childnode)
		{
			telnet_passwd = xml->getValStr(childnode);
		}

		childnode = xml->enterNode(rootNode, "default_layer");
		if(childnode)
		{
			telnet_deflayer = xml->getValStr(childnode);
		}
	}

	rootNode = xml->getRootNode("gameUpdateHertz");
	if(rootNode != NULL){
		gameUpdateHertz_ = xml->getValInt(rootNode);
	}

	rootNode = xml->getRootNode("ip");
	if(rootNode != NULL)
	{
		strcpy(ip_, xml->getValStr(rootNode).c_str());
	}

	rootNode = xml->getRootNode("port");
	if(rootNode != NULL){
		port_ = xml->getValInt(rootNode);
	}

	rootNode = xml->getRootNode("entryScriptFile");
	if(rootNode != NULL)
	{
		strcpy(entryScriptFile_, xml->getValStr(rootNode).c_str());
	}

	rootNode = xml->getRootNode("accountName");
	if(rootNode != NULL)
	{
		strcpy(accountName_, xml->getValStr(rootNode).c_str());
	}

	rootNode = xml->getRootNode("useLastAccountName");
	if(rootNode != NULL)
	{
		useLastAccountName_ = xml->getValStr(rootNode) != "false";
	}
	
	rootNode = xml->getRootNode("encrypt_login");
	if(rootNode != NULL)
	{
		encrypt_login_ = xml->getValInt(rootNode);
	}
	
	rootNode = xml->getRootNode("aliasEntityID");
	if(rootNode != NULL)
	{
		EntityDef::entityAliasID((xml->getValStr(rootNode) == "true"));
	}

	rootNode = xml->getRootNode("entitydefAliasID");
	if(rootNode != NULL){
		EntityDef::entitydefAliasID((xml->getValStr(rootNode) == "true"));
	}

	rootNode = xml->getRootNode("isOnInitCallPropertysSetMethods");
	if (rootNode != NULL)
		isOnInitCallPropertysSetMethods_ = (xml->getValStr(rootNode) == "true");

	return true;
}

//-------------------------------------------------------------------------------------	
uint32 Config::tcp_SOMAXCONN()
{
	return tcp_SOMAXCONN_;
}

//-------------------------------------------------------------------------------------	
void Config::writeAccountName(const char* name)
{
	if(!useLastAccountName_)
		return;

	TiXmlNode* rootNode = NULL;
	XML* xml = new XML(Resmgr::getSingleton().matchRes(fileName_).c_str());

	if(!xml->isGood())
	{
		ERROR_MSG(fmt::format("Config::writeAccountName: load {} is failed!\n",
			fileName_.c_str()));

		SAFE_RELEASE(xml);
		return;
	}

	rootNode = xml->getRootNode("accountName");
	if(rootNode != NULL)
	{
		rootNode->SetValue(name);
	}

	xml->getTxdoc()->SaveFile(fileName_.c_str());
	SAFE_RELEASE(xml);
}

//-------------------------------------------------------------------------------------		
}
