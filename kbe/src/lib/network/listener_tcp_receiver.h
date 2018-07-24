// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NETWORKTCPLISTENER_RECEIVER_H
#define KBE_NETWORKTCPLISTENER_RECEIVER_H

#include "common/common.h"
#include "common/timer.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/packet.h"
#include "network/channel.h"
#include "network/listener_receiver.h"

namespace KBEngine { 
namespace Network
{

class ListenerTcpReceiver : public ListenerReceiver
{
public:
	ListenerTcpReceiver(EndPoint & endpoint, Channel::Traits traits, NetworkInterface & networkInterface);
	virtual ~ListenerTcpReceiver();

protected:
	virtual int handleInputNotification(int fd);

protected:
};

}
}

#ifdef CODE_INLINE
#include "listener_tcp_receiver.inl"
#endif
#endif // KBE_NETWORKTCPLISTENER_RECEIVER_H
