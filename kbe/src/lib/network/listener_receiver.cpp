#include "listener_receiver.hpp"
#ifndef CODE_INLINE
#include "listener_receiver.ipp"
#endif

#include "network/address.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "network/socket.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/packet_receiver.hpp"

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
ListenerReceiver::ListenerReceiver(Socket & socket,
	   NetworkInterface & networkInterface	) :
	socket_(socket),
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
	Socket* pNewSocket = socket_.accept();
	if(pNewSocket == NULL){
		WARNING_MSG("PacketReceiver::handleInputNotification: accept socketID(%d) %s!\n",
			 fd, strerror(errno));
		
		this->dispatcher().errorReporter().reportException(
				REASON_GENERAL_NETWORK);
	}
	else
	{
		Channel* pchannel = new Channel(networkInterface_, pNewSocket, Channel::INTERNAL);
		
		if(networkInterface_.registerChannel(pchannel))
		{
			networkInterface_.dispatcher().registerFileDescriptor(pNewSocket->get(), pchannel->packetReceiver());
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