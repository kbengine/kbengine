#include "componentbridge.hpp"
#ifndef CODE_INLINE
#include "componentbridge.ipp"
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
KBE_SINGLETON_INIT(Componentbridge);

//-------------------------------------------------------------------------------------
Componentbridge::Componentbridge(Mercury::NetworkInterface & networkInterface, 
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

	// dispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
Componentbridge::~Componentbridge()
{
	epBroadcast_.close();
	//dispatcher().cancelFrequentTask(this);
	DEBUG_MSG("broadcast interface(componentType=%d, componentID=%d) is completed!\n", componentType_, componentID_);
}

//-------------------------------------------------------------------------------------
Mercury::EventDispatcher & Componentbridge::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
Components& Componentbridge::getComponents()
{
	return Components::getSingleton();
}

//-------------------------------------------------------------------------------------
bool Componentbridge::findInterfaces()
{
	int8 findComponentTypes[] = {UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE, 
								UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE};
	
	switch(componentType_)
	{
	case CELLAPP_TYPE:
		findComponentTypes[0] = BASEAPPMGR_TYPE;
		findComponentTypes[1] = CELLAPPMGR_TYPE;
		findComponentTypes[2] = DBMGR_TYPE;
		break;
	case BASEAPP_TYPE:
		findComponentTypes[0] = BASEAPPMGR_TYPE;
		findComponentTypes[1] = CELLAPPMGR_TYPE;
		findComponentTypes[2] = DBMGR_TYPE;
		break;
	case BASEAPPMGR_TYPE:
		findComponentTypes[0] = CELLAPPMGR_TYPE;
		findComponentTypes[1] = DBMGR_TYPE;
		break;
	case CELLAPPMGR_TYPE:
		findComponentTypes[0] = BASEAPPMGR_TYPE;
		findComponentTypes[1] = DBMGR_TYPE;
		break;
	default:
		break;
	};

	int ifind = 0;

	while(findComponentTypes[ifind] != UNKNOWN_COMPONENT_TYPE)
	{
		int8 findComponentType = findComponentTypes[ifind];
		ifind++;

		Mercury::EndPoint epListen;
		struct timeval tv;
		fd_set fds;
		Mercury::UDPPacket streamBuf;
		// Initialise the endpoint
		epListen.socket(SOCK_DGRAM);
		if (!epListen.good() ||
			 epListen.bind(htons(0)) == -1)
		{
			ERROR_MSG("Componentbridge::process:Couldn't bind listener socket to port %d\n",
					0);
			return false;
		}

		epListen.setbroadcast(true);

		tv.tv_sec = 1;
		tv.tv_usec = 0;


		Mercury::Bundle bundle(NULL, Mercury::PROTOCOL_UDP);

		bundle.newMessage(MachineInterface::onFindInterfaceAddr);

		MachineInterface::onFindInterfaceAddrArgs4::staticAddToBundle(bundle, getUserUID(), getUsername(), 
			componentType_, findComponentType);

		bundle.sendto(epListen, htons(KBE_MACHINE_BRAODCAST_PORT), Mercury::BROADCAST);

		// Listen for the message we just sent to come back to ourselves.
		while (1)
		{
			FD_ZERO( &fds );
			FD_SET((int)epListen, &fds);
			int selgot = select(epListen+1, &fds, NULL, NULL, &tv);
			if (selgot == 0)
			{
				DEBUG_MSG("Componentbridge::process:find %s...\n", COMPONENT_NAME[findComponentType]);
				continue;
			}
			else if (selgot == -1)
			{
				ERROR_MSG("Componentbridge::process:Broadcast discovery select error. %s.\n",
						kbe_strerror());
				return false;
			}
			else
			{
				sockaddr_in	sin;

				// Read packet into buffer
				int len = epListen.recvfrom(streamBuf.data(), 4096, sin);
				if (len == -1)
				{
					ERROR_MSG("Componentbridge::process:Broadcast discovery recvfrom error. %s.\n",
							kbe_strerror());
					continue;
				}
				
				streamBuf.wpos(len);

				MachineInterface::onBroadcastInterfaceArgs6 args;
				args.createFromStream(streamBuf);
				

				INFO_MSG("Componentbridge::process: found %s, addr:%s:.\n",
					COMPONENT_NAME[args.componentType], inet_ntoa((struct in_addr&)args.addr), args.port);

			}
		}
	}

	return true;
}
//-------------------------------------------------------------------------------------
bool Componentbridge::process()
{
	// 如果是cellappmgr或者baseapmgrp则向machine请求获得dbmgr的地址
	Mercury::Bundle bundle(NULL, Mercury::PROTOCOL_UDP);

	bundle.newMessage(MachineInterface::onBroadcastInterface);

	MachineInterface::onBroadcastInterfaceArgs6::staticAddToBundle(bundle, getUserUID(), getUsername(), 
		componentType_, componentID_, 
		networkInterface_.addr().ip, networkInterface_.addr().port);

	bundle.sendto(epBroadcast_, htons(KBE_MACHINE_BRAODCAST_PORT), Mercury::BROADCAST);

	broadcastCount_--;

	if(broadcastCount_ <= 0)
	{
		findInterfaces();
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
}