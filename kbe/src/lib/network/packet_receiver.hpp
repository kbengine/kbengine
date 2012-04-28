/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __NETWORKPACKET_RECEIVER__
#define __NETWORKPACKET_RECEIVER__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/interfaces.hpp"
#include "network/tcp_packet.hpp"

namespace KBEngine { 
namespace Mercury
{
class EndPoint;
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;

class PacketReceiver : public InputNotificationHandler
{
public:
	PacketReceiver(EndPoint & endpoint, NetworkInterface & networkInterface);
	virtual ~PacketReceiver();

	virtual Reason processPacket(Channel* pChannel, Packet * pPacket);
	virtual Reason processFilteredPacket(Channel* pChannel, Packet * pPacket) = 0;
	EventDispatcher& dispatcher();
protected:
	virtual int handleInputNotification(int fd);
	virtual bool processSocket(bool expectingPacket) = 0;
	virtual bool checkSocketErrors(int len, bool expectingPacket) = 0;
protected:
	EndPoint & endpoint_;
	NetworkInterface & networkInterface_;
};

}
}

#ifdef CODE_INLINE
#include "packet_receiver.ipp"
#endif
#endif // __NETWORKPACKET_RECEIVER__