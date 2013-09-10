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
		uint32 extrbuffer, uint32 extwbuffer,
		int32 intlisteningPort, const char * intlisteningInterface,
		uint32 intrbuffer, uint32 intwbuffer):
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
			WARNING_MSG(boost::format("NetworkInterface::~NetworkInterface: "
					"Channel to %1% is still registered\n") %
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
	
	pDelayedChannels_->init(this->mainDispatcher(), this);
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
										const char * listeningInterface, EndPoint* pEP, ListenerReceiver* pLR, uint32 rbuffer, 
										uint32 wbuffer)
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
		ERROR_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): couldn't create a socket\n") %
			pEndPointName);

		return false;
	}
	
	/*
	int val = 1;
	setsockopt((*pEP), SOL_SOCKET, SO_REUSEADDR,
			(const char*)&val, sizeof(val));
	*/
	
	this->dispatcher().registerFileDescriptor(*pEP, pLR);
	
	char ifname[IFNAMSIZ];
	u_int32_t ifaddr = INADDR_ANY;
	bool listeningInterfaceEmpty =
		(listeningInterface == NULL || listeningInterface[0] == 0);

	if (listeningInterface &&
		(strcmp(listeningInterface, USE_KBEMACHINED) == 0))
	{
		INFO_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): "
				"Querying KBEMachined for interface\n") % pEndPointName);
		
		// 没有实现, 向KBEMachined查询接口
	}
	else if (pEP->findIndicatedInterface(listeningInterface, ifname) == 0)
	{
		INFO_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): "
				"Creating on interface '%2%' (= %3%)\n") %
			pEndPointName % listeningInterface % ifname );

		if (pEP->getInterfaceAddress( ifname, ifaddr ) != 0)
		{
			WARNING_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): "
				"Couldn't get addr of interface %2% so using all interfaces\n") %
				pEndPointName % ifname );
		}
	}
	else if (!listeningInterfaceEmpty)
	{
		WARNING_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): "
				"Couldn't parse interface spec '%2%' so using all interfaces\n") %
			pEndPointName % listeningInterface );
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
		ERROR_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): "
				"Couldn't bind the socket to %2%:%3% (%4%)\n") %
			pEndPointName % inet_ntoa((struct in_addr&)ifaddr) % ntohs(listeningPort) % kbe_strerror());
		
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
			ERROR_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): "
				"Couldn't determine ip addr of default interface\n") % pEndPointName );

			pEP->close();
			pEP->detach();
			return false;
		}

		INFO_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): "
				"bound to all interfaces with default route "
				"interface on %2% ( %3% )\n") %
			pEndPointName % ifname % address.c_str() );
	}
	
	pEP->setnonblocking(true);
	pEP->setnodelay(true);
	pEP->addr(address);
	
#ifdef KBE_SERVER
	if(rbuffer > 0)
	{
		if (!pEP->setBufferSize(SO_RCVBUF, rbuffer))
		{
			WARNING_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): "
				"Operating with a receive buffer of only %2% bytes (instead of %3%)\n") %
				pEndPointName % pEP->getBufferSize(SO_RCVBUF) % rbuffer);
		}
	}
	if(wbuffer > 0)
	{
		if (!pEP->setBufferSize(SO_SNDBUF, wbuffer))
		{
			WARNING_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): "
				"Operating with a send buffer of only %2% bytes (instead of %3%)\n") %
				pEndPointName % pEP->getBufferSize(SO_SNDBUF) % wbuffer);
		}
	}
#endif

	int backlog = Mercury::g_SOMAXCONN;
	if(backlog < 5)
		backlog = 5;

	if(pEP->listen(backlog) == -1)
	{
		ERROR_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): "
			"listen to %2% (%3%)\n") %
			pEndPointName % address.c_str() % kbe_strerror());

		pEP->close();
		pEP->detach();
		return false;
	}
	
	INFO_MSG(boost::format("NetworkInterface::recreateListeningSocket(%1%): address %2%, SOMAXCONN=%3%.\n") % 
		pEndPointName % address.c_str() % backlog);

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
	INFO_MSG(boost::format("NetworkInterface::handleTimeout: EXTERNAL(%1%), INTERNAL(%2%).\n") % 
		extaddr().c_str() % intaddr().c_str());
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
	for(; iter != channelMap_.end(); iter++)
	{
		if(iter->second->endpoint() && *iter->second->endpoint() == fd)
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
		CRITICAL_MSG(boost::format("NetworkInterface::registerChannel: channel %1% is exist.\n") %
		pChannel->c_str());
		return false;
	}

	channelMap_[addr] = pChannel;

	if(pChannel->isExternal())
		numExtChannels_++;

	INFO_MSG(boost::format("NetworkInterface::registerChannel: new channel: %1%.\n") % pChannel->c_str());
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
	}

	channelMap_.clear();
	numExtChannels_ = 0;

	return true;
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::deregisterChannel(Channel* pChannel)
{
	const Address & addr = pChannel->addr();
	KBE_ASSERT(pChannel->endpoint() != NULL);

	if(pChannel->isExternal())
		numExtChannels_--;

	if(pChannelDeregisterHandler_)
	{
		pChannelDeregisterHandler_->onChannelDeregister(pChannel);
	}

	INFO_MSG(boost::format("NetworkInterface::deregisterChannel: del channel: %1%\n") %
		pChannel->c_str());

	if (!channelMap_.erase(addr))
	{
		CRITICAL_MSG(boost::format("NetworkInterface::deregisterChannel: "
				"Channel not found %1%!\n") %
			pChannel->c_str() );

		return false;
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::deregisterChannel(const Address & addr)
{
	ChannelMap::iterator iter = channelMap_.find(addr);
	if(iter == channelMap_.end())
	{
		CRITICAL_MSG(boost::format("NetworkInterface::deregisterChannel: "
				"addr not found %1%!\n") %
			addr.c_str() );
		return false;
	}

	Channel* pChannel = iter->second;
	KBE_ASSERT(pChannel->endpoint() != NULL);

	if(pChannel->isExternal())
		numExtChannels_--;

	if(pChannelDeregisterHandler_)
	{
		pChannelDeregisterHandler_->onChannelDeregister(pChannel);
	}

	INFO_MSG(boost::format("NetworkInterface::deregisterChannel: del channel: %1%\n") %
		pChannel->c_str());

	if (!channelMap_.erase(addr))
	{
		CRITICAL_MSG(boost::format("NetworkInterface::deregisterChannel: "
				"Channel not found %1%!\n") %
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
		ERROR_MSG(boost::format("NetworkInterface::onChannelTimeOut: "
					"Channel %1% timed out but no handler is registered.\n") %
				pChannel->c_str() );
	}
}

//-------------------------------------------------------------------------------------
Reason NetworkInterface::send(Bundle & bundle, Channel * pChannel)
{
	Reason reason = REASON_SUCCESS;
	const Bundle::Packets& pakcets = bundle.packets();
	Bundle::Packets::const_iterator iter = pakcets.begin();
	for (; iter != pakcets.end(); iter++)
	{
		reason = this->sendPacket((*iter), pChannel);
		if(reason != REASON_SUCCESS)
			break; 
	}
	
	bundle.onSendComplete();
	return reason;
}

//-------------------------------------------------------------------------------------
Reason NetworkInterface::sendPacket(Packet * pPacket, Channel * pChannel)
{
	PacketFilterPtr pFilter = pChannel ? pChannel->pFilter() : NULL;
	this->onPacketOut(*pPacket);
	
	Reason reason = REASON_SUCCESS;
	if (pFilter)
	{
		reason = pFilter->send(*this, pChannel, pPacket);
	}
	else
	{
		reason = this->basicSendWithRetries(pChannel, pPacket);
	}
	
	return reason;
}

//-------------------------------------------------------------------------------------
Reason NetworkInterface::basicSendWithRetries(Channel * pChannel, Packet * pPacket)
{
	// 尝试发送的次数
	int retries = 0;
	Reason reason;
	
	pPacket->sentSize = 0;

	while(true)
	{
		retries++;

		reason = this->basicSendSingleTry(pChannel, pPacket);

		if (reason == REASON_SUCCESS)
			return reason;

		// 如果发送出现错误那么我们可以继续尝试一次， 外部通道超过3次退出
		// 内部通道则无限尝试
		if (reason == REASON_NO_SUCH_PORT && retries <= 3)
		{
			continue;
		}

		// 如果系统发送缓冲已经满了，则我们等待10ms
		if ((reason == REASON_RESOURCE_UNAVAILABLE || reason == REASON_GENERAL_NETWORK) 
			&& (pChannel->isInternal() || retries <= 3))
		{
			WARNING_MSG(boost::format("NetworkInterface::basicSendWithRetries: "
				"Transmit queue full, waiting for space... (%1%)\n") %
				retries );
			
			int fd = *pChannel->endpoint();
			this->pDispatcher_->processNetwork(false);
			
			// 有可能会在processNetwork处理时被强制关闭通道而造成崩溃， 所以此处需要检查一下
			if(this->findChannel(fd) == NULL)
				return REASON_CHANNEL_LOST;

			continue;
		}

		break;
	}

	// 其他错误退出尝试
	ERROR_MSG(boost::format("NetworkInterface::basicSendWithRetries: packet discarded(reason=%1%).\n") % (reasonToString(reason)));

	// 如果是外部通道， 那么此时后续包都将出错， 没有必要继续和其通讯了
	if(pChannel->isExternal())
	{
		pChannel->condemn();
	}

	return reason;
}

//-------------------------------------------------------------------------------------
Reason NetworkInterface::basicSendSingleTry(Channel * pChannel, Packet * pPacket)
{
	EndPoint * endpoint = pChannel->endpoint();
	KBE_ASSERT(pPacket->rpos() == 0);
	int len = endpoint->send(pPacket->data() + pPacket->sentSize, pPacket->totalSize() - pPacket->sentSize);
	if(len > 0)
		pPacket->sentSize += len;

	if (pPacket->sentSize == pPacket->totalSize())
	{
		return REASON_SUCCESS;
	}
	else
	{
		return NetworkInterface::getSendErrorReason(endpoint, pPacket->sentSize, pPacket->totalSize());
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
			case EPIPE:			reason = REASON_CLIENT_DISCONNECTED; break;
			case ECONNRESET:	reason = REASON_CLIENT_DISCONNECTED; break;
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
			switch (err)
			{
				case WSAECONNREFUSED:	reason = REASON_NO_SUCH_PORT; break;
				case WSAECONNRESET:	reason = REASON_CLIENT_DISCONNECTED; break;
				case WSAECONNABORTED:	reason = REASON_CLIENT_DISCONNECTED; break;
				default:reason = REASON_GENERAL_NETWORK;break;
			}
		}
	#endif

	if (retSendSize == -1)
	{
		if (reason != REASON_NO_SUCH_PORT)
		{
			ERROR_MSG(boost::format("NetworkInterface::getSendErrorReason(%1%): "
					"Could not send packet: %2%\n") %
				endpoint->addr().c_str() % kbe_strerror( err ) );
		}
	}
	else
	{
		WARNING_MSG(boost::format("NetworkInterface::getSendErrorReason(%1%): "
			"Packet length %2% does not match sent length %3% (%4%)\n") %
			endpoint->addr().c_str() % packetTotalSize % retSendSize % kbe_strerror( err ) );
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
void NetworkInterface::processAllChannelPackets(KBEngine::Mercury::MessageHandlers* pMsgHandlers)
{
	ChannelMap::iterator iter = channelMap_.begin();
	for(; iter != channelMap_.end(); )
	{
		Mercury::Channel* pChannel = iter->second;

		if(pChannel->isDestroyed() || pChannel->isCondemn())
		{
			++iter;
			deregisterChannel(pChannel);
			pChannel->destroy();
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
