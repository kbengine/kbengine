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


#include "bundle_broadcast.hpp"
#ifndef CODE_INLINE
#include "bundle_broadcast.ipp"
#endif

#include "network/address.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/event_poller.hpp"


namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
BundleBroadcast::BundleBroadcast(NetworkInterface & networkInterface, 
								   uint16 bindPort, uint32 recvWindowSize):
	Bundle(NULL, Mercury::PROTOCOL_UDP),
	epListen_(),
	networkInterface_(networkInterface),
	recvWindowSize_(recvWindowSize)
{
	// Initialise the endpoint
	epListen_.socket(SOCK_DGRAM);
	epBroadcast_.socket(SOCK_DGRAM);

	if (!epListen_.good() ||
		epListen_.bind(htons(bindPort), htonl(INADDR_ANY)) == -1)
	{
		ERROR_MSG("BundleBroadcast::BundleBroadcast: Couldn't bind listener socket to port %d, %s\n", 
			bindPort, kbe_strerror());
		networkInterface.mainDispatcher().breakProcessing();
	}
	else
	{
		Address addr;
		epListen_.getlocaladdress( (u_int16_t*)&addr.port,
			(u_int32_t*)&addr.ip );
		
		epListen_.addr(addr);

		// DEBUG_MSG("BundleBroadcast::BundleBroadcast: epListen %s\n", epListen_.c_str());
		if(epBroadcast_.setbroadcast(true) != 0)
		{
			ERROR_MSG("BundleBroadcast::BundleBroadcast: Couldn't broadcast socket, port %d, %s\n", 
				bindPort, kbe_strerror());
			networkInterface.mainDispatcher().breakProcessing();
		}
	}
}

//-------------------------------------------------------------------------------------
BundleBroadcast::~BundleBroadcast()
{
}

//-------------------------------------------------------------------------------------
EventDispatcher & BundleBroadcast::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
bool BundleBroadcast::broadcast(uint16 port)
{
	if (!epBroadcast_.good())
		return false;
	
	if(port == 0)
		port = KBE_MACHINE_BRAODCAST_PORT;

	
	this->sendto(epBroadcast_, htons(port), Mercury::BROADCAST);
	return true;
}

//-------------------------------------------------------------------------------------
bool BundleBroadcast::receive(MessageArgs* recvArgs, sockaddr_in* psin)
{
	if (!epListen_.good())
		return false;

	struct timeval tv;
	fd_set fds;
	
	int icount = 1;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	
	if(!pCurrPacket())
		newPacket();

	while (1)
	{
		FD_ZERO( &fds );
		FD_SET((int)epListen_, &fds);
		int selgot = select(epListen_+1, &fds, NULL, NULL, &tv);

		if (selgot == 0)
		{
			DEBUG_MSG("BundleBroadcast::receive: waiting(%d), listen(%s) ...\n", icount++, epListen_.addr().c_str());
			continue;
		}
		else if (selgot == -1)
		{
			ERROR_MSG("BundleBroadcast::receive: select error. %s.\n",
					kbe_strerror());
			return false;
		}
		else
		{
			sockaddr_in	sin;
			pCurrPacket()->resetPacket();
			
			if(psin == NULL)
				psin = &sin;

			int len = epListen_.recvfrom(pCurrPacket()->data(), recvWindowSize_, *psin);
			if (len == -1)
			{
				ERROR_MSG("BundleBroadcast::receive: recvfrom error. %s.\n",
						kbe_strerror());
				continue;
			}
			
			pCurrPacket()->wpos(len);
			if(recvArgs != NULL)
				recvArgs->createFromStream(*pCurrPacket());
			break;

		}
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
}
}