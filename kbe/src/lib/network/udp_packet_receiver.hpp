/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __NETWORKUDPPACKET_RECEIVER__
#define __NETWORKUDPPACKET_RECEIVER__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/interfaces.hpp"
#include "network/udp_packet.hpp"
#include "network/packet_receiver.hpp"

namespace KBEngine { 
namespace Mercury
{
class Socket;
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;

class UDPPacketReceiver : public PacketReceiver
{
public:
	UDPPacketReceiver(EndPoint & endpoint, NetworkInterface & networkInterface);
	~UDPPacketReceiver();

	Reason processFilteredPacket(Packet * p);

protected:
	bool processSocket(bool expectingPacket);
	bool checkSocketErrors(int len, bool expectingPacket);
protected:
	TCPPacketPtr pNextPacket_;
};

}
}

#ifdef CODE_INLINE
#include "udp_packet_receiver.ipp"
#endif
#endif // __NETWORKTCPPACKET_RECEIVER__