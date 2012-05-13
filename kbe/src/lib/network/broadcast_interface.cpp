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
BroadcastInterface::BroadcastInterface(NetworkInterface & networkInterface, 
									   COMPONENT_TYPE componentType, COMPONENT_ID componentID) :
	Task(),
	Mercury::UDPPacketReceiver(epBroadcast_, networkInterface),
	epBroadcast_(),
	networkInterface_(networkInterface),
	componentType_(componentType),
	componentID_(componentID),
	broadcastCount_(3)
{
	epBroadcast_.socket(SOCK_DGRAM);
	epBroadcast_.setbroadcast(true);

	dispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
BroadcastInterface::~BroadcastInterface()
{
	epBroadcast_.close();
	//dispatcher().cancelFrequentTask(this);
	DEBUG_MSG("broadcast interface(componentType=%d, componentID=%d) is completed!\n", componentType_, componentID_);
}

//-------------------------------------------------------------------------------------
EventDispatcher & BroadcastInterface::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
bool BroadcastInterface::process()
{
	Bundle bundle(NULL, PROTOCOL_UDP);

	bundle.newMessage(MachineInterface::onBroadcastInterface);

	MachineInterface::onBroadcastInterfaceArgs6::staticAddToBundle(bundle, getUserUID(), getUsername(), 
		componentType_, componentID_, 
		networkInterface_.addr().ip, networkInterface_.addr().port);

	bundle.sendto(epBroadcast_, htons(KBE_MACHINE_BRAODCAST_PORT), Mercury::BROADCAST);

	broadcastCount_--;

	if(broadcastCount_ <= 0)
	{
		delete this;
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
}
}