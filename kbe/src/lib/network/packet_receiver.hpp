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
#include "network/packet.hpp"

namespace KBEngine { 
namespace Mercury
{
class Socket;
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;

class PacketReceiver : public InputNotificationHandler
{
public:
	PacketReceiver(Socket & socket, NetworkInterface & networkInterface);
	~PacketReceiver();

	Reason processPacket(const Address & addr, Packet * p);
	Reason processFilteredPacket(const Address & addr, Packet * p);

private:
	virtual int handleInputNotification(int fd);
	bool processSocket(bool expectingPacket);
	bool checkSocketErrors(int len, bool expectingPacket);

	Reason processOrderedPacket(const Address & addr, Packet * p,
		Channel * pChannel);

	bool processPiggybacks(const Address & addr,
			Packet * p);
	EventDispatcher & dispatcher();
private:
	Socket & socket_;
	NetworkInterface & networkInterface_;
	PacketPtr pNextPacket_;
};

}
}

#ifdef CODE_INLINE
#include "packet_receiver.ipp"
#endif
#endif // __NETWORKPACKET_RECEIVER__