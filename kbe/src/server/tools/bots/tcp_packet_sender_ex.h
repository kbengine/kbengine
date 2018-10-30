// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_NETWORKTCPPACKET_SENDER_EX_H
#define KBE_NETWORKTCPPACKET_SENDER_EX_H

#include "network/tcp_packet_sender.h"

namespace KBEngine { 

class ClientObject;

namespace Network
{

class TCPPacketSenderEx : public TCPPacketSender
{
public:
	TCPPacketSenderEx(EndPoint & endpoint, NetworkInterface & networkInterface, ClientObject* pClientObject);
	~TCPPacketSenderEx();

	virtual Channel* getChannel();

protected:
	virtual void onGetError(Channel* pChannel, const std::string& err);

	ClientObject* pClientObject_;
};
}
}


#endif // KBE_NETWORKTCPPACKET_SENDER_EX_H
