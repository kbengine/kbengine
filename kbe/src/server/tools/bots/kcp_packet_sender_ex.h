// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_NETWORKKCPPACKET_SENDER_EX_H
#define KBE_NETWORKKCPPACKET_SENDER_EX_H

#include "network/kcp_packet_sender.h"

namespace KBEngine { 

class ClientObject;

namespace Network
{

class KCPPacketSenderEx : public KCPPacketSender
{
public:
	KCPPacketSenderEx(EndPoint & endpoint, NetworkInterface & networkInterface, ClientObject* pClientObject);
	~KCPPacketSenderEx();

	virtual Channel* getChannel();

protected:
	virtual void onGetError(Channel* pChannel, const std::string& err);
	virtual void onSent(Packet* pPacket);

	ClientObject* pClientObject_;
};
}
}


#endif // KBE_NETWORKKCPPACKET_SENDER_EX_H
