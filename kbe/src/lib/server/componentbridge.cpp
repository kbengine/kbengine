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


#include "componentbridge.hpp"
#ifndef CODE_INLINE
#include "componentbridge.ipp"
#endif
#include "network/bundle_broadcast.hpp"
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
	networkInterface_(networkInterface),
	componentType_(componentType),
	componentID_(componentID),
	broadcastCount_(1)
{
	// dispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
Componentbridge::~Componentbridge()
{
	//dispatcher().cancelFrequentTask(this);
	//DEBUG_MSG("Componentbridge::~Componentbridge(): local interface(componentType=%s, componentID=%"PRAppID")!\n", 
	//	COMPONENT_NAME[componentType_], componentID_);
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
	case LOGINAPP_TYPE:
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

		INFO_MSG("Componentbridge::process: finding %s...\n",
			COMPONENT_NAME[findComponentType]);
		
		srand((unsigned int)time(NULL));
		uint16 nport = KBE_PORT_START + (rand() % 1000);
		Mercury::BundleBroadcast bhandler(networkInterface_, nport);
		if(!bhandler.good())
			continue;

		if(bhandler.pCurrPacket() != NULL)
		{
			bhandler.pCurrPacket()->resetPacket();
		}

		bhandler.newMessage(MachineInterface::onFindInterfaceAddr);
		MachineInterface::onFindInterfaceAddrArgs6::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
			componentType_, findComponentType, networkInterface_.addr().ip, bhandler.epListen().addr().port);

		if(!bhandler.broadcast())
		{
			ERROR_MSG("Componentbridge::process: broadcast error!\n");
			return false;
		}
	
		MachineInterface::onBroadcastInterfaceArgs6 args;
		if(bhandler.receive(&args, 0))
		{
			if(args.componentType == UNKNOWN_COMPONENT_TYPE)
			{
				//INFO_MSG("Componentbridge::process: not found %s, try again...\n",
				//	COMPONENT_NAME[findComponentType]);
				
				KBEngine::sleep(1000);
				continue;
			}

			INFO_MSG("Componentbridge::process: found %s, addr:%s:%u\n",
				COMPONENT_NAME[args.componentType], inet_ntoa((struct in_addr&)args.addr), ntohs(args.port));

			ifind++;
			
			Componentbridge::getComponents().addComponent(args.uid, args.username.c_str(), 
				(KBEngine::COMPONENT_TYPE)args.componentType, args.componentID, args.addr, args.port);
		}
		else
		{
			ERROR_MSG("Componentbridge::process: receive error!\n");
			return false;
		}
	}

	return true;
}
//-------------------------------------------------------------------------------------
bool Componentbridge::process()
{
	// 如果是cellappmgr或者baseapmgrp则向machine请求获得dbmgr的地址
	Mercury::BundleBroadcast bhandler(networkInterface_, KBE_PORT_BROADCAST_DISCOVERY);

	bhandler.newMessage(MachineInterface::onBroadcastInterface);
	MachineInterface::onBroadcastInterfaceArgs6::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
		componentType_, componentID_, 
		networkInterface_.addr().ip, networkInterface_.addr().port);
	
	bhandler.broadcast();
	broadcastCount_--;

	if(broadcastCount_ <= 0)
	{
		bhandler.close();
		if(componentType_ != MACHINE_TYPE)
			findInterfaces();
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
}