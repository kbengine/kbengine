#include "broadcast_interface.hpp"
#ifndef CODE_INLINE
#include "broadcast_interface.ipp"
#endif

#include "network/address.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "network/endpoint.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/packet_receiver.hpp"
#include "network/error_reporter.hpp"
#include "network/udp_packet.hpp"

#include "../server/machine/machine_interface.hpp"

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
BroadcastInterface::BroadcastInterface(NetworkInterface & networkInterface) :
	Mercury::UDPPacketReceiver(epBroadcast_, networkInterface),
	epBroadcast_(),
	networkInterface_(networkInterface)
{
	epBroadcast_.socket(SOCK_DGRAM);
	epBroadcast_.setbroadcast(true);
}

//-------------------------------------------------------------------------------------
BroadcastInterface::~BroadcastInterface()
{
	epBroadcast_.close();
}

//-------------------------------------------------------------------------------------
EventDispatcher & BroadcastInterface::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
void BroadcastInterface::process()
{
	Bundle bundle(NULL, PROTOCOL_UDP);
	bundle.newMessage(MachineInterface::onBroadcastInterface);
	MachineInterface::onBroadcastInterfaceArgs4 args;
	bundle.sendto(epBroadcast_, htons(KBE_MACHINE_BRAODCAST_PORT), Mercury::BROADCAST);
}

//-------------------------------------------------------------------------------------
}
}