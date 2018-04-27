// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "listener_receiver.h"
#ifndef CODE_INLINE
#include "listener_receiver.inl"
#endif

#include "network/address.h"
#include "network/bundle.h"
#include "network/endpoint.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/packet_receiver.h"
#include "network/error_reporter.h"

namespace KBEngine { 
namespace Network
{
//-------------------------------------------------------------------------------------
ListenerReceiver::ListenerReceiver(EndPoint & endpoint,
								   Channel::Traits traits, 
									NetworkInterface & networkInterface	) :
	endpoint_(endpoint),
	traits_(traits),
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
	int tickcount = 0;

	while(tickcount ++ < 256)
	{
		EndPoint* pNewEndPoint = endpoint_.accept();
		if(pNewEndPoint == NULL){

			if(tickcount == 1)
			{
				WARNING_MSG(fmt::format("PacketReceiver::handleInputNotification: accept endpoint({}) {}! channelSize={}\n",
					fd, kbe_strerror(), networkInterface_.channels().size()));
				
				this->dispatcher().errorReporter().reportException(
						REASON_GENERAL_NETWORK);
			}

			break;
		}
		else
		{
			Channel* pChannel = Network::Channel::createPoolObject();
			bool ret = pChannel->initialize(networkInterface_, pNewEndPoint, traits_);
			if(!ret)
			{
				ERROR_MSG(fmt::format("ListenerReceiver::handleInputNotification: initialize({}) is failed!\n",
					pChannel->c_str()));

				pChannel->destroy();
				Network::Channel::reclaimPoolObject(pChannel);
				return 0;
			}

			if(!networkInterface_.registerChannel(pChannel))
			{
				ERROR_MSG(fmt::format("ListenerReceiver::handleInputNotification: registerChannel({}) is failed!\n",
					pChannel->c_str()));

				pChannel->destroy();
				Network::Channel::reclaimPoolObject(pChannel);
			}
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
