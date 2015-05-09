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


#include "bundle_broadcast.h"
#ifndef CODE_INLINE
#include "bundle_broadcast.inl"
#endif

#include "network/address.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/event_poller.h"


namespace KBEngine { 
namespace Network
{
//-------------------------------------------------------------------------------------
BundleBroadcast::BundleBroadcast(NetworkInterface & networkInterface, 
								   uint16 bindPort, uint32 recvWindowSize):
	Bundle(NULL, Network::PROTOCOL_UDP),
	epListen_(),
	networkInterface_(networkInterface),
	recvWindowSize_(recvWindowSize),
	good_(false),
	itry_(5)
{
	epListen_.socket(SOCK_DGRAM);
	epBroadcast_.socket(SOCK_DGRAM);

	if (!epListen_.good() || !epBroadcast_.good())
	{
		ERROR_MSG(fmt::format("BundleBroadcast::BundleBroadcast: init socket is error, {}\n", 
			kbe_strerror()));

		networkInterface_.dispatcher().breakProcessing();
	}
	else
	{
		int count = 0;

		while(true)
		{
			if (epListen_.bind(htons(bindPort), htonl(INADDR_ANY)) != 0)
			{
				good_ = false;
				KBEngine::sleep(10);
				count++;

				if(count > 30)
				{
					WARNING_MSG(fmt::format("BundleBroadcast::BundleBroadcast: Couldn't bind listener socket to port {}, {}\n",
						bindPort, kbe_strerror()));

					break;
				}
			}
			else
			{
				epListen_.addr(htons(bindPort), htonl(INADDR_ANY));
				good_ = true;

				// DEBUG_MSG(fmt::format("BundleBroadcast::BundleBroadcast: epListen {}\n", epListen_.c_str()));
				break;
			}
		}
	}

	pCurrPacket()->data_resize(recvWindowSize_);
}

//-------------------------------------------------------------------------------------
BundleBroadcast::~BundleBroadcast()
{
	close();
}

//-------------------------------------------------------------------------------------
void BundleBroadcast::close()
{
	// DEBUG_MSG("BundleBroadcast::close()\n");
	epListen_.close();
	epBroadcast_.close();
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
		port = KBE_MACHINE_BRAODCAST_SEND_PORT;

	epBroadcast_.addr(port, Network::BROADCAST);

	if(epBroadcast_.setbroadcast(true) != 0)
	{
		ERROR_MSG(fmt::format("BundleBroadcast::broadcast: Couldn't broadcast socket, port {}, {}\n", 
			port, kbe_strerror()));

		networkInterface_.dispatcher().breakProcessing();
		return false;
	}

	epBroadcast_.sendto(this, htons(port), Network::BROADCAST);
	return true;
}

//-------------------------------------------------------------------------------------
bool BundleBroadcast::receive(MessageArgs* recvArgs, sockaddr_in* psin, int32 timeout, bool showerr)
{
	if (!epListen_.good())
		return false;

	struct timeval tv;
	fd_set fds;
	
	int icount = 1;
	tv.tv_sec = 0;
	tv.tv_usec = timeout;
	
	if(!pCurrPacket())
		newPacket();

	while (1)
	{
		FD_ZERO( &fds );
		FD_SET((int)epListen_, &fds);
		int selgot = select(epListen_+1, &fds, NULL, NULL, &tv);

		if (selgot == 0)
		{
			if(icount > itry_)
			{
				if(showerr)
				{
					ERROR_MSG("BundleBroadcast::receive: is failed(please check {firewall rule => broadcastaddr not is LAN ?})!\n");
				}
				
				return false;
			}
			else
			{
				//DEBUG_MSG(fmt::format("BundleBroadcast::receive: retries({}), bind_addr({}) ...\n", 
				//	icount, epListen_.addr()));
			}

			icount++;
			continue;
		}
		else if (selgot == -1)
		{
			if(showerr)
			{
				ERROR_MSG(fmt::format("BundleBroadcast::receive: select error. {}.\n",
						kbe_strerror()));
			}

			return false;
		}
		else
		{
			sockaddr_in	sin;
			pCurrPacket()->resetPacket();
			
			if(psin == NULL)
				psin = &sin;

			pCurrPacket()->data_resize(recvWindowSize_);

			int len = epListen_.recvfrom(pCurrPacket()->data(), recvWindowSize_, *psin);
			if (len == -1)
			{
				if(showerr)
				{
					ERROR_MSG(fmt::format("BundleBroadcast::receive: recvfrom error. {}.\n",
							kbe_strerror()));
				}

				continue;
			}
			
			//DEBUG_MSG(fmt::format("BundleBroadcast::receive: from {}, datalen={}.\n", 
			//	inet_ntoa((struct in_addr&)psin->sin_addr.s_addr), len));

			pCurrPacket()->wpos(len);

			if(recvArgs != NULL)
			{
				try
				{
					recvArgs->createFromStream(*pCurrPacket());
				}
				catch(MemoryStreamException &)
				{
					ERROR_MSG(fmt::format("BundleBroadcast::receive: data is error. size={}, from {}.\n",
							len, inet_ntoa((struct in_addr&)psin->sin_addr.s_addr)));

					continue;
				}
			}

			break;
		}
	}
	
	return true;
}

//-------------------------------------------------------------------------------------
}
}
