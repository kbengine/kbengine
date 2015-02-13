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


#include "network_interface.h"
#ifndef CODE_INLINE
#include "network_interface.inl"
#endif

#include "network/address.h"
#include "network/event_dispatcher.h"
#include "network/packet_receiver.h"
#include "network/listener_receiver.h"
#include "network/channel.h"
#include "network/packet.h"
#include "network/delayed_channels.h"
#include "network/interfaces.h"
#include "network/message_handler.h"

namespace KBEngine { 
namespace Network
{
const int NetworkInterface::RECV_BUFFER_SIZE = 16 * 1024 * 1024; // 16MB
const char * NetworkInterface::USE_KBEMACHINED = "kbemachine";

//-------------------------------------------------------------------------------------
NetworkInterface::NetworkInterface(Network::EventDispatcher * pDispatcher,
		int32 extlisteningPort_min, int32 extlisteningPort_max, const char * extlisteningInterface,
		uint32 extrbuffer, uint32 extwbuffer,
		int32 intlisteningPort, const char * intlisteningInterface,
		uint32 intrbuffer, uint32 intwbuffer):
	extEndpoint_(),
	intEndpoint_(),
	channelMap_(),
	pDispatcher_(pDispatcher),
	pExtensionData_(NULL),
	pExtListenerReceiver_(NULL),
	pIntListenerReceiver_(NULL),
	pDelayedChannels_(new DelayedChannels()),
	pChannelTimeOutHandler_(NULL),
	pChannelDeregisterHandler_(NULL),
	isExternal_(extlisteningPort_min != -1),
	numExtChannels_(0)
{
	if(isExternal())
	{
		pExtListenerReceiver_ = new ListenerReceiver(extEndpoint_, Channel::EXTERNAL, *this);
		this->recreateListeningSocket("EXTERNAL", htons(extlisteningPort_min), htons(extlisteningPort_max), 
			extlisteningInterface, &extEndpoint_, pExtListenerReceiver_, extrbuffer, extwbuffer);

		// 如果配置了对外端口范围， 如果范围过小这里extEndpoint_可能没有端口可用了
		if(extlisteningPort_min != -1)
		{
			KBE_ASSERT(extEndpoint_.good() && "Channel::EXTERNAL: no available port, "
				"please check for kbengine_defs.xml!\n");
		}
	}

	if(intlisteningPort != -1)
	{
		pIntListenerReceiver_ = new ListenerReceiver(intEndpoint_, Channel::INTERNAL, *this);
		this->recreateListeningSocket("INTERNAL", intlisteningPort, intlisteningPort, 
			intlisteningInterface, &intEndpoint_, pIntListenerReceiver_, intrbuffer, intwbuffer);
	}

	
	KBE_ASSERT(good() && "NetworkInterface::NetworkInterface: no available port, "
		"please check for kbengine_defs.xml!\n");

	pDelayedChannels_->init(this->dispatcher(), this);
}

//-------------------------------------------------------------------------------------
NetworkInterface::~NetworkInterface()
{
	ChannelMap::iterator iter = channelMap_.begin();
	while (iter != channelMap_.end())
	{
		ChannelMap::iterator oldIter = iter++;
		Channel * pChannel = oldIter->second;
		pChannel->destroy();
		delete pChannel;
	}

	channelMap_.clear();

	this->closeSocket();

	if (pDispatcher_ != NULL)
	{
		pDelayedChannels_->fini(this->dispatcher());
		pDispatcher_ = NULL;
	}

	SAFE_RELEASE(pDelayedChannels_);
	SAFE_RELEASE(pExtListenerReceiver_);
	SAFE_RELEASE(pIntListenerReceiver_);
}

//-------------------------------------------------------------------------------------
void NetworkInterface::closeSocket()
{
	if (extEndpoint_.good())
	{
		this->dispatcher().deregisterReadFileDescriptor(extEndpoint_);
		extEndpoint_.close();
	}

	if (intEndpoint_.good())
	{
		this->dispatcher().deregisterReadFileDescriptor(intEndpoint_);
		intEndpoint_.close();
	}
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::recreateListeningSocket(const char* pEndPointName, uint16 listeningPort_min, uint16 listeningPort_max, 
										const char * listeningInterface, EndPoint* pEP, ListenerReceiver* pLR, uint32 rbuffer, 
										uint32 wbuffer)
{
	KBE_ASSERT(listeningInterface && pEP && pLR);

	if (pEP->good())
	{
		this->dispatcher().deregisterReadFileDescriptor(*pEP);
		pEP->close();
	}

	Address address;
	address.ip = 0;
	address.port = 0;

	pEP->socket(SOCK_STREAM);
	if (!pEP->good())
	{
		ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): couldn't create a socket\n",
			pEndPointName));

		return false;
	}
	
	/*
	int val = 1;
	setsockopt((*pEP), SOL_SOCKET, SO_REUSEADDR,
			(const char*)&val, sizeof(val));
	*/
	
	this->dispatcher().registerReadFileDescriptor(*pEP, pLR);
	
	char ifname[IFNAMSIZ];
	u_int32_t ifaddr = INADDR_ANY;

	bool listeningInterfaceEmpty =
		(listeningInterface == NULL || listeningInterface[0] == 0);

	if(pEP->findIndicatedInterface(listeningInterface, ifname) == 0)
	{
		INFO_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Creating on interface '{}' (= {})\n",
			pEndPointName, listeningInterface, ifname));

		if (pEP->getInterfaceAddress( ifname, ifaddr ) != 0)
		{
			WARNING_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Couldn't get addr of interface {} so using all interfaces\n",
				pEndPointName, ifname));
		}
	}
	else if (!listeningInterfaceEmpty)
	{
		WARNING_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Couldn't parse interface spec '{}' so using all interfaces\n",
			pEndPointName, listeningInterface));
	}
	
	bool foundport = false;
	uint32 listeningPort = listeningPort_min;
	if(listeningPort_min != listeningPort_max)
	{
		for(int lpIdx=ntohs(listeningPort_min); lpIdx<ntohs(listeningPort_max); ++lpIdx)
		{
			listeningPort = htons(lpIdx);
			if (pEP->bind(listeningPort, ifaddr) != 0)
			{
				continue;
			}
			else
			{
				foundport = true;
				break;
			}
		}
	}
	else
	{
		if (pEP->bind(listeningPort, ifaddr) == 0)
		{
			foundport = true;
		}
	}

	if(!foundport)
	{
		ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Couldn't bind the socket to {}:{} ({})\n",
			pEndPointName, inet_ntoa((struct in_addr&)ifaddr), ntohs(listeningPort), kbe_strerror()));
		
		pEP->close();
		return false;
	}

	pEP->getlocaladdress( (u_int16_t*)&address.port,
		(u_int32_t*)&address.ip );

	if (address.ip == 0)
	{
		if (pEP->findDefaultInterface(ifname) != 0 ||
			pEP->getInterfaceAddress(ifname,
				(u_int32_t&)address.ip) != 0)
		{
			ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Couldn't determine ip addr of default interface\n", pEndPointName));

			pEP->close();
			return false;
		}

		INFO_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"bound to all interfaces with default route "
				"interface on {} ( {} )\n",
			pEndPointName, ifname, address.c_str()));
	}
	
	pEP->setnonblocking(true);
	pEP->setnodelay(true);
	pEP->addr(address);
	
	if(rbuffer > 0)
	{
		if (!pEP->setBufferSize(SO_RCVBUF, rbuffer))
		{
			WARNING_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Operating with a receive buffer of only {} bytes (instead of {})\n",
				pEndPointName, pEP->getBufferSize(SO_RCVBUF), rbuffer));
		}
	}
	if(wbuffer > 0)
	{
		if (!pEP->setBufferSize(SO_SNDBUF, wbuffer))
		{
			WARNING_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Operating with a send buffer of only {} bytes (instead of {})\n",
				pEndPointName, pEP->getBufferSize(SO_SNDBUF), wbuffer));
		}
	}

	int backlog = Network::g_SOMAXCONN;
	if(backlog < 5)
		backlog = 5;

	if(pEP->listen(backlog) == -1)
	{
		ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
			"listen to {} ({})\n",
			pEndPointName, address.c_str(), kbe_strerror()));

		pEP->close();
		return false;
	}
	
	INFO_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): address {}, SOMAXCONN={}.\n", 
		pEndPointName, address.c_str(), backlog));

	return true;
}

//-------------------------------------------------------------------------------------
void NetworkInterface::delayedSend(Channel & channel)
{
	pDelayedChannels_->add(channel);
}

//-------------------------------------------------------------------------------------
void NetworkInterface::sendIfDelayed(Channel & channel)
{
	pDelayedChannels_->sendIfDelayed(channel);
}

//-------------------------------------------------------------------------------------
void NetworkInterface::handleTimeout(TimerHandle handle, void * arg)
{
	INFO_MSG(fmt::format("NetworkInterface::handleTimeout: EXTERNAL({}), INTERNAL({}).\n", 
		extaddr().c_str(), intaddr().c_str()));
}

//-------------------------------------------------------------------------------------
Channel * NetworkInterface::findChannel(const Address & addr)
{
	if (addr.ip == 0)
		return NULL;

	ChannelMap::iterator iter = channelMap_.find(addr);
	Channel * pChannel = (iter != channelMap_.end()) ? iter->second : NULL;
	return pChannel;
}

//-------------------------------------------------------------------------------------
Channel * NetworkInterface::findChannel(int fd)
{
	ChannelMap::iterator iter = channelMap_.begin();
	for(; iter != channelMap_.end(); ++iter)
	{
		if(iter->second->pEndPoint() && *iter->second->pEndPoint() == fd)
			return iter->second;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::registerChannel(Channel* pChannel)
{
	const Address & addr = pChannel->addr();
	KBE_ASSERT(addr.ip != 0);
	KBE_ASSERT(&pChannel->networkInterface() == this);
	ChannelMap::iterator iter = channelMap_.find(addr);
	Channel * pExisting = iter != channelMap_.end() ? iter->second : NULL;

	if(pExisting)
	{
		CRITICAL_MSG(fmt::format("NetworkInterface::registerChannel: channel {} is exist.\n",
		pChannel->c_str()));
		return false;
	}

	channelMap_[addr] = pChannel;

	if(pChannel->isExternal())
		numExtChannels_++;

	//INFO_MSG(fmt::format("NetworkInterface::registerChannel: new channel: {}.\n", pChannel->c_str()));
	return true;
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::deregisterAllChannels()
{
	ChannelMap::iterator iter = channelMap_.begin();
	while (iter != channelMap_.end())
	{
		ChannelMap::iterator oldIter = iter++;
		Channel * pChannel = oldIter->second;
		pChannel->destroy();
		Network::Channel::ObjPool().reclaimObject(pChannel);
	}

	channelMap_.clear();
	numExtChannels_ = 0;

	return true;
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::deregisterChannel(Channel* pChannel)
{
	const Address & addr = pChannel->addr();
	KBE_ASSERT(pChannel->pEndPoint() != NULL);

	if(pChannel->isExternal())
		numExtChannels_--;

	//INFO_MSG(fmt::format("NetworkInterface::deregisterChannel: del channel: {}\n",
	//	pChannel->c_str()));

	if (!channelMap_.erase(addr))
	{
		CRITICAL_MSG(fmt::format("NetworkInterface::deregisterChannel: "
				"Channel not found {}!\n",
			pChannel->c_str()));

		return false;
	}

	if(pChannelDeregisterHandler_)
	{
		pChannelDeregisterHandler_->onChannelDeregister(pChannel);
	}	

	return true;
}

//-------------------------------------------------------------------------------------
void NetworkInterface::onChannelGone(Channel * pChannel)
{
}

//-------------------------------------------------------------------------------------
void NetworkInterface::onChannelTimeOut(Channel * pChannel)
{
	if (pChannelTimeOutHandler_)
	{
		pChannelTimeOutHandler_->onChannelTimeOut(pChannel);
	}
	else
	{
		ERROR_MSG(fmt::format("NetworkInterface::onChannelTimeOut: "
					"Channel {} timed out but no handler is registered.\n",
				pChannel->c_str()));
	}
}

//-------------------------------------------------------------------------------------
void NetworkInterface::processAllChannelPackets(KBEngine::Network::MessageHandlers* pMsgHandlers)
{
	ChannelMap::iterator iter = channelMap_.begin();
	for(; iter != channelMap_.end(); )
	{
		Network::Channel* pChannel = iter->second;

		if(pChannel->isDestroyed())
		{
			++iter;
		}
		else if(pChannel->isCondemn())
		{
			++iter;
			deregisterChannel(pChannel);
			pChannel->destroy();
			Network::Channel::ObjPool().reclaimObject(pChannel);
		}
		else
		{
			pChannel->processPackets(pMsgHandlers);
			++iter;
		}
	}
}
//-------------------------------------------------------------------------------------
}
}
