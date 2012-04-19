/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __NETWORKPACKET_SENDER__
#define __NETWORKPACKET_SENDER__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/interfaces.hpp"

namespace KBEngine { 
namespace Mercury
{
class Packet;
class EndPoint;
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;

class PacketSender : public OutputNotificationHandler
{
public:
	PacketSender(EndPoint & endpoint, NetworkInterface & networkInterface);
	virtual ~PacketSender();

	EventDispatcher& dispatcher();
protected:
	virtual int handleOutputNotification(int fd);
protected:
	EndPoint & endpoint_;
	NetworkInterface & networkInterface_;
};

}
}

#ifdef CODE_INLINE
#include "packet_sender.ipp"
#endif
#endif // __NETWORKPACKET_RECEIVER__