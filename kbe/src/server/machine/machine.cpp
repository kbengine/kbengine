#include "machine.hpp"
#include "machine_interface.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Machine);

//-------------------------------------------------------------------------------------
Machine::Machine(Mercury::EventDispatcher& dispatcher, Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType):
	ServerApp(dispatcher, ninterface, componentType),
	Mercury::UDPPacketReceiver(epBroadcast_, ninterface),
	broadcastAddr_(INADDR_ANY),
	epBroadcast_()
{
}

//-------------------------------------------------------------------------------------
Machine::~Machine()
{
}

//-------------------------------------------------------------------------------------
bool Machine::initNetwork()
{
	epBroadcast_.socket(SOCK_DGRAM);
	this->getMainDispatcher().registerFileDescriptor(epBroadcast_, this);

	if (!epBroadcast_.good() ||
		 epBroadcast_.bind(htons(KBE_MACHINE_BRAODCAST_PORT), broadcastAddr_) == -1)
	{
		ERROR_MSG("Failed to bind socket to '%s'. %s.\n",
							inet_ntoa((struct in_addr &)broadcastAddr_),
							strerror(errno));
		return false;
	}

	epBroadcast_.setbroadcast( true );

	Mercury::Address address;
	address.ip = 0;
	address.port = 0;
	epBroadcast_.getlocaladdress( (u_int16_t*)&address.port,
		(u_int32_t*)&address.ip );

	epBroadcast_.setnonblocking(true);
	epBroadcast_.addr(address);

	INFO_MSG("bind broadcast successfully! addr:%s\n", epBroadcast_.addr().c_str());
	return true;
}

//-------------------------------------------------------------------------------------
bool Machine::run()
{
	bool ret = true;

	while(true)
	{
		this->getMainDispatcher().processOnce(false);
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
