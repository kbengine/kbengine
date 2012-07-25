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


#include "network_interface.hpp"
#ifndef CODE_INLINE
#include "network_interface.ipp"
#endif

#include "network/address.hpp"
#include "network/event_dispatcher.hpp"
#include "network/packet_receiver.hpp"
#include "network/listener_receiver.hpp"
#include "network/channel.hpp"
#include "network/packet.hpp"
#include "network/delayed_channels.hpp"
#include "network/interfaces.hpp"
#include "network/message_handler.hpp"

namespace KBEngine { 
namespace Mercury
{
const int NetworkInterface::RECV_BUFFER_SIZE = 16 * 1024 * 1024; // 16MB
const char * NetworkInterface::USE_KBEMACHINED = "kbemachined";

//-------------------------------------------------------------------------------------
NetworkInterface::NetworkInterface(Mercury::EventDispatcher * pMainDispatcher,
		int32 extlisteningPort_min, int32 extlisteningPort_max, const char * extlisteningInterface,
		int32 intlisteningPort, const char * intlisteningInterface):
	extEndpoint_(),
	intEndpoint_(),
	channelMap_(),
	pDispatcher_(new EventDispatcher),
	pMainDispatcher_(NULL),
	pExtensionData_(NULL),
	pExtListenerReceiver_(NULL),
	pIntListenerReceiver_(NULL),
	pDelayedChannels_(new DelayedChannels()),
	pChannelTimeOutHandler_(NULL),
	isExternal_(extlisteningPort_min != -1)
{
	if(isExternal())
	{
		pExtListenerReceiver_ = new ListenerReceiver(extEndpoint_, Channel::EXTERNAL, *this);
		this->recreateListeningSocket("EXTERNAL", htons(extlisteningPort_min), htons(extlisteningPort_max), 
			extlisteningInterface, &extEndpoint_, pExtListenerReceiver_);
	}

	if(intlisteningPort != -1)
	{
		pIntListenerReceiver_ = new ListenerReceiver(intEndpoint_, Channel::INTERNAL, *this);
		this->recreateListeningSocket("INTERNAL", intlisteningPort, intlisteningPort, 
			intlisteningInterface, &intEndpoint_, pIntListenerReceiver_);
	}

	KBE_ASSERT(good());

	if (pMainDispatcher != NULL)
	{
		this->attach(*pMainDispatcher);
	}
}

//-------------------------------------------------------------------------------------
NetworkInterface::~NetworkInterface()
{
	ChannelMap::iterator iter = channelMap_.begin();
	while (iter != channelMap_.end())
	{
		ChannelMap::iterator oldIter = iter++;
		Channel * pChannel = oldIter->second;

		if (pChannel->isOwnedByInterface())
		{
			pChannel->destroy();
		}
		else
		{
			WARNING_MSG("NetworkInterface::~NetworkInterface: "
					"Channel to %s is still registered\n",
				pChannel->c_str());
		}
	}

	this->detach();
	this->closeSocket();

	SAFE_RELEASE(pDispatcher_);
	SAFE_RELEASE(pDelayedChannels_);
	SAFE_RELEASE(pExtListenerReceiver_);
	SAFE_RELEASE(pIntListenerReceiver_);
}

//-------------------------------------------------------------------------------------
void NetworkInterface::attach(EventDispatcher & mainDispatcher)
{
	KBE_ASSERT(pMainDispatcher_ == NULL);
	pMainDispatcher_ = &mainDispatcher;
	mainDispatcher.attach(this->dispatcher());
	
	pDelayedChannels_->init(this->mainDispatcher());
}

//-------------------------------------------------------------------------------------
void NetworkInterface::detach()
{
	if (pMainDispatcher_ != NULL)
	{
		pDelayedChannels_->fini(this->mainDispatcher());
		pMainDispatcher_->detach(this->dispatcher());
		pMainDispatcher_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
void NetworkInterface::closeSocket()
{
	if (extEndpoint_.good())
	{
		this->dispatcher().deregisterFileDescriptor(extEndpoint_);
		extEndpoint_.close();
		extEndpoint_.detach();
	}

	if (intEndpoint_.good())
	{
		this->dispatcher().deregisterFileDescriptor(intEndpoint_);
		intEndpoint_.close();
		intEndpoint_.detach();
	}
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::recreateListeningSocket(const char* pEndPointName, uint16 listeningPort_min, uint16 listeningPort_max, 
										const char * listeningInterface, EndPoint* pEP, ListenerReceiver* pLR)
{
	KBE_ASSERT(listeningInterface && pEP && pLR);

	if (pEP->good())
	{
		this->dispatcher().deregisterFileDescriptor(*pEP);
		pEP->close();
		pEP->detach();
	}

	Address address;
	address.ip = 0;
	address.port = 0;

	pEP->socket(SOCK_STREAM);
	if (!pEP->good())
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket(%s): couldn't create a socket\n", 
			pEndPointName);
		return false;
	}

	this->dispatcher().registerFileDescriptor(*pEP, pLR);
	
	char ifname[IFNAMSIZ];
	u_int32_t ifaddr = INADDR_ANY;
	bool listeningInterfaceEmpty =
		(listeningInterface == NULL || listeningInterface[0] == 0);

	// Query kbemachined over the local interface (dev: lo) for what it
	// believes the internal interface is.
	if (listeningInterface &&
		(strcmp(listeningInterface, USE_KBEMACHINED) == 0))
	{
		INFO_MSG( "NetworkInterface::recreateListeningSocket(%s): "
				"Querying KBEMachined for interface\n", pEndPointName);
		
		// 没有实现, 向KBEMachined查询接口
	}
	else if (pEP->findIndicatedInterface(listeningInterface, ifname) == 0)
	{
		INFO_MSG( "NetworkInterface::recreateListeningSocket(%s): "
				"Creating on interface '%s' (= %s)\n",
			pEndPointName, listeningInterface, ifname );
		if (pEP->getInterfaceAddress( ifname, ifaddr ) != 0)
		{
			WARNING_MSG( "NetworkInterface::recreateListeningSocket(%s): "
				"Couldn't get addr of interface %s so using all interfaces\n",
				pEndPointName, ifname );
		}
	}
	else if (!listeningInterfaceEmpty)
	{
		WARNING_MSG( "NetworkInterface::recreateListeningSocket(%s): "
				"Couldn't parse interface spec '%s' so using all interfaces\n",
			pEndPointName, listeningInterface );
	}
	
	bool foundport = false;
	uint32 listeningPort = listeningPort_min;
	if(listeningPort_min != listeningPort_max)
	{
		for(int lpIdx=ntohs(listeningPort_min); lpIdx<ntohs(listeningPort_max); lpIdx++)
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
		ERROR_MSG("NetworkInterface::recreateListeningSocket(%s): "
				"Couldn't bind the socket to %s (%s)\n",
			pEndPointName, address.c_str(), kbe_strerror());
		
		pEP->close();
		pEP->detach();
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
			ERROR_MSG( "NetworkInterface::recreateListeningSocket(%s): "
				"Couldn't determine ip addr of default interface\n", pEndPointName );

			pEP->close();
			pEP->detach();
			return false;
		}

		INFO_MSG( "NetworkInterface::recreateListeningSocket(%s): "
				"bound to all interfaces with default route "
				"interface on %s ( %s )\n",
			pEndPointName, ifname, address.c_str() );
	}
	
	pEP->setnonblocking(true);
	pEP->setnodelay(true);
	pEP->addr(address);
	
#ifdef KBE_SERVER
	if (!pEP->setBufferSize(SO_RCVBUF, RECV_BUFFER_SIZE))
	{
		WARNING_MSG("NetworkInterface::recreateListeningSocket(%s): "
			"Operating with a receive buffer of only %d bytes (instead of %d)\n",
			pEndPointName, pEP->getBufferSize(SO_RCVBUF), RECV_BUFFER_SIZE);
	}
#endif

	if(pEP->listen(5) == -1)
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket(%s): "
			"listen to %s (%s)\n",
			pEndPointName, address.c_str(), kbe_strerror());

		pEP->close();
		pEP->detach();
		return false;
	}
	
	INFO_MSG("NetworkInterface::recreateListeningSocket(%s): address %s\n", pEndPointName, address.c_str());
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
	INFO_MSG("NetworkInterface::handleTimeout: EXTERNAL(%s), INTERNAL(%s).\n", 
		extaddr().c_str(), intaddr().c_str());
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
bool NetworkInterface::registerChannel(Channel* pChannel)
{
	const Address & addr = pChannel->addr();
	KBE_ASSERT(addr.ip != 0);
	KBE_ASSERT(&pChannel->networkInterface() == this);
	ChannelMap::iterator iter = channelMap_.find(addr);
	Channel * pExisting = iter != channelMap_.end() ? iter->second : NULL;

	if(pExisting)
	{
		CRITICAL_MSG("NetworkInterface::registerChannel: channel %s is exist.\n", \
		pChannel->c_str());
		return false;
	}

	channelMap_[addr] = pChannel;
	INFO_MSG("NetworkInterface::registerChannel: new channel: %s.\n", pChannel->c_str());
	return true;
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::deregisterChannel(Channel* pChannel)
{
	const Address & addr = pChannel->addr();
	KBE_ASSERT(pChannel->endpoint() != NULL);

	if(pChannelDeregisterHandler_)
	{
		pChannelDeregisterHandler_->onChannelDeregister(pChannel);
	}

	INFO_MSG("NetworkInterface::deregisterChannel: del channel: %s\n", pChannel->c_str());

	if (!channelMap_.erase(addr))
	{
		CRITICAL_MSG( "NetworkInterface::deregisterChannel: "
				"Channel not found %s!\n",
			pChannel->c_str() );
		return false;
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
		ERROR_MSG( "NetworkInterface::onChannelTimeOut: "
					"Channel %s timed out but no handler is registered.\n",
				pChannel->c_str() );
	}
}

//-------------------------------------------------------------------------------------
Reason NetworkInterface::send(Bundle & bundle, Channel * pChannel)
{
	const Bundle::Packets& pakcets = bundle.packets();
	Bundle::Packets::const_iterator iter = pakcets.begin();
	for (; iter != pakcets.end(); iter++)
	{
		this->sendPacket((*iter), pChannel);
	}
	
	bundle.onSendComplete();
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
Reason NetworkInterface::sendPacket(Packet * pPacket, Channel * pChannel)
{
	PacketFilterPtr pFilter = pChannel ? pChannel->pFilter() : NULL;
	this->onPacketOut(*pPacket);
	
	if (pFilter)
	{
		pFilter->send(*this, pChannel, pPacket);
	}
	else
	{
		this->basicSendWithRetries(pChannel, pPacket);
	}
	
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
Reason NetworkInterface::basicSendWithRetries(Channel * pChannel, Packet * pPacket)
{
	// 尝试发送的次数
	int retries = 0;
	Reason reason;

	while(true)
	{
		retries++;

		reason = this->basicSendSingleTry(pChannel, pPacket);

		if (reason == REASON_SUCCESS)
			return reason;

		// 如果发送出现错误那么我们可以继续尝试一次， 超过3次退出
		if (reason == REASON_NO_SUCH_PORT && retries <= 3)
		{
			continue;
		}

		// 如果系统发送缓冲已经满了，则我们等待10ms
		if (reason == REASON_RESOURCE_UNAVAILABLE && retries <= 3)
		{
			fd_set	fds;
			struct timeval tv = { 0, 10000 };
			FD_ZERO( &fds );
			FD_SET(*pChannel->endpoint(), &fds);

			WARNING_MSG( "NetworkInterface::basicSendWithRetries: "
				"Transmit queue full, waiting for space... (%d)\n",
				retries );

			select(*pChannel->endpoint() + 1, NULL, &fds, NULL, &tv);
			continue;
		}

		// 其他错误退出尝试
		break;
	}

	return reason;
}

//-------------------------------------------------------------------------------------
Reason NetworkInterface::basicSendSingleTry(Channel * pChannel, Packet * pPacket)
{
	EndPoint * endpoint = pChannel->endpoint();
	int len = endpoint->send(pPacket->data(), pPacket->totalSize());

	if (len == (int)pPacket->totalSize())
	{
		return REASON_SUCCESS;
	}
	else
	{
		return NetworkInterface::getSendErrorReason(endpoint, len, pPacket->totalSize());
	}
}

//-------------------------------------------------------------------------------------
Reason NetworkInterface::getSendErrorReason(const EndPoint * endpoint, 
											int retSendSize, int packetTotalSize)
{
	int err;
	Reason reason;

	#ifdef unix
		err = errno;

		switch (err)
		{
			case ECONNREFUSED:	reason = REASON_NO_SUCH_PORT; break;
			case EAGAIN:		reason = REASON_RESOURCE_UNAVAILABLE; break;
			case ENOBUFS:		reason = REASON_TRANSMIT_QUEUE_FULL; break;
			default:			reason = REASON_GENERAL_NETWORK; break;
		}
	#else
		err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK || err == WSAEINTR)
		{
			reason = REASON_RESOURCE_UNAVAILABLE;
		}
		else
		{
			reason = REASON_GENERAL_NETWORK;
		}
	#endif

	if (retSendSize == -1)
	{
		if (reason != REASON_NO_SUCH_PORT)
		{
			ERROR_MSG( "NetworkInterface::getSendErrorReason( %s ): "
					"Could not send packet: %s\n",
				endpoint->addr().c_str(), kbe_strerror( err ) );
		}
	}
	else
	{
		WARNING_MSG( "NetworkInterface::getSendErrorReason( %s ): "
			"Packet length %d does not match sent length %d (err = %s)\n",
			endpoint->addr().c_str(), packetTotalSize, retSendSize, kbe_strerror( err ) );
	}

	return reason;
}

//-------------------------------------------------------------------------------------
void NetworkInterface::onPacketIn(const Packet & packet)
{
}

//-------------------------------------------------------------------------------------
void NetworkInterface::onPacketOut(const Packet & packet)
{
}

//-------------------------------------------------------------------------------------
void NetworkInterface::handleChannels(KBEngine::Mercury::MessageHandlers* pMsgHandlers)
{
	ChannelMap::iterator iter = channelMap_.begin();
	for(; iter != channelMap_.end(); iter++)
	{
		Mercury::Channel* pChannel = iter->second;
		pChannel->handleMessage(pMsgHandlers);
	}
}
//-------------------------------------------------------------------------------------
}
}