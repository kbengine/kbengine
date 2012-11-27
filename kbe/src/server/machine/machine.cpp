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


#include "machine.hpp"
#include "machine_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "network/bundle_broadcast.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"

#include "../baseappmgr/baseappmgr_interface.hpp"
#include "../cellappmgr/cellappmgr_interface.hpp"
#include "../baseapp/baseapp_interface.hpp"
#include "../cellapp/cellapp_interface.hpp"
#include "../dbmgr/dbmgr_interface.hpp"
#include "../loginapp/loginapp_interface.hpp"
#include "../resourcemgr/resourcemgr_interface.hpp"
#include "../tools/message_log/messagelog_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Machine);

//-------------------------------------------------------------------------------------
Machine::Machine(Mercury::EventDispatcher& dispatcher, 
				 Mercury::NetworkInterface& ninterface, 
				 COMPONENT_TYPE componentType,
				 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
	broadcastAddr_(0),
	ep_(),
	epBroadcast_(),
	epLocal_(),
	pEPPacketReceiver_(NULL),
	pEBPacketReceiver_(NULL),
	pEPLocalPacketReceiver_(NULL)

{
}

//-------------------------------------------------------------------------------------
Machine::~Machine()
{
	ep_.close();
	epBroadcast_.close();
	epLocal_.close();

	SAFE_RELEASE(pEPPacketReceiver_);
	SAFE_RELEASE(pEBPacketReceiver_);
	SAFE_RELEASE(pEPLocalPacketReceiver_);
}

//-------------------------------------------------------------------------------------
void Machine::onBroadcastInterface(Mercury::Channel* pChannel, int32 uid, std::string& username,
								   int8 componentType, uint64 componentID, 
									uint32 intaddr, uint16 intport,
									uint32 extaddr, uint16 extport)
{
	if(componentType == MACHINE_TYPE)
	{
		if(uid == getUserUID() && 
			intaddr == this->getNetworkInterface().intaddr().ip)
		{
			return;
		}
	}

	INFO_MSG(boost::format("Machine::onBroadcastInterface[%1%]: uid:%2%, username:%3%, componentType:%4%, "
			"componentID:%5%, intaddr:%6%, intport:%7%, extaddr:%8%, extport:%9%.\n") %
			pChannel->c_str() % 
			uid %
			username.c_str() %
			COMPONENT_NAME_EX((COMPONENT_TYPE)componentType) %
			componentID %
			inet_ntoa((struct in_addr&)intaddr) %
			ntohs(intport) %
			(extaddr != 0 ? inet_ntoa((struct in_addr&)extaddr) : "nonsupport") %
			ntohs(extport));

	Componentbridge::getComponents().addComponent(uid, username.c_str(), 
		(KBEngine::COMPONENT_TYPE)componentType, componentID, intaddr, intport, extaddr, extport);
}

//-------------------------------------------------------------------------------------
void Machine::onFindInterfaceAddr(Mercury::Channel* pChannel, int32 uid, std::string& username, int8 componentType, 
								  int8 findComponentType, uint32 finderAddr, uint16 finderRecvPort)
{
	KBEngine::COMPONENT_TYPE tfindComponentType = (KBEngine::COMPONENT_TYPE)findComponentType;
	KBEngine::COMPONENT_TYPE tComponentType = (KBEngine::COMPONENT_TYPE)componentType;

	// 如果不是guiconsole发出的, uid也不等于当前服务器的uid则不理会。
	if(tComponentType != CONSOLE_TYPE && uid != KBEngine::getUserUID())
		return;

	INFO_MSG(boost::format("Machine::onFindInterfaceAddr[%1%]: uid:%2%, username:%3%, "
			"componentType:%4%, find:%5%, finderaddr:%6%, finderRecvPort:%7%.\n") %
		pChannel->c_str() % uid % username.c_str() % 
		COMPONENT_NAME_EX((COMPONENT_TYPE)componentType) %
		COMPONENT_NAME_EX((COMPONENT_TYPE)findComponentType) %
		inet_ntoa((struct in_addr&)finderAddr) % ntohs(finderRecvPort));

	Mercury::EndPoint ep;
	ep.socket(SOCK_DGRAM);

	if (!ep.good())
	{
		ERROR_MSG("Machine::onFindInterfaceAddr: Failed to create socket.\n");
		return;
	}
	
	Components::COMPONENTS& components = Componentbridge::getComponents().getComponents(tfindComponentType);
	Components::COMPONENTS::iterator iter = components.begin();
	bool found = false;
	Mercury::Bundle bundle;

	for(; iter != components.end(); )
	{
		if(tComponentType != CONSOLE_TYPE)
		{
			if((*iter).uid != uid)
			{
				++iter;
				continue;
			}
		}

		const Components::ComponentInfos* pinfos = &(*iter);
		
		bool usable = Componentbridge::getComponents().checkComponentUsable(pinfos);
		
		if(usable)
		{
			if(ep_.addr().ip == pinfos->pIntAddr->ip || tComponentType == CONSOLE_TYPE)
			{
				found = true;
				MachineInterface::onBroadcastInterfaceArgs8::staticAddToBundle(bundle, pinfos->uid, 
					pinfos->username, findComponentType, pinfos->cid, pinfos->pIntAddr->ip, pinfos->pIntAddr->port,
					pinfos->pExtAddr->ip, pinfos->pExtAddr->port);
			}

			++iter;
		}
		else
		{
			WARNING_MSG(boost::format("Machine::onFindInterfaceAddr: %1%[%2%] invalid, erase %3%.\n") %
				COMPONENT_NAME_EX(pinfos->componentType) %
				pinfos->cid %
				COMPONENT_NAME_EX(pinfos->componentType));

			iter = components.erase(iter);
		}
	}

	if(!found)
	{
		// 如果是控制台， 且uid不是一致的则无需返回找不到消息
		// 控制台可能广播到其他组去了
		if(tComponentType == CONSOLE_TYPE && KBEngine::getUserUID() != uid)
		{
			return;
		}

		WARNING_MSG(boost::format("Machine::onFindInterfaceAddr: %1% not found %2%.\n") %
			COMPONENT_NAME_EX(tComponentType) % 
			COMPONENT_NAME_EX(tfindComponentType));

		MachineInterface::onBroadcastInterfaceArgs8::staticAddToBundle(bundle, KBEngine::getUserUID(), 
			"", UNKNOWN_COMPONENT_TYPE, 0, 0, 0, 0, 0);
	}

	if(finderAddr != 0 && finderRecvPort != 0)
		bundle.sendto(ep, finderRecvPort, finderAddr);
	else
		bundle.send(this->getNetworkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------
bool Machine::findBroadcastInterface()
{

	std::map<u_int32_t, std::string> interfaces;
	Mercury::BundleBroadcast bhandler(networkInterface_, KBE_PORT_BROADCAST_DISCOVERY);

	if (!bhandler.epListen().getInterfaces(interfaces))
	{
		ERROR_MSG("Machine::findBroadcastInterface: Failed to discover network interfaces\n");
		return false;
	}

	uint8 data = 1;
	bhandler << data;
	if (!bhandler.broadcast(KBE_PORT_BROADCAST_DISCOVERY))
	{
		ERROR_MSG(boost::format("Machine::findBroadcastInterface:Failed to send broadcast discovery message. error:%1%\n") %
			kbe_strerror());
		return false;
	}
	
	sockaddr_in	sin;

	if(bhandler.receive(NULL, &sin))
	{
		INFO_MSG(boost::format("Machine::findBroadcastInterface:Machine::findBroadcastInterface: Broadcast discovery receipt from %1%.\n") %
					inet_ntoa((struct in_addr&)sin.sin_addr.s_addr) );

		std::map< u_int32_t, std::string >::iterator iter;

		iter = interfaces.find( (u_int32_t &)sin.sin_addr.s_addr );
		if (iter != interfaces.end())
		{
			INFO_MSG(boost::format("Machine::findBroadcastInterface: Confirmed %1% (%2%) as default broadcast route interface.\n") %
				inet_ntoa((struct in_addr&)sin.sin_addr.s_addr) %
				iter->second.c_str());

			broadcastAddr_ = sin.sin_addr.s_addr;
			return true;
		}
	}
	
	std::string sinterface = "\t[";
	std::map< u_int32_t, std::string >::iterator iter = interfaces.begin();
	for(; iter != interfaces.end(); iter++)
	{
		sinterface += inet_ntoa((struct in_addr&)iter->first);
		sinterface += ", ";
	}

	sinterface += "]";

	ERROR_MSG(boost::format("Machine::findBroadcastInterface: Broadcast discovery [%1%] "
		"not a valid interface. available interfaces:%2%\n") %
		inet_ntoa((struct in_addr&)sin.sin_addr.s_addr) % sinterface.c_str());

	return false;
}

//-------------------------------------------------------------------------------------
bool Machine::initNetwork()
{
	epBroadcast_.socket(SOCK_DGRAM);
	ep_.socket(SOCK_DGRAM);
	epLocal_.socket(SOCK_DGRAM); 

	Mercury::Address address;
	address.ip = 0;
	address.port = 0;

	if (broadcastAddr_ == 0 && !this->findBroadcastInterface())
	{
		ERROR_MSG("Machine::initNetwork: Failed to determine default broadcast interface. "
				"Make sure that your broadcast route is set correctly. "
				"e.g. /sbin/ip route add broadcast 255.255.255.255 dev eth0\n" );

		return false;
	}

	if (!ep_.good() ||
		 ep_.bind(htons(KBE_MACHINE_BRAODCAST_SEND_PORT), broadcastAddr_) == -1)
	{
		ERROR_MSG(boost::format("Machine::initNetwork: Failed to bind socket to '%1%:%2%'. %3%.\n") %
							inet_ntoa((struct in_addr &)broadcastAddr_) %
							(KBE_MACHINE_BRAODCAST_SEND_PORT) %
							kbe_strerror());

		return false;
	}

	address.ip = broadcastAddr_;
	address.port = htons(KBE_MACHINE_BRAODCAST_SEND_PORT);
	ep_.setbroadcast( true );
	ep_.setnonblocking(true);
	ep_.addr(address);
	pEPPacketReceiver_ = new Mercury::UDPPacketReceiver(ep_, this->getNetworkInterface());

	if(!this->getMainDispatcher().registerFileDescriptor(ep_, pEPPacketReceiver_))
	{
		ERROR_MSG("Machine::initNetwork: registerFileDescriptor ep is failed!\n");
		return false;
	}
	
#if KBE_PLATFORM == PLATFORM_WIN32
	u_int32_t baddr = htonl(INADDR_ANY);
#else
	u_int32_t baddr = Mercury::BROADCAST;
#endif

	if (!epBroadcast_.good() ||
		epBroadcast_.bind(htons(KBE_MACHINE_BRAODCAST_SEND_PORT), baddr) == -1)
	{
		ERROR_MSG(boost::format("Machine::initNetwork: Failed to bind socket to '%1%:%2%'. %3%.\n") %
							inet_ntoa((struct in_addr &)baddr) %
							(KBE_MACHINE_BRAODCAST_SEND_PORT) %
							kbe_strerror());

#if KBE_PLATFORM != PLATFORM_WIN32
		return false;
#endif
	}
	else
	{
		address.ip = baddr;
		address.port = htons(KBE_MACHINE_BRAODCAST_SEND_PORT);
		epBroadcast_.setnonblocking(true);
		epBroadcast_.addr(address);
		pEBPacketReceiver_ = new Mercury::UDPPacketReceiver(epBroadcast_, this->getNetworkInterface());
	
		if(!this->getMainDispatcher().registerFileDescriptor(epBroadcast_, pEBPacketReceiver_))
		{
			ERROR_MSG("Machine::initNetwork: registerFileDescriptor epBroadcast is failed!\n");
			return false;
		}

	}

	if (!epLocal_.good() ||
		 epLocal_.bind(htons(KBE_MACHINE_BRAODCAST_SEND_PORT), Mercury::LOCALHOST) == -1)
	{
		ERROR_MSG(boost::format("Machine::initNetwork: Failed to bind socket to (lo). %1%.\n") %
							kbe_strerror() );

		return false;
	}

	address.ip = Mercury::LOCALHOST;
	address.port = htons(KBE_MACHINE_BRAODCAST_SEND_PORT);
	epLocal_.setnonblocking(true);
	epLocal_.addr(address);
	pEPLocalPacketReceiver_ = new Mercury::UDPPacketReceiver(epLocal_, this->getNetworkInterface());

	if(!this->getMainDispatcher().registerFileDescriptor(epLocal_, pEPLocalPacketReceiver_))
	{
		ERROR_MSG("Machine::initNetwork: registerFileDescriptor epLocal is failed!\n");
		return false;
	}

	INFO_MSG(boost::format("Machine::initNetwork: bind broadcast successfully! addr:%1%\n") % ep_.addr().c_str());
	return true;
}

//-------------------------------------------------------------------------------------
bool Machine::run()
{
	bool ret = true;

	while(!this->getMainDispatcher().isBreakProcessing())
	{
		threadPool_.onMainThreadTick();
		this->getMainDispatcher().processOnce(false);
		getNetworkInterface().handleChannels(&MachineInterface::messageHandlers);
		KBEngine::sleep(100);
	};

	return ret;
}

//-------------------------------------------------------------------------------------
void Machine::handleTimeout(TimerHandle handle, void * arg)
{
	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
bool Machine::initializeBegin()
{
	if(!initNetwork())
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool Machine::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Machine::initializeEnd()
{
	pActiveTimerHandle_.cancel(); // machine不需要与其他组件保持活动状态关系
	return true;
}

//-------------------------------------------------------------------------------------
void Machine::finalise()
{
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void Machine::startserver(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	int32 uid = 0;
	COMPONENT_TYPE componentType;
	uint32 recvip;
	uint16 recvport;
	Mercury::Bundle bundle;
	bool success = true;

	s >> uid;
	s >> componentType;
	s >> recvip;
	s >> recvport;


	INFO_MSG(boost::format("Machine::startserver: uid=%1%, [%2%], addr:%3%, recvPort:%4%.\n") % 
		uid %  COMPONENT_NAME[componentType] % 
		inet_ntoa((struct in_addr&)recvip) % ntohs(recvport));

	Mercury::EndPoint ep;
	ep.socket(SOCK_DGRAM);

	if (!ep.good())
	{
		ERROR_MSG("Machine::startserver: Failed to create socket.\n");
		return;
	}

#if KBE_PLATFORM == PLATFORM_WIN32
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	std::string str = Resmgr::getSingleton().getEnv().hybrid_path;
	str += COMPONENT_NAME[componentType];
	str += ".exe";

	wchar_t* szCmdline = KBEngine::char2wchar(str.c_str());
	wchar_t* currdir = KBEngine::char2wchar(Resmgr::getSingleton().getEnv().hybrid_path.c_str());

	ZeroMemory( &si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi));

	if(!CreateProcess( NULL,   // No module name (use command line)
		szCmdline,      // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		CREATE_NEW_CONSOLE,    // No creation flags
		NULL,           // Use parent's environment block
		currdir,        // Use parent's starting directory
		&si,            // Pointer to STARTUPINFO structure
		&pi )           // Pointer to PROCESS_INFORMATION structure
	)
	{
		ERROR_MSG(boost::format("Machine::startserver:CreateProcess failed (%1%).\n") %
			GetLastError());

		success = false;
	}
	
	free(szCmdline);
	free(currdir);
#else
#endif
	
	bundle << success;
	bundle.sendto(ep, recvport, recvip);
}

//-------------------------------------------------------------------------------------
void Machine::stopserver(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	int32 uid = 0;
	COMPONENT_TYPE componentType;
	uint32 recvip;
	uint16 recvport;
	Mercury::Bundle bundle;
	bool success = true;

	s >> uid;
	s >> componentType;
	s >> recvip;
	s >> recvport;

	Mercury::EndPoint ep;
	ep.socket(SOCK_DGRAM);

	if (!ep.good())
	{
		ERROR_MSG("Machine::stopserver: Failed to create socket.\n");
		return;
	}
	
	Components::COMPONENTS& components = Componentbridge::getComponents().getComponents(componentType);
	Components::COMPONENTS::iterator iter = components.begin();

	for(; iter != components.end(); )
	{
		Components::ComponentInfos* cinfos = &(*iter);

		if(cinfos->uid != uid)
			continue;

		if(componentType != cinfos->componentType)
		{
			continue;
		}

		bool usable = Componentbridge::getComponents().checkComponentUsable(&(*iter));
		
		if(!usable)
		{
			success = true;
			break;
		}

		Mercury::Bundle closebundle;
		COMMON_MERCURY_MESSAGE(componentType, closebundle, reqCloseServer);

		Mercury::EndPoint ep1;
		ep1.socket(SOCK_STREAM);

		if (!ep1.good())
		{
			ERROR_MSG("Machine::stopserver: Failed to create socket.\n");
			success = false;
			break;
		}
		
		if(ep1.connect((*iter).pIntAddr.get()->port, (*iter).pIntAddr.get()->ip) == -1)
		{
			ERROR_MSG(boost::format("Machine::stopserver: connect server is error(%1%)!\n") % kbe_strerror());
			success = false;
			break;
		}

		closebundle.send(ep1);
		Mercury::TCPPacket recvpacket;
		recvpacket.resize(255);
		int len = ep1.recv(recvpacket.data(), 1);
		if(len != 1)
		{
			success = false;
			break;
		}

		recvpacket >> success;
		break;
	}

	bundle << success;
	bundle.sendto(ep, recvport, recvip);
}


//-------------------------------------------------------------------------------------

}
