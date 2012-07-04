/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


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
#include "network/error_reporter.hpp"

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
			 fd, kbe_strerror());
		
		this->dispatcher().errorReporter().reportException(
				REASON_GENERAL_NETWORK);
	}
	else
	{
		Channel* pchannel = new Channel(networkInterface_, pNewEndPoint, Channel::EXTERNAL);
		if(!networkInterface_.registerChannel(pchannel))
		{
			ERROR_MSG("ListenerReceiver::handleInputNotification:registerChannel(%s) is failed!\n",
				pchannel->c_str());
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