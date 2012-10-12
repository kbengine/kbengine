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

#include "../../server/machine/machine_interface.hpp"

namespace KBEngine { 
KBE_SINGLETON_INIT(Componentbridge);

//-------------------------------------------------------------------------------------
Componentbridge::Componentbridge(Mercury::NetworkInterface & networkInterface, 
									   COMPONENT_TYPE componentType, COMPONENT_ID componentID) :
	Task(),
	networkInterface_(networkInterface),
	componentType_(componentType),
	componentID_(componentID)
{
	// dispatcher().addFrequentTask(this);
	getComponents().pNetworkInterface(&networkInterface);
}

//-------------------------------------------------------------------------------------
Componentbridge::~Componentbridge()
{
	//dispatcher().cancelFrequentTask(this);
	//DEBUG_MSG("Componentbridge::~Componentbridge(): local interface(componentType=%s, componentID=%"PRAppID")!\n", 
	//	COMPONENT_NAME_EX(componentType_), componentID_);

	getComponents().clear(0, false);
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
void Componentbridge::onChannelDeregister(Mercury::Channel * pChannel)
{
	getComponents().removeComponentFromChannel(pChannel);
}

//-------------------------------------------------------------------------------------
bool Componentbridge::findInterfaces()
{
	int8 findComponentTypes[] = {UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE, 
								UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE, UNKNOWN_COMPONENT_TYPE};

	switch(componentType_)
	{
	case CELLAPP_TYPE:
		findComponentTypes[0] = MESSAGELOG_TYPE;
		findComponentTypes[1] = RESOURCEMGR_TYPE;
		findComponentTypes[2] = DBMGR_TYPE;
		findComponentTypes[3] = CELLAPPMGR_TYPE;
		findComponentTypes[4] = BASEAPPMGR_TYPE;
		break;
	case BASEAPP_TYPE:
		findComponentTypes[0] = MESSAGELOG_TYPE;
		findComponentTypes[1] = RESOURCEMGR_TYPE;
		findComponentTypes[2] = DBMGR_TYPE;
		findComponentTypes[3] = BASEAPPMGR_TYPE;
		findComponentTypes[4] = CELLAPPMGR_TYPE;
		break;
	case BASEAPPMGR_TYPE:
		findComponentTypes[0] = MESSAGELOG_TYPE;
		findComponentTypes[1] = DBMGR_TYPE;
		findComponentTypes[2] = CELLAPPMGR_TYPE;
		break;
	case CELLAPPMGR_TYPE:
		findComponentTypes[0] = MESSAGELOG_TYPE;
		findComponentTypes[1] = DBMGR_TYPE;
		findComponentTypes[2] = BASEAPPMGR_TYPE;
		break;
	case LOGINAPP_TYPE:
		findComponentTypes[0] = MESSAGELOG_TYPE;
		findComponentTypes[1] = DBMGR_TYPE;
		findComponentTypes[2] = BASEAPPMGR_TYPE;
		break;
	case DBMGR_TYPE:
		findComponentTypes[0] = MESSAGELOG_TYPE;
		break;
	default:
		if(componentType_ != MESSAGELOG_TYPE && componentType_ != MACHINE_TYPE)
			findComponentTypes[0] = MESSAGELOG_TYPE;
		break;
	};

	int ifind = 0;
	srand(KBEngine::getSystemTime());
	uint16 nport = KBE_PORT_START + (rand() % 1000);

	while(findComponentTypes[ifind] != UNKNOWN_COMPONENT_TYPE)
	{
		if(dispatcher().isBreakProcessing())
			return false;

		int8 findComponentType = findComponentTypes[ifind];

		INFO_MSG("Componentbridge::process: finding %s...\n",
			COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType));
		
		Mercury::BundleBroadcast bhandler(networkInterface_, nport);
		if(!bhandler.good())
		{
			KBEngine::sleep(10);
			nport = KBE_PORT_START + (rand() % 1000);
			continue;
		}

		if(bhandler.pCurrPacket() != NULL)
		{
			bhandler.pCurrPacket()->resetPacket();
		}

		bhandler.newMessage(MachineInterface::onFindInterfaceAddr);
		MachineInterface::onFindInterfaceAddrArgs6::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
			componentType_, findComponentType, networkInterface_.intaddr().ip, bhandler.epListen().addr().port);

		if(!bhandler.broadcast())
		{
			ERROR_MSG("Componentbridge::process: broadcast error!\n");
			return false;
		}
	
		MachineInterface::onBroadcastInterfaceArgs8 args;
		if(bhandler.receive(&args, 0, 1000000))
		{
			if(args.componentType == UNKNOWN_COMPONENT_TYPE)
			{
				//INFO_MSG("Componentbridge::process: not found %s, try again...\n",
				//	COMPONENT_NAME_EX(findComponentType));
				
				KBEngine::sleep(1000);
				
				// 如果是这些辅助组件没找到则跳过
				if(findComponentType == MESSAGELOG_TYPE || findComponentType == RESOURCEMGR_TYPE)
				{
					WARNING_MSG("Componentbridge::process: not found %s!\n",
						COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType));

					findComponentTypes[ifind] = -1; // 跳过标志

					ifind++;
				}

				continue;
			}

			INFO_MSG("Componentbridge::process: found %s, addr:%s:%u\n",
				COMPONENT_NAME_EX((COMPONENT_TYPE)args.componentType), inet_ntoa((struct in_addr&)args.intaddr), ntohs(args.intaddr));

			Componentbridge::getComponents().addComponent(args.uid, args.username.c_str(), 
				(KBEngine::COMPONENT_TYPE)args.componentType, args.componentID, args.intaddr, args.intport, args.extaddr, args.extport);
			
			// 防止接收到的数据不是想要的数据
			if(findComponentType == args.componentType)
			{
				ifind++;
			}
			else
			{
				ERROR_MSG("Componentbridge::process: %s not found. receive data is error!\n", COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType));
			}
		}
		else
		{
			ERROR_MSG("Componentbridge::process: receive error!\n");
			return false;
		}
	}
	
	ifind = 0;

	// 开始注册到所有的组件
	while(findComponentTypes[ifind] != UNKNOWN_COMPONENT_TYPE)
	{
		if(dispatcher().isBreakProcessing())
			return false;

		int8 findComponentType = findComponentTypes[ifind++];
		
		if(findComponentType == -1)
			continue;

		INFO_MSG("Componentbridge::process: register self to %s...\n",
			COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType));

		if(getComponents().connectComponent(static_cast<COMPONENT_TYPE>(findComponentType), getUserUID(), 0) != 0)
		{
			ERROR_MSG("Componentbridge::register self to %s is error!\n",
			COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType));

			dispatcher().breakProcessing();
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
	MachineInterface::onBroadcastInterfaceArgs8::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
		componentType_, componentID_, 
		networkInterface_.intaddr().ip, networkInterface_.intaddr().port,
		networkInterface_.extaddr().ip, networkInterface_.extaddr().port);
	
	bhandler.broadcast();

	bhandler.close();
	if(componentType_ != MACHINE_TYPE)
		findInterfaces();

	return false;
}

//-------------------------------------------------------------------------------------
}
