#include "machine.hpp"
#include "machine_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Machine);

//-------------------------------------------------------------------------------------
Machine::Machine(Mercury::EventDispatcher& dispatcher, 
				 Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType):
	ServerApp(dispatcher, ninterface, componentType),
	broadcastAddr_(0),
	ep_(),
	epBroadcast_(),
	epLocal_(),
	pEPPacketReceiver_(NULL),
	pEBPacketReceiver_(NULL),
	pEPLocalPacketReceiver_(NULL)

{
}

//-------------------------------------------------------------------------------------
Machine::~Machine()
{
	SAFE_RELEASE(pEPPacketReceiver_);
	SAFE_RELEASE(pEBPacketReceiver_);
	SAFE_RELEASE(pEPLocalPacketReceiver_);
}

//-------------------------------------------------------------------------------------
void Machine::onBroadcastInterface(int32 uid, std::string& username,
								   int8 componentType, int32 componentID, 
								   uint32 addr, uint16 port)
{
	INFO_MSG("Machine::onBroadcastInterface: uid:%d, username:%s, componentType:%s, "
			"componentID:%d, addr:%u, port:%u\n", 
		uid, username.c_str(), COMPONENT_NAME[componentType], componentID, addr, port);

	Componentbridge::getComponents().addComponent(uid, username.c_str(), 
		(KBEngine::COMPONENT_TYPE)componentType, componentID, NULL);
}

//-------------------------------------------------------------------------------------
void Machine::onFindInterfaceAddr(int32 uid, std::string& username, int8 componentType, int8 findComponentType)
{
	INFO_MSG("Machine::onFindInterfaceAddr: uid:%d, username:%s, componentType:%d, "
			"find:%s\n", 
		uid, username.c_str(), COMPONENT_NAME[componentType],  COMPONENT_NAME[findComponentType]);
}

//-------------------------------------------------------------------------------------
bool Machine::findBroadcastInterface()
{

	std::map<u_int32_t, std::string> interfaces;
	Mercury::EndPoint epListen;
	struct timeval tv;
	fd_set fds;
	char streamBuf[4096];

	// Initialise the endpoint
	epListen.socket(SOCK_DGRAM);
	if (!epListen.good() ||
		 epListen.bind(htons(KBE_PORT_BROADCAST_DISCOVERY)) == -1)
	{
		ERROR_MSG("Couldn't bind broadcast listener socket to port %d\n",
				KBE_PORT_BROADCAST_DISCOVERY);
		return false;
	}

	epListen.setbroadcast(true);

	// Perform a discovery of all network interfaces on this host
	if (!epListen.getInterfaces(interfaces))
	{
		ERROR_MSG("Failed to discover network interfaces\n");
		return false;
	}


	char data[1] = {1};
	
	if (epListen.sendto(data, 1, htons( KBE_PORT_BROADCAST_DISCOVERY ), Mercury::BROADCAST ) != 1)
	{
		ERROR_MSG("Failed to send broadcast discovery message. error:%s\n", kbe_strerror());
		return false;
	}

	tv.tv_sec = 1;
	tv.tv_usec = 0;

	// Listen for the message we just sent to come back to ourselves.
	while (1)
	{
		FD_ZERO( &fds );
		FD_SET( (int)epListen, &fds );
		int selgot = select( epListen+1, &fds, NULL, NULL, &tv );
		if (selgot == 0)
		{
			ERROR_MSG("Timed out before receiving any broadcast discovery responses\n");
			return false;
		}
		else if (selgot == -1)
		{
			ERROR_MSG("Broadcast discovery select error. %s.\n",
					kbe_strerror());
			return false;
		}
		else
		{
			sockaddr_in	sin;

			// Read packet into buffer
			int len = epListen.recvfrom( &streamBuf, sizeof( streamBuf ), sin );
			if (len == -1)
			{
				ERROR_MSG("Broadcast discovery recvfrom error. %s.\n",
						kbe_strerror());
				continue;
			}

			INFO_MSG("Broadcast discovery receipt from %s.\n",
					inet_ntoa((struct in_addr&)sin.sin_addr.s_addr) );

			// check messages received against the list of our interfaces
			std::map< u_int32_t, std::string >::iterator iter;

			iter = interfaces.find( (u_int32_t &)sin.sin_addr.s_addr );
			if (iter != interfaces.end())
			{
				INFO_MSG("Confirmed %s (%s) as default broadcast route interface.\n",
					inet_ntoa((struct in_addr&)sin.sin_addr.s_addr),
					iter->second.c_str() );
				broadcastAddr_ = sin.sin_addr.s_addr;
				break;
			}

			ERROR_MSG("Broadcast discovery %s not a valid interface.\n",
					inet_ntoa((struct in_addr&)sin.sin_addr.s_addr) );
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Machine::initNetwork()
{
	epBroadcast_.socket(SOCK_DGRAM);
	ep_.socket(SOCK_DGRAM);
	epLocal_.socket(SOCK_DGRAM); 

	Mercury::Address address;
	address.ip = 0;
	address.port = 0;

	if (broadcastAddr_ == 0 && !this->findBroadcastInterface())
	{
		ERROR_MSG("Failed to determine default broadcast interface. "
				"Make sure that your broadcast route is set correctly. "
				"e.g. /sbin/ip route add broadcast 255.255.255.255 dev eth0\n" );
		return false;
	}

	if (!ep_.good() ||
		 ep_.bind(htons(KBE_MACHINE_BRAODCAST_PORT), broadcastAddr_) == -1)
	{
		ERROR_MSG("Failed to bind socket to '%s'. %s.\n",
							inet_ntoa((struct in_addr &)broadcastAddr_),
							kbe_strerror());
		return false;
	}

	address.ip = broadcastAddr_;
	address.port = htons(KBE_MACHINE_BRAODCAST_PORT);
	ep_.setbroadcast( true );
	ep_.setnonblocking(true);
	ep_.addr(address);
	pEPPacketReceiver_ = new Mercury::UDPPacketReceiver(ep_, this->getNetworkInterface());

	if(!this->getMainDispatcher().registerFileDescriptor(ep_, pEPPacketReceiver_))
	{
		ERROR_MSG("registerFileDescriptor ep is failed!\n");
		return false;
	}

	if (!epBroadcast_.good() ||
		epBroadcast_.bind(htons(KBE_MACHINE_BRAODCAST_PORT), Mercury::BROADCAST) == -1)
	{
		ERROR_MSG("Failed to bind socket to '%s'. %s.\n",
							inet_ntoa((struct in_addr &)Mercury::BROADCAST),
							kbe_strerror());
//		return false;
	}
	else
	{
		address.ip = Mercury::BROADCAST;
		address.port = htons(KBE_MACHINE_BRAODCAST_PORT);
		epBroadcast_.setnonblocking(true);
		epBroadcast_.addr(address);
		pEBPacketReceiver_ = new Mercury::UDPPacketReceiver(epBroadcast_, this->getNetworkInterface());
	
		if(!this->getMainDispatcher().registerFileDescriptor(epBroadcast_, pEBPacketReceiver_))
		{
			ERROR_MSG("registerFileDescriptor epBroadcast is failed!\n");
			return false;
		}

	}

	if (!epLocal_.good() ||
		 epLocal_.bind(htons(KBE_MACHINE_BRAODCAST_PORT), Mercury::LOCALHOST) == -1)
	{
		ERROR_MSG("Failed to bind socket to (lo). %s.\n",
							kbe_strerror() );
		return false;
	}

	address.ip = Mercury::LOCALHOST;
	address.port = htons(KBE_MACHINE_BRAODCAST_PORT);
	epLocal_.setnonblocking(true);
	epLocal_.addr(address);
	pEPLocalPacketReceiver_ = new Mercury::UDPPacketReceiver(epLocal_, this->getNetworkInterface());

	if(!this->getMainDispatcher().registerFileDescriptor(epLocal_, pEPLocalPacketReceiver_))
	{
		ERROR_MSG("registerFileDescriptor epLocal is failed!\n");
		return false;
	}

	INFO_MSG("bind broadcast successfully! addr:%s\n", ep_.addr().c_str());
	return true;
}

//-------------------------------------------------------------------------------------
bool Machine::run()
{
	bool ret = true;

	while(!this->getMainDispatcher().isBreakProcessing())
	{
		this->getMainDispatcher().processOnce(false);
		getNetworkInterface().handleChannels(&MachineInterface::messageHandlers);
		KBEngine::sleep(100);
	};

	return ret;
}

//-------------------------------------------------------------------------------------
void Machine::handleTimeout(TimerHandle handle, void * arg)
{
}

//-------------------------------------------------------------------------------------
bool Machine::initializeBegin()
{
	if(!initNetwork())
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool Machine::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Machine::initializeEnd()
{
	return true;
}

//-------------------------------------------------------------------------------------
void Machine::finalise()
{
}

//-------------------------------------------------------------------------------------

}
