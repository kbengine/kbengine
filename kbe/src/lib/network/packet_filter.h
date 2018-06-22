// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_PACKET_FILTER_H
#define KBE_PACKET_FILTER_H

#include "network/common.h"
#include "common/smartpointer.h"
#include "common/refcountable.h"

namespace KBEngine { 
namespace Network
{

class Channel;
class NetworkInterface;
class Packet;
class Address;
class PacketFilter;
class PacketReceiver;
class PacketSender;

class PacketFilter : public RefCountable
{
public:
	virtual ~PacketFilter() {}

	virtual Reason send(Channel * pChannel, PacketSender& sender, Packet * pPacket, int userarg);

	virtual Reason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);
};

typedef SmartPointer<PacketFilter> PacketFilterPtr;

}
}

#ifdef CODE_INLINE
#include "packet_filter.inl"
#endif

#endif // KBE_PACKET_FILTER_H
