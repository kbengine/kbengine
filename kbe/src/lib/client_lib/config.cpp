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


#include "config.hpp"
#include "network/common.hpp"
#include "network/address.hpp"
#include "resmgr/resmgr.hpp"

namespace KBEngine{
KBE_SINGLETON_INIT(Config);

//-------------------------------------------------------------------------------------
Config::Config():
gameUpdateHertz_(10),
tcp_SOMAXCONN_(5),
port_(0),
channelInternalTimeout_(60.0f),
channelExternalTimeout_(60.0f),
fileName_(),
useLastAccountName_(false)
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
	TiXmlNode* node = NULL, *rootNode = NULL;
	XmlPlus* xml = new XmlPlus(Resmgr::getSingleton().matchRes(fileName_).c_str());

	if(!xml->isGood())
	{
		ERROR_MSG(boost::format("Config::loadConfig: load %1% is failed!\n") %
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
	
	rootNode = xml->getRootNode("channelCommon");
	if(rootNode != NULL)
	{
		TiXmlNode* childnode = xml->enterNode(rootNode, "timeout");
		if(childnode)
		{
			TiXmlNode* childnode1 = xml->enterNode(childnode, "internal");
			if(childnode1)
			{
				channelInternalTimeout_ = KBE_MAX(1.f, float(xml->getValFloat(childnode1)));
				Mercury::g_channelInternalTimeout = channelInternalTimeout_;
			}

			childnode1 = xml->enterNode(childnode, "external");
			if(childnode)
			{
				channelExternalTimeout_ = KBE_MAX(1.f, float(xml->getValFloat(childnode1)));
				Mercury::g_channelExternalTimeout = channelExternalTimeout_;
			}
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
	
	SAFE_RELEASE(xml);
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

	TiXmlNode* node = NULL, *rootNode = NULL;
	XmlPlus* xml = new XmlPlus(Resmgr::getSingleton().matchRes(fileName_).c_str());

	if(!xml->isGood())
	{
		ERROR_MSG(boost::format("Config::writeAccountName: load %1% is failed!\n") %
			fileName_.c_str());

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
