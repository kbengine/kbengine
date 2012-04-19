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

namespace KBEngine { 
namespace Mercury
{
const int NetworkInterface::RECV_BUFFER_SIZE = 16 * 1024 * 1024; // 16MB
const char * NetworkInterface::USE_KBEMACHINED = "kbemachined";

//-------------------------------------------------------------------------------------
NetworkInterface::NetworkInterface(Mercury::EventDispatcher * pMainDispatcher,
		NetworkInterfaceType networkInterfaceType,
		uint16 listeningPort, const char * listeningInterface) :
	endpoint_(),
	address_(Address::NONE),
	channelMap_(),
	isExternal_(networkInterfaceType == NETWORK_INTERFACE_EXTERNAL),
	pDispatcher_(new EventDispatcher),
	pMainDispatcher_(NULL),
	pExtensionData_(NULL),
	pListenerReceiver_(NULL),
	pDelayedChannels_(new DelayedChannels()),
	pChannelTimeOutHandler_(NULL)
{
	pListenerReceiver_ = new ListenerReceiver(endpoint_, *this);
	this->recreateListeningSocket(listeningPort, listeningInterface);

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

	delete pDispatcher_;
	pDispatcher_ = NULL;
	
	delete pDelayedChannels_;
	pDelayedChannels_ = NULL;
	
	delete pListenerReceiver_;
	pListenerReceiver_ = NULL;
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
		pDelayedChannels_->fini( this->mainDispatcher() );
		pMainDispatcher_->detach(this->dispatcher());
		pMainDispatcher_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
void NetworkInterface::closeSocket()
{
	if (endpoint_.good())
	{
		this->dispatcher().deregisterFileDescriptor(endpoint_);
		endpoint_.close();
		endpoint_.detach();
	}
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::recreateListeningSocket(uint16 listeningPort,
	const char * listeningInterface)
{
	this->closeSocket();

	address_.ip = 0;
	address_.port = 0;

	endpoint_.socket(SOCK_STREAM);
	if (!endpoint_.good())
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket: couldn't create a socket\n");
		return false;
	}

	this->dispatcher().registerFileDescriptor(endpoint_, pListenerReceiver_);
	
	char ifname[IFNAMSIZ];
	u_int32_t ifaddr = INADDR_ANY;
	bool listeningInterfaceEmpty =
		(listeningInterface == NULL || listeningInterface[0] == 0);

	// Query kbemachined over the local interface (dev: lo) for what it
	// believes the internal interface is.
	if (listeningInterface &&
		(strcmp( listeningInterface, USE_KBEMACHINED ) == 0))
	{
		INFO_MSG( "NetworkInterface::recreateListeningSocket: "
				"Querying KBEMachined for interface\n" );
		
		// 没有实现, 向KBEMachined查询接口
	}
	else if (endpoint_.findIndicatedInterface( listeningInterface, ifname ) == 0)
	{
		INFO_MSG( "NetworkInterface::recreateListeningSocket: "
				"Creating on interface '%s' (= %s)\n",
			listeningInterface, ifname );
		if (endpoint_.getInterfaceAddress( ifname, ifaddr ) != 0)
		{
			WARNING_MSG( "NetworkInterface::recreateListeningSocket: "
				"Couldn't get addr of interface %s so using all interfaces\n",
				ifname );
		}
	}
	else if (!listeningInterfaceEmpty)
	{
		WARNING_MSG( "NetworkInterface::recreateListeningSocket: "
				"Couldn't parse interface spec '%s' so using all interfaces\n",
			listeningInterface );
	}
	
	if (endpoint_.bind(listeningPort, ifaddr) != 0)
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket: "
				"Couldn't bind the socket to %s (%s)\n",
			address_.c_str(), strerror(errno));
		
		endpoint_.close();
		endpoint_.detach();
		return false;
	}
	
	endpoint_.getlocaladdress( (u_int16_t*)&address_.port,
		(u_int32_t*)&address_.ip );

	if (address_.ip == 0)
	{
		if (endpoint_.findDefaultInterface( ifname ) != 0 ||
			endpoint_.getInterfaceAddress( ifname,
				(u_int32_t&)address_.ip ) != 0)
		{
			ERROR_MSG( "NetworkInterface::recreateListeningSocket: "
				"Couldn't determine ip addr of default interface\n" );

			endpoint_.close();
			endpoint_.detach();
			return false;
		}

		INFO_MSG( "NetworkInterface::recreateListeningSocket: "
				"bound to all interfaces with default route "
				"interface on %s ( %s )\n",
			ifname, address_.c_str() );
	}
	
	endpoint_.setnonblocking(true);
	endpoint_.setnodelay(true);
	endpoint_.addr(address_);
	
#ifdef KBE_SERVER
	if (!endpoint_.setBufferSize(SO_RCVBUF, RECV_BUFFER_SIZE))
	{
		WARNING_MSG("NetworkInterface::recreateListeningSocket: "
			"Operating with a receive buffer of only %d bytes (instead of %d)\n",
			endpoint_.getBufferSize(SO_RCVBUF), RECV_BUFFER_SIZE);
	}
#endif

	if(endpoint_.listen(5) == -1)
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket: "
			"listen to %s (%s)\n",
			address_.c_str(), strerror(errno));

		endpoint_.close();
		endpoint_.detach();
		return false;
	}
	
	INFO_MSG("NetworkInterface::recreateListeningSocket: address %s\n", address_.c_str());
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
	INFO_MSG("NetworkInterface::handleTimeout: address %s\n", address_.c_str());
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
bool NetworkInterface::registerChannel(Channel* channel)
{
	const Address & addr = channel->addr();
	KBE_ASSERT(addr.ip != 0);
	KBE_ASSERT(&channel->networkInterface() == this);
	ChannelMap::iterator iter = channelMap_.find(addr);
	Channel * pExisting = iter != channelMap_.end() ? iter->second : NULL;

	if(pExisting)
	{
		CRITICAL_MSG("NetworkInterface::registerChannel: channel %s is exist.\n", \
		channel->addr().c_str());
		return false;
	}

	channelMap_[addr] = channel;
	INFO_MSG("NetworkInterface::registerChannel: new channel: %s.\n", channel->addr().c_str());
	return true;
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::deregisterChannel(Channel* channel)
{
	const Address & addr = channel->addr();
	KBE_ASSERT(channel->endpoint() != NULL);

	if (!channelMap_.erase(addr))
	{
		CRITICAL_MSG( "NetworkInterface::deregisterChannel: "
				"Channel not found %s!\n",
			addr.c_str() );
		return false;
	}
	
	INFO_MSG("NetworkInterface::deregisterChannel: del channel: %s\n", addr.c_str());
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
		pChannelTimeOutHandler_->onTimeOut( pChannel );
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
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
Reason NetworkInterface::sendPacket(Packet * pPacket, Channel * pChannel)
{
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
}
}