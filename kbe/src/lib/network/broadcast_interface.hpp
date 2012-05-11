/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __BROADCAST_INTERFACE__
#define __BROADCAST_INTERFACE__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/tasks.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/interfaces.hpp"
#include "network/endpoint.hpp"
#include "network/udp_packet_receiver.hpp"

namespace KBEngine { 
namespace Mercury
{
class Address;
class NetworkInterface;
class EventDispatcher;
class UDPPacket;

class BroadcastInterface : public Task, public UDPPacketReceiver
{
public:
	BroadcastInterface(NetworkInterface & networkInterface);
	~BroadcastInterface();

private:
	virtual void process();
	EventDispatcher & dispatcher();
private:
	EndPoint epBroadcast_;
	NetworkInterface & networkInterface_;
};

}
}

#ifdef CODE_INLINE
#include "listener_receiver.ipp"
#endif
#endif // __NETWORKLISTENER_RECEIVER__