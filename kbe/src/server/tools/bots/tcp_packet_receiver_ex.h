// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_NETWORKTCPPACKET_RECEIVER_EX_H
#define KBE_NETWORKTCPPACKET_RECEIVER_EX_H

#include "network/tcp_packet_receiver.h"

namespace KBEngine { 

class ClientObject;

namespace Network
{

class TCPPacketReceiverEx : public TCPPacketReceiver
{
public:
	TCPPacketReceiverEx(EndPoint & endpoint, NetworkInterface & networkInterface, ClientObject* pClientObject);
	~TCPPacketReceiverEx();

	virtual Channel* getChannel();

protected:
	virtual void onGetError(Channel* pChannel, const std::string& err);

	ClientObject* pClientObject_;
};
}
}

#endif // KBE_NETWORKTCPPACKET_RECEIVER_EX_H
