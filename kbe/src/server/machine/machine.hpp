/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __MACHINE_H__
#define __MACHINE_H__
	
// common include	
#include "server/kbemain.hpp"
#include "server/serverapp.hpp"
#include "server/idallocate.hpp"
#include "server/serverconfig.hpp"
#include "cstdkbe/timer.hpp"
#include "network/endpoint.hpp"
#include "network/udp_packet_receiver.hpp"
#include "network/common.hpp"

//#define NDEBUG
#include <map>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class Machine:	public ServerApp, 
				public TimerHandler, 
				public Singleton<Machine>
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK,
		TIMEOUT_LOADING_TICK
	};
	
	Machine(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, COMPONENT_TYPE componentType);
	~Machine();
	
	bool run();
	
	bool findBroadcastInterface();

	void onBroadcastInterface(int32 uid, std::string& username, 
							int8 componentType, int32 componentID, 
							uint32 addr, uint16 port);

	void handleTimeout(TimerHandle handle, void * arg);

	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	bool initNetwork();
protected:
	// udpπ„≤•µÿ÷∑
	u_int32_t broadcastAddr_;
	Mercury::EndPoint ep_;
	Mercury::EndPoint epBroadcast_;

	Mercury::EndPoint epLocal_;

	Mercury::UDPPacketReceiver* pEPPacketReceiver_;
	Mercury::UDPPacketReceiver* pEBPacketReceiver_;
	Mercury::UDPPacketReceiver* pEPLocalPacketReceiver_;
};

}
#endif
