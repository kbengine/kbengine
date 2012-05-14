/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __BROADCAST_HANDLER__
#define __BROADCAST_HANDLER__

#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/interfaces.hpp"
#include "message_handler.hpp"
#include "network/bundle.hpp"
#include "network/endpoint.hpp"

namespace KBEngine { 
namespace Mercury
{
class NetworkInterface;

/*
	可以方便的处理如:向局域网内广播某些信息， 并处理收集相关信息。
*/
class BroadcastHandler : public Bundle
{
public:
	BroadcastHandler(NetworkInterface & networkInterface, uint16 bindPort = 0, 
		uint32 recvWindowSize = 4096);
	virtual ~BroadcastHandler();

	EventDispatcher& dispatcher();
	
	bool broadcast(uint16 port = 0);
	bool receive(MessageArgs* recvArgs, sockaddr_in* psin = NULL);

	Mercury::EndPoint& epListen() { return epListen_; }
protected:
	Mercury::EndPoint epListen_;
	NetworkInterface & networkInterface_;
	uint32 recvWindowSize_;
};

}
}

#ifdef CODE_INLINE
#include "broadcast_handler.ipp"
#endif
#endif // __NETWORKPACKET_RECEIVER__