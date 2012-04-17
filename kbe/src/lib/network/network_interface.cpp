#include "network_interface.hpp"
#ifndef CODE_INLINE
#include "network_interface.ipp"
#endif

namespace KBEngine { 
namespace Mercury
{
const int NetworkInterface::RECV_BUFFER_SIZE = 16 * 1024 * 1024; // 16MB
const char * NetworkInterface::USE_KBEMACHINED = "kbemachined";

//-------------------------------------------------------------------------------------
NetworkInterface::NetworkInterface(Mercury::EventDispatcher * pMainDispatcher,
		NetworkInterfaceType networkInterfaceType,
		uint16 listeningPort, const char * listeningInterface) :
	socket_(),
	address_(Address::NONE),
	channelMap_(),
	isExternal_(networkInterfaceType == NETWORK_INTERFACE_EXTERNAL),
	isVerbose_(true),
	pDispatcher_(new EventDispatcher),
	pMainDispatcher_(NULL),
	pExtensionData_(NULL),
	pPacketReceiver_(NULL)
{
	pPacketReceiver_ = new PacketReceiver(socket_, *this);
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
}

//-------------------------------------------------------------------------------------
void NetworkInterface::attach(EventDispatcher & mainDispatcher)
{
	KBE_ASSERT(pMainDispatcher_ == NULL);
	pMainDispatcher_ = &mainDispatcher;
	mainDispatcher.attach(this->dispatcher());
}

//-------------------------------------------------------------------------------------
void NetworkInterface::detach()
{
	if (pMainDispatcher_ != NULL)
	{
		pMainDispatcher_->detach(this->dispatcher());
		pMainDispatcher_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
void NetworkInterface::closeSocket()
{
	if (socket_.good())
	{
		this->dispatcher().deregisterFileDescriptor(socket_);
		socket_.close();
		socket_.detach();
	}
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::recreateListeningSocket(uint16 listeningPort,
	const char * listeningInterface)
{
	this->closeSocket();

	address_.ip = 0;
	address_.port = 0;

	socket_.socket(SOCK_STREAM);
	if (!socket_.good())
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket: couldn't create a socket\n");
		return false;
	}

	this->dispatcher().registerFileDescriptor(socket_, pPacketReceiver_);
	
	char ifname[IFNAMSIZ];
	u_int32_t ifaddr = INADDR_ANY;
	bool listeningInterfaceEmpty =
		(listeningInterface == NULL || listeningInterface[0] == 0);

	// Query bwmachined over the local interface (dev: lo) for what it
	// believes the internal interface is.
	if (listeningInterface &&
		(strcmp( listeningInterface, USE_KBEMACHINED ) == 0))
	{
		INFO_MSG( "NetworkInterface::recreateListeningSocket: "
				"Querying KBEMachined for interface\n" );
		
		// 没有实现
	}
	else if (socket_.findIndicatedInterface( listeningInterface, ifname ) == 0)
	{
		INFO_MSG( "NetworkInterface::recreateListeningSocket: "
				"Creating on interface '%s' (= %s)\n",
			listeningInterface, ifname );
		if (socket_.getInterfaceAddress( ifname, ifaddr ) != 0)
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
	
	if (socket_.bind(listeningPort, ifaddr) != 0)
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket: "
				"Couldn't bind the socket to %s (%s)\n",
			address_.c_str(), strerror(errno));
		
		socket_.close();
		socket_.detach();
		return false;
	}
	
	socket_.getlocaladdress( (u_int16_t*)&address_.port,
		(u_int32_t*)&address_.ip );

	if (address_.ip == 0)
	{
		if (socket_.findDefaultInterface( ifname ) != 0 ||
			socket_.getInterfaceAddress( ifname,
				(u_int32_t&)address_.ip ) != 0)
		{
			ERROR_MSG( "NetworkInterface::recreateListeningSocket: "
				"Couldn't determine ip addr of default interface\n" );

			socket_.close();
			socket_.detach();
			return false;
		}

		INFO_MSG( "NetworkInterface::recreateListeningSocket: "
				"bound to all interfaces with default route "
				"interface on %s ( %s )\n",
			ifname, address_.c_str() );
	}
	
	socket_.setnonblocking(true);
	socket_.setnodelay(true);
	socket_.address(address_);
	
#ifdef KBE_SERVER
	if (!socket_.setBufferSize(SO_RCVBUF, RECV_BUFFER_SIZE))
	{
		WARNING_MSG("NetworkInterface::recreateListeningSocket: "
			"Operating with a receive buffer of only %d bytes (instead of %d)\n",
			socket_.getBufferSize(SO_RCVBUF), RECV_BUFFER_SIZE);
	}
#endif

	if(socket_.listen(5) == -1)
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket: "
			"listen to %s (%s)\n",
			address_.c_str(), strerror(errno));

		socket_.close();
		socket_.detach();
		return false;
	}
	
	INFO_MSG("NetworkInterface::recreateListeningSocket: address %s\n", address_.c_str());
	return true;
}

//-------------------------------------------------------------------------------------
void NetworkInterface::delayedSend(Channel & channel)
{
	//pDelayedChannels_->add(channel);
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
	{
		return NULL;
	}

	ChannelMap::iterator iter = channelMap_.find(addr);
	Channel * pChannel = iter != channelMap_.end() ? iter->second : NULL;

	return pChannel;
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::registerChannel(Channel & channel)
{
	KBE_ASSERT( channel.addr() != Address::NONE );
	KBE_ASSERT( &channel.networkInterface() == this );

	ChannelMap::iterator iter = channelMap_.find(channel.addr());
	Channel * pExisting = iter != channelMap_.end() ? iter->second : NULL;

	if(!pExisting)
	{
		CRITICAL_MSG("NetworkInterface::registerChannel: channel %s is exist.\n", channel.addr().c_str());
		return false;
	}

	channelMap_[channel.addr()] = &channel;
	return true;
}

//-------------------------------------------------------------------------------------
bool NetworkInterface::deregisterChannel(Channel & channel)
{
	const Address & addr = channel.addr();
	KBE_ASSERT( addr != Address::NONE );

	if (!channelMap_.erase( addr ))
	{
		CRITICAL_MSG( "NetworkInterface::deregisterChannel: "
				"Channel not found %s!\n",
			addr.c_str() );
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
	ERROR_MSG("NetworkInterface::onChannelTimeOut: Channel %s timed out.\n",
			pChannel->c_str());
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