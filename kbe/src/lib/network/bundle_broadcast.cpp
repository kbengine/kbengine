// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
	itry_(5),
	machine_addresses_()
{
	epListen_.socket(SOCK_DGRAM);
	epBroadcast_.socket(SOCK_DGRAM);

	if (!epListen_.good() || !epBroadcast_.good())
	{
		ERROR_MSG(fmt::format("BundleBroadcast::BundleBroadcast: Socket initialization failed! {}\n", 
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
					WARNING_MSG(fmt::format("BundleBroadcast::BundleBroadcast: Cannot bind to port {}, {}\n",
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
void BundleBroadcast::addBroadCastAddress(std::string addr)
{
	machine_addresses_.push_back(addr);
}

//-------------------------------------------------------------------------------------
bool BundleBroadcast::broadcast(uint16 port)
{
	if (!epBroadcast_.good())
		return false;
	
	if(port == 0)
		port = KBE_MACHINE_BROADCAST_SEND_PORT;

	epBroadcast_.addr(port, Network::BROADCAST);

	if (epBroadcast_.setbroadcast(true) != 0)
	{
		ERROR_MSG(fmt::format("BundleBroadcast::broadcast: Cannot broadcast socket on port {}, {}\n",
			port, kbe_strerror()));

		networkInterface_.dispatcher().breakProcessing();
		return false;
	}

	this->finiMessage();
	KBE_ASSERT(packets().size() == 1);

	epBroadcast_.sendto(packets()[0]->data(), packets()[0]->length(), htons(port), Network::BROADCAST);

	// 如果指定了地址池，则向所有地址发送消息
	std::vector< std::string >::iterator addr_iter = machine_addresses_.begin();
	for (; addr_iter != machine_addresses_.end(); ++addr_iter)
	{
		Network::EndPoint ep;
		ep.socket(SOCK_DGRAM);

		if (!ep.good())
		{
			ERROR_MSG("BundleBroadcast::broadcast: ep error!\n");
			break;
		}

		u_int32_t  uaddress;
		Network::Address::string2ip((*addr_iter).c_str(), uaddress);
		ep.sendto(packets()[0]->data(), packets()[0]->length(), htons(KBE_MACHINE_BROADCAST_SEND_PORT), uaddress);
	}

	return true;
}

//-------------------------------------------------------------------------------------
#if KBE_PLATFORM != PLATFORM_UNIX
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
					ERROR_MSG("BundleBroadcast::receive: failed! It can be caused by the firewall, the broadcastaddr, etc."
					"Maybe broadcastaddr is not a LAN ADDR, or the Machine process is not running.\n");
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
					ERROR_MSG(fmt::format("BundleBroadcast::receive: data wrong. size={}, from {}.\n",
							len, inet_ntoa((struct in_addr&)psin->sin_addr.s_addr)));

					continue;
				}
			}

			break;
		}
	}
	
	return true;
}

#else
#include <sys/poll.h>
bool BundleBroadcast::receive(MessageArgs* recvArgs, sockaddr_in* psin, int32 timeout, bool showerr)
{
	if (!epListen_.good())
		return false;

	int maxi = 0;
	int icount = 1;
	struct pollfd clientfds[1024];

	clientfds[0].fd = epListen_;
	clientfds[0].events = POLLIN;

	for (int i = 1; i < 1024; i++)
		clientfds[i].fd = -1;

	if (!pCurrPacket())
		newPacket();

	while (1)
	{
		int nready = poll(clientfds, maxi + 1, timeout / 1000);

		if (nready == -1)
		{
			if (showerr)
			{
				ERROR_MSG(fmt::format("BundleBroadcast::receive: select error. {}.\n",
					kbe_strerror()));
			}

			return false;
		}
		else if (nready == 0)
		{
			if (icount > itry_)
			{
				if (showerr)
				{
					ERROR_MSG("BundleBroadcast::receive: failed! It can be caused by the firewall, the broadcastaddr, etc."
						"Maybe broadcastaddr is not a LAN ADDR, or the Machine process is not running.\n");
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
		else
		{
			if (clientfds[0].revents & POLLIN)
			{
				sockaddr_in	sin;
				pCurrPacket()->resetPacket();

				if (psin == NULL)
					psin = &sin;

				pCurrPacket()->data_resize(recvWindowSize_);

				int len = epListen_.recvfrom(pCurrPacket()->data(), recvWindowSize_, *psin);
				if (len == -1)
				{
					if (showerr)
					{
						ERROR_MSG(fmt::format("BundleBroadcast::receive: recvfrom error. {}.\n",
							kbe_strerror()));
					}

					continue;
				}

				//DEBUG_MSG(fmt::format("BundleBroadcast::receive: from {}, datalen={}.\n", 
				//	inet_ntoa((struct in_addr&)psin->sin_addr.s_addr), len));

				pCurrPacket()->wpos(len);

				if (recvArgs != NULL)
				{
					try
					{
						recvArgs->createFromStream(*pCurrPacket());
					}
					catch (MemoryStreamException &)
					{
						ERROR_MSG(fmt::format("BundleBroadcast::receive: data wrong. size={}, from {}.\n",
							len, inet_ntoa((struct in_addr&)psin->sin_addr.s_addr)));

						continue;
					}
				}

				break;
			}
		}
	}

	return true;

}

#endif

//-------------------------------------------------------------------------------------
}
}
