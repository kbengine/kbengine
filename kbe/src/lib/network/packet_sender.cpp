#include "packet_sender.hpp"
#ifndef CODE_INLINE
#include "packet_sender.ipp"
#endif

#include "network/address.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "network/endpoint.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/event_poller.hpp"
namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
PacketSender::PacketSender(EndPoint & endpoint,
	   NetworkInterface & networkInterface):
	endpoint_(endpoint),
	networkInterface_(networkInterface)
{
}

//-------------------------------------------------------------------------------------
PacketSender::~PacketSender()
{
}

//-------------------------------------------------------------------------------------
int PacketSender::handleOutputNotification(int fd)
{
	return 0;
}

//-------------------------------------------------------------------------------------
EventDispatcher & PacketSender::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
}
}