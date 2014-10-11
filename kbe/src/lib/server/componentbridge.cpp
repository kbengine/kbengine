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
#include "helper/sys_info.hpp"
#include "server/serverconfig.hpp"

#include "../../server/machine/machine_interface.hpp"

namespace KBEngine { 
KBE_SINGLETON_INIT(Componentbridge);

//-------------------------------------------------------------------------------------
Componentbridge::Componentbridge(Mercury::NetworkInterface & networkInterface, 
									   COMPONENT_TYPE componentType, COMPONENT_ID componentID) :
	Task(),
	networkInterface_(networkInterface),
	componentType_(componentType),
	componentID_(componentID),
	state_(0),
	findIdx_(0)
{
	for(uint8 i=0; i<8; i++)
		findComponentTypes_[i] = UNKNOWN_COMPONENT_TYPE;

	switch(componentType_)
	{
	case CELLAPP_TYPE:
		findComponentTypes_[0] = MESSAGELOG_TYPE;
		findComponentTypes_[1] = DBMGR_TYPE;
		findComponentTypes_[2] = CELLAPPMGR_TYPE;
		findComponentTypes_[3] = BASEAPPMGR_TYPE;
		break;
	case BASEAPP_TYPE:
		findComponentTypes_[0] = MESSAGELOG_TYPE;
		findComponentTypes_[1] = DBMGR_TYPE;
		findComponentTypes_[2] = BASEAPPMGR_TYPE;
		findComponentTypes_[3] = CELLAPPMGR_TYPE;
		break;
	case BASEAPPMGR_TYPE:
		findComponentTypes_[0] = MESSAGELOG_TYPE;
		findComponentTypes_[1] = DBMGR_TYPE;
		findComponentTypes_[2] = CELLAPPMGR_TYPE;
		break;
	case CELLAPPMGR_TYPE:
		findComponentTypes_[0] = MESSAGELOG_TYPE;
		findComponentTypes_[1] = DBMGR_TYPE;
		findComponentTypes_[2] = BASEAPPMGR_TYPE;
		break;
	case LOGINAPP_TYPE:
		findComponentTypes_[0] = MESSAGELOG_TYPE;
		findComponentTypes_[1] = DBMGR_TYPE;
		findComponentTypes_[2] = BASEAPPMGR_TYPE;
		break;
	case DBMGR_TYPE:
		findComponentTypes_[0] = MESSAGELOG_TYPE;
		break;
	default:
		if(componentType_ != MESSAGELOG_TYPE && 
			componentType_ != MACHINE_TYPE && 
			componentType_ != BILLING_TYPE)
			findComponentTypes_[0] = MESSAGELOG_TYPE;
		break;
	};

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
	return networkInterface_.mainDispatcher();
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
	if(state_ == 1)
	{
		srand(KBEngine::getSystemTime());
		uint16 nport = KBE_PORT_START + (rand() % 1000);

		while(findComponentTypes_[findIdx_] != UNKNOWN_COMPONENT_TYPE)
		{
			if(dispatcher().isBreakProcessing() || dispatcher().isWaitBreakProcessing())
				return false;

			int8 findComponentType = findComponentTypes_[findIdx_];

			static int count = 0;
			INFO_MSG(fmt::format("Componentbridge::process: finding {}({})...\n",
				COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType), ++count));
			
			Mercury::BundleBroadcast bhandler(networkInterface_, nport);
			if(!bhandler.good())
			{
				return false;
			}

			bhandler.itry(0);
			if(bhandler.pCurrPacket() != NULL)
			{
				bhandler.pCurrPacket()->resetPacket();
			}

			bhandler.newMessage(MachineInterface::onFindInterfaceAddr);
			MachineInterface::onFindInterfaceAddrArgs7::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
				componentType_, componentID_, findComponentType, networkInterface_.intaddr().ip, bhandler.epListen().addr().port);

			if(!bhandler.broadcast())
			{
				ERROR_MSG("Componentbridge::process: broadcast error!\n");
				return false;
			}
		
			int32 timeout = 1500000;
			bool showerr = true;
			MachineInterface::onBroadcastInterfaceArgs22 args;

RESTART_RECV:

			if(bhandler.receive(&args, 0, timeout, showerr))
			{
				bool isContinue = false;
				showerr = false;
				timeout = 1000000;

				do
				{
					if(isContinue)
					{
						try
						{
							args.createFromStream(*bhandler.pCurrPacket());
						}catch(MemoryStreamException &)
						{
							break;
						}
					}
					
					if(args.componentIDEx != componentID_)
					{
						WARNING_MSG(fmt::format("Componentbridge::process: msg.componentID {} != {}.\n", 
							args.componentIDEx, componentID_));
						
						args.componentIDEx = 0;
						goto RESTART_RECV;
					}

					// ����Ҳ���
					if(args.componentType == UNKNOWN_COMPONENT_TYPE)
					{
						isContinue = true;
						continue;
					}

					INFO_MSG(fmt::format("Componentbridge::process: found {}, addr:{}:{}\n",
						COMPONENT_NAME_EX((COMPONENT_TYPE)args.componentType),
						inet_ntoa((struct in_addr&)args.intaddr),
						ntohs(args.intport)));

					Components::getSingleton().addComponent(args.uid, args.username.c_str(), 
						(KBEngine::COMPONENT_TYPE)args.componentType, args.componentID, args.globalorderid, args.grouporderid, 
						args.intaddr, args.intport, args.extaddr, args.extport, args.extaddrEx, args.pid, args.cpu, args.mem, 
						args.usedmem, args.extradata, args.extradata1, args.extradata2, args.extradata3);

					isContinue = true;
				}while(bhandler.pCurrPacket()->opsize() > 0);

				// ��ֹ���յ������ݲ�����Ҫ������
				if(findComponentType == args.componentType)
				{
					// �������������� ��messagelog������������ȥ�� �������Ծ���ͬ����־
					if(findComponentType == (int8)MESSAGELOG_TYPE)
					{
						findComponentTypes_[findIdx_] = -1;
						if(getComponents().connectComponent(static_cast<COMPONENT_TYPE>(findComponentType), getUserUID(), 0) != 0)
						{
							ERROR_MSG(fmt::format("Componentbridge::register self to {} is error!\n",
							COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType)));
							findIdx_++;
							//dispatcher().breakProcessing();
							return false;
						}
						else
						{
							findIdx_++;
							continue;
						}
					}
				}
				
				goto RESTART_RECV;
			}
			else
			{
				if(Components::getSingleton().getComponents((COMPONENT_TYPE)findComponentType).size() > 0)
				{
					findIdx_++;
					count = 0;
				}
				else
				{
					if(showerr)
					{
						ERROR_MSG("Componentbridge::process: receive error!\n");
					}

					// �������Щ�������û�ҵ�������
					int helperComponentIdx = 0;
					while(1)
					{
						COMPONENT_TYPE helperComponentType = ALL_HELPER_COMPONENT_TYPE[helperComponentIdx++];
						if(helperComponentType == UNKNOWN_COMPONENT_TYPE)
						{
							break;
						}
						else if(findComponentType == helperComponentType)
						{
							WARNING_MSG(fmt::format("Componentbridge::process: not found {}!\n",
								COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType)));

							findComponentTypes_[findIdx_] = -1; // ������־
							count = 0;
							findIdx_++;
							return false;
						}
					}
				}

				return false;
			}
		}

		state_ = 2;
		findIdx_ = 0;
		return false;
	}

	if(state_ == 2)
	{
		// ��ʼע�ᵽ���е����
		while(findComponentTypes_[findIdx_] != UNKNOWN_COMPONENT_TYPE)
		{
			if(dispatcher().isBreakProcessing())
				return false;

			int8 findComponentType = findComponentTypes_[findIdx_];
			
			if(findComponentType == -1)
			{
				findIdx_++;
				return false;
			}

			INFO_MSG(fmt::format("Componentbridge::process: register self to {}...\n",
				COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType)));

			if(getComponents().connectComponent(static_cast<COMPONENT_TYPE>(findComponentType), getUserUID(), 0) != 0)
			{
				ERROR_MSG(fmt::format("Componentbridge::register self to {} is error!\n",
				COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType)));
				//dispatcher().breakProcessing();
				return false;
			}

			findIdx_++;
			return false;
		}
	}

	return true;
}
//-------------------------------------------------------------------------------------
bool Componentbridge::process()
{
	if(state_ == 0)
	{
		uint64 cidex = 0;
		while(cidex++ < 3)
		{
			// �����cellappmgr����baseapmgrp����machine������dbmgr�ĵ�ַ
			Mercury::BundleBroadcast bhandler(networkInterface_, KBE_PORT_BROADCAST_DISCOVERY);

			bhandler.newMessage(MachineInterface::onBroadcastInterface);
			MachineInterface::onBroadcastInterfaceArgs22::staticAddToBundle(bhandler, getUserUID(), getUsername(), 
				componentType_, componentID_, cidex, g_componentGlobalOrder, g_componentGroupOrder,
				networkInterface_.intaddr().ip, networkInterface_.intaddr().port,
				networkInterface_.extaddr().ip, networkInterface_.extaddr().port, g_kbeSrvConfig.getConfig().externalAddress, getProcessPID(),
				SystemInfo::getSingleton().getCPUPerByPID(), 0.f, (uint32)SystemInfo::getSingleton().getMemUsedByPID(), 0, 0, 0, 0, 0, 0);
			
			bhandler.broadcast();

			bhandler.close();

			sleep(50);
		}

		state_ = 1;
		return true;
	}
	else
	{
		if(componentType_ != MACHINE_TYPE)
		{
			static uint64 lastTime = timestamp();
			
			if(timestamp() - lastTime > uint64(stampsPerSecond()))
			{
				if(!findInterfaces())
				{
					if(state_ != 2)
						lastTime = timestamp();
					return true;
				}
			}
			else
				return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------
}
