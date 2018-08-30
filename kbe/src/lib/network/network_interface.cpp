// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "network_interface.h"
#ifndef CODE_INLINE
#include "network_interface.inl"
#endif

#include "network/address.h"
#include "network/event_dispatcher.h"
#include "network/packet_receiver.h"
#include "network/listener_udp_receiver.h"
#include "network/listener_tcp_receiver.h"
#include "network/channel.h"
#include "network/packet.h"
#include "network/delayed_channels.h"
#include "network/interfaces.h"
#include "network/message_handler.h"

namespace KBEngine { 
namespace Network
{

//-------------------------------------------------------------------------------------
NetworkInterface::NetworkInterface(Network::EventDispatcher * pDispatcher,
		int32 extlisteningTcpPort_min, int32 extlisteningTcpPort_max, int32 extlisteningUdpPort_min, int32 extlisteningUdpPort_max, const char * extlisteningInterface,
		uint32 extrbuffer, uint32 extwbuffer,
		int32 intlisteningPort, const char * intlisteningInterface,
		uint32 intrbuffer, uint32 intwbuffer):
	extTcpEndpoint_(),
	extUdpEndpoint_(),
	intTcpEndpoint_(),
	channelMap_(),
	pDispatcher_(pDispatcher),
	pExtListenerReceiver_(NULL),
	pExtUdpListenerReceiver_(NULL),
	pIntListenerReceiver_(NULL),
	pDelayedChannels_(new DelayedChannels()),
	pChannelTimeOutHandler_(NULL),
	pChannelDeregisterHandler_(NULL),
	numExtChannels_(0)
{
	if(extlisteningTcpPort_min != -1)
	{
		pExtListenerReceiver_ = new ListenerTcpReceiver(extTcpEndpoint_, Channel::EXTERNAL, *this);

		this->initialize("EXTERNAL-TCP", htons(extlisteningTcpPort_min), htons(extlisteningTcpPort_max),
			extlisteningInterface, &extTcpEndpoint_, pExtListenerReceiver_, extrbuffer, extwbuffer);

		// 如果配置了对外端口范围， 如果范围过小这里extEndpoint_可能没有端口可用了
		if(extlisteningTcpPort_min != -1)
		{
			KBE_ASSERT(extTcpEndpoint_.good() && "Channel::EXTERNAL-TCP: no available port, "
				"please check for kbengine[_defs].xml!\n");
		}
	}

	if (extlisteningUdpPort_min != -1)
	{
		pExtUdpListenerReceiver_ = new ListenerUdpReceiver(extUdpEndpoint_, Channel::EXTERNAL, *this);

		this->initialize("EXTERNAL-UDP", htons(extlisteningUdpPort_min), htons(extlisteningUdpPort_max),
			extlisteningInterface, &extUdpEndpoint_, pExtUdpListenerReceiver_, extrbuffer, extwbuffer);

		// 如果配置了对外端口范围， 如果范围过小这里extEndpoint_可能没有端口可用了
		if (extlisteningUdpPort_min != -1)
		{
			KBE_ASSERT(extTcpEndpoint_.good() && "Channel::EXTERNAL-UDP: no available udp-port, "
				"please check for kbengine[_defs].xml!\n");
		}
	}

	if(intlisteningPort != -1)
	{
		pIntListenerReceiver_ = new ListenerTcpReceiver(intTcpEndpoint_, Channel::INTERNAL, *this);

		this->initialize("INTERNAL-TCP", intlisteningPort, intlisteningPort,
			intlisteningInterface, &intTcpEndpoint_, pIntListenerReceiver_, intrbuffer, intwbuffer);
	}

	KBE_ASSERT(good() && "NetworkInterface::NetworkInterface: no available port, "
		"please check for kbengine[_defs].xml!\n");

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
	if (extTcpEndpoint_.good())
	{
		this->dispatcher().deregisterReadFileDescriptor(extTcpEndpoint_);
		extTcpEndpoint_.close();
	}

	if (extUdpEndpoint_.good())
	{
		this->dispatcher().deregisterReadFileDescriptor(extUdpEndpoint_);
		extUdpEndpoint_.close();
	}

	if (intTcpEndpoint_.good())
	{
		this->dispatcher().deregisterReadFileDescriptor(intTcpEndpoint_);
		intTcpEndpoint_.close();
	}
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::initialize(const char* pEndPointName, uint16 listeningPort_min, uint16 listeningPort_max,
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

	bool isTCP = strstr(pEndPointName, "-TCP") != NULL;

	if(isTCP)
		pEP->socket(SOCK_STREAM);
	else
		pEP->socket(SOCK_DGRAM);

	if (!pEP->good())
	{
		ERROR_MSG(fmt::format("NetworkInterface::initialize({}): couldn't create a socket\n",
			pEndPointName));

		return false;
	}
	
	if (listeningPort_min > 0 && listeningPort_min == listeningPort_max)
		pEP->setreuseaddr(true);
	
	this->dispatcher().registerReadFileDescriptor(*pEP, pLR);
	
	u_int32_t ifIPAddr = INADDR_ANY;

	bool listeningInterfaceEmpty =
		(listeningInterface == NULL || listeningInterface[0] == 0);

	// 查找指定接口名 NIP、MAC、IP是否可用
	if(pEP->findIndicatedInterface(listeningInterface, ifIPAddr) == 0)
	{
		char szIp[MAX_IP] = {0};
		Address::ip2string(ifIPAddr, szIp);

		INFO_MSG(fmt::format("NetworkInterface::initialize({}): Creating on interface '{}' (= {})\n",
			pEndPointName, listeningInterface, szIp));
	}

	// 如果不为空又找不到那么警告用户错误的设置，同时我们采用默认的方式(绑定到INADDR_ANY)
	else if (!listeningInterfaceEmpty)
	{
		WARNING_MSG(fmt::format("NetworkInterface::initialize({}): Couldn't parse interface spec '{}' so using all interfaces\n",
			pEndPointName, listeningInterface));
	}
	
	// 尝试绑定到端口，如果被占用向后递增
	bool foundport = false;
	uint32 listeningPort = listeningPort_min;
	if(listeningPort_min != listeningPort_max)
	{
		for(int lpIdx=ntohs(listeningPort_min); lpIdx<=ntohs(listeningPort_max); ++lpIdx)
		{
			listeningPort = htons(lpIdx);
			if (pEP->bind(listeningPort, ifIPAddr) != 0)
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
		if (pEP->bind(listeningPort, ifIPAddr) == 0)
		{
			foundport = true;
		}
	}

	// 如果无法绑定到合适的端口那么报错返回，进程将退出
	if(!foundport)
	{
		ERROR_MSG(fmt::format("NetworkInterface::initialize({}): Couldn't bind the socket to {}:{} ({})\n",
			pEndPointName, inet_ntoa((struct in_addr&)ifIPAddr), ntohs(listeningPort), kbe_strerror()));
		
		pEP->close();
		return false;
	}

	// 获得当前绑定的地址，如果是INADDR_ANY这里获得的IP是0
	pEP->getlocaladdress( (u_int16_t*)&address.port,
		(u_int32_t*)&address.ip );

	if (0 == address.ip)
	{
		u_int32_t addr;
		if(0 == pEP->getDefaultInterfaceAddress(addr))
		{
			address.ip = addr;

			char szIp[MAX_IP] = {0};
			Address::ip2string(address.ip, szIp);
			INFO_MSG(fmt::format("NetworkInterface::initialize({}): bound to all interfaces with default route interface on {} ( {} )\n",
				pEndPointName, szIp, address.c_str()));
		}
		else
		{
			ERROR_MSG(fmt::format("NetworkInterface::initialize({}): Couldn't determine ip addr of default interface\n", pEndPointName));

			pEP->close();
			return false;
		}
	}
	
	pEP->setnonblocking(true);
	pEP->setnodelay(true);
	pEP->addr(address);
	
	if(rbuffer > 0)
	{
		if (!pEP->setBufferSize(SO_RCVBUF, rbuffer))
		{
			WARNING_MSG(fmt::format("NetworkInterface::initialize({}): Operating with a receive buffer of only {} bytes (instead of {})\n",
				pEndPointName, pEP->getBufferSize(SO_RCVBUF), rbuffer));
		}
	}
	if(wbuffer > 0)
	{
		if (!pEP->setBufferSize(SO_SNDBUF, wbuffer))
		{
			WARNING_MSG(fmt::format("NetworkInterface::initialize({}): Operating with a send buffer of only {} bytes (instead of {})\n",
				pEndPointName, pEP->getBufferSize(SO_SNDBUF), wbuffer));
		}
	}


	int backlog = Network::g_SOMAXCONN;
	if (backlog < 5)
		backlog = 5;

	if (isTCP)
	{
		if (pEP->listen(backlog) == -1)
		{
			ERROR_MSG(fmt::format("NetworkInterface::initialize({}): listen to {} ({})\n",
				pEndPointName, address.c_str(), kbe_strerror()));

			pEP->close();
			return false;
		}
	}

	INFO_MSG(fmt::format("NetworkInterface::initialize({}): address {}, SOMAXCONN={}.\n", 
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
		extTcpAddr().c_str(), intTcpAddr().c_str()));
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
		Network::Channel::reclaimPoolObject(pChannel);
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
void NetworkInterface::processChannels(KBEngine::Network::MessageHandlers* pMsgHandlers)
{
	ChannelMap::iterator iter = channelMap_.begin();
	for(; iter != channelMap_.end(); )
	{
		Network::Channel* pChannel = iter->second;

		if(pChannel->isDestroyed())
		{
			++iter;
		}
		else if(pChannel->condemn() > 0)
		{
			++iter;

			if (pChannel->condemn() == Network::Channel::FLAG_CONDEMN_AND_WAIT_DESTROY && pChannel->sending())
			{
				pChannel->updateTick(pMsgHandlers);
			}
			else
			{
				deregisterChannel(pChannel);
				pChannel->destroy();
				Network::Channel::reclaimPoolObject(pChannel);
			}
		}
		else
		{
			pChannel->updateTick(pMsgHandlers);
			++iter;
		}
	}
}
//-------------------------------------------------------------------------------------
}
}
