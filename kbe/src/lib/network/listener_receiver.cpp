#include "listener_receiver.hpp"
#ifndef CODE_INLINE
#include "listener_receiver.ipp"
#endif

#include "network/address.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "network/endpoint.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/packet_receiver.hpp"

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
ListenerReceiver::ListenerReceiver(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	endpoint_(endpoint),
	networkInterface_(networkInterface)
{
}

//-------------------------------------------------------------------------------------
ListenerReceiver::~ListenerReceiver()
{
}

//-------------------------------------------------------------------------------------
int ListenerReceiver::handleInputNotification(int fd)
{
	EndPoint* pNewEndPoint = endpoint_.accept();
	if(pNewEndPoint == NULL){
		WARNING_MSG("PacketReceiver::handleInputNotification: accept endpoint(%d) %s!\n",
			 fd, strerror(errno));
		
		this->dispatcher().errorReporter().reportException(
				REASON_GENERAL_NETWORK);
	}
	else
	{
		Channel* pchannel = new Channel(networkInterface_, pNewEndPoint, Channel::INTERNAL);
		if(networkInterface_.registerChannel(pchannel))
		{
		}
	}
	return 0;
}

//-------------------------------------------------------------------------------------
EventDispatcher & ListenerReceiver::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
}
}