// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_WEBSOCKET_PACKET_FILTER_H
#define KBE_WEBSOCKET_PACKET_FILTER_H

#include "network/packet_filter.h"
#include "network/websocket_protocol.h"

namespace KBEngine { 
namespace Network
{
class TCPPacket;

class WebSocketPacketFilter : public PacketFilter
{
public:
	WebSocketPacketFilter(Channel* pChannel);
	virtual ~WebSocketPacketFilter();

	virtual Reason send(Channel * pChannel, PacketSender& sender, Packet * pPacket, int userarg);
	virtual Reason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);

protected:
	void reset();
	Reason onPing(Channel * pChannel, Packet* pPacket);

protected:
	enum FragmentDataTypes
	{
		FRAGMENT_MESSAGE_HREAD,
		FRAGMENT_MESSAGE_DATAS
	};

	int32										pFragmentDatasRemain_;
	FragmentDataTypes							fragmentDatasFlag_;

	uint8										msg_opcode_;
	uint8										msg_fin_;
	uint8										msg_masked_;
	uint32										msg_mask_;
	int32										msg_length_field_;
	uint64										msg_payload_length_;
	websocket::WebSocketProtocol::FrameType		msg_frameType_;

	Channel*									pChannel_;

	TCPPacket*									pTCPPacket_;
};


}
}

#endif // KBE_WEBSOCKET_PACKET_FILTER_H
