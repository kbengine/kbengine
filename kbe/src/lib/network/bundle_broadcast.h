// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_BUNDLE_BROADCAST_H
#define KBE_BUNDLE_BROADCAST_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "message_handler.h"
#include "network/bundle.h"
#include "network/endpoint.h"

namespace KBEngine { 
namespace Network
{
class NetworkInterface;

/*
	可以方便的处理如:向局域网内广播某些信息， 并处理收集相关信息。
*/
class BundleBroadcast : public Bundle
{
public:
	BundleBroadcast(NetworkInterface & networkInterface, uint16 bindPort = KBE_PORT_BROADCAST_DISCOVERY, 
		uint32 recvWindowSize = PACKET_MAX_SIZE_UDP);
	virtual ~BundleBroadcast();

	EventDispatcher& dispatcher();
	
	bool broadcast(uint16 port = 0);
	bool receive(MessageArgs* recvArgs, sockaddr_in* psin = NULL, int32 timeout = 100000, bool showerr = true);

	Network::EndPoint& epListen() { 
		return epListen_; 
	}

	void close();

	bool good() const { 
		return epListen_.good() && good_; 
	}

	void itry(int8 i){ 
		itry_ = i; 
	}

	void addBroadCastAddress(std::string addr);

protected:
	Network::EndPoint epListen_, epBroadcast_;
	NetworkInterface & networkInterface_;
	uint32 recvWindowSize_;
	bool good_;
	int8 itry_;
	std::vector< std::string > machine_addresses_;
};

}
}

#ifdef CODE_INLINE
#include "bundle_broadcast.inl"
#endif
#endif // KBE_BUNDLE_BROADCAST_H
