/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __NETWORKLISTENER_RECEIVER__
#define __NETWORKLISTENER_RECEIVER__

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

class ListenerReceiver : public InputNotificationHandler
{
public:
	ListenerReceiver(Socket & socket, NetworkInterface & networkInterface);
	~ListenerReceiver();

private:
	virtual int handleInputNotification(int fd);
	EventDispatcher & dispatcher();
private:
	Socket & socket_;
	NetworkInterface & networkInterface_;
};

}
}

#ifdef CODE_INLINE
#include "listener_receiver.ipp"
#endif
#endif // __NETWORKLISTENER_RECEIVER__