// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_NETWORKKCPPACKET_RECEIVER_EX_H
#define KBE_NETWORKKCPPACKET_RECEIVER_EX_H

#include "network/kcp_packet_receiver.h"

namespace KBEngine { 

class ClientObject;

namespace Network
{

class KCPPacketReceiverEx : public KCPPacketReceiver
{
public:
	KCPPacketReceiverEx(EndPoint & endpoint, NetworkInterface & networkInterface, ClientObject* pClientObject);
	~KCPPacketReceiverEx();

	virtual Channel* getChannel();
	virtual Channel* findChannel(const Address& addr);

protected:
	virtual void onGetError(Channel* pChannel, const std::string& err);

	ClientObject* pClientObject_;
};
}
}

#endif // KBE_NETWORKKCPPACKET_RECEIVER_EX_H
