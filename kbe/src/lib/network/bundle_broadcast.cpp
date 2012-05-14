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

	if (!epListen_.good() ||
		 epListen_.bind(htons(bindPort)) == -1)
	{
		ERROR_MSG("BundleBroadcast::receive: Couldn't bind listener socket to port %d, %s\n", 
			bindPort, kbe_strerror());
		networkInterface.mainDispatcher().breakProcessing();
	}
	else
	{
		epListen_.setbroadcast(true);
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
	if (!epListen_.good())
		return false;
	
	if(port == 0)
		port = KBE_MACHINE_BRAODCAST_PORT;
	this->sendto(epListen_, htons(port), Mercury::BROADCAST);
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
			DEBUG_MSG("BundleBroadcast::receive: waiting(%d) ...\n", icount++);
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