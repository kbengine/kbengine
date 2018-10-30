// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "packet_filter.h"

#ifndef CODE_INLINE
#include "packet_filter.inl"
#endif

#include "network/channel.h"
#include "network/network_interface.h"
#include "network/packet_receiver.h"
#include "network/packet_sender.h"

namespace KBEngine { 
namespace Network
{
//-------------------------------------------------------------------------------------
Reason PacketFilter::send(Channel * pChannel, PacketSender& sender, Packet * pPacket, int userarg)
{
	return sender.processFilterPacket(pChannel, pPacket, userarg);
}

//-------------------------------------------------------------------------------------
Reason PacketFilter::recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket)
{
	return receiver.processFilteredPacket(pChannel, pPacket);
}

//-------------------------------------------------------------------------------------
} 
}
