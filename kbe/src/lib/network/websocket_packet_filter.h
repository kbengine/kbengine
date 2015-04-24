/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


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

	virtual Reason send(Channel * pChannel, PacketSender& sender, Packet * pPacket);
	virtual Reason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);

protected:
	void resetFrame();

protected:
	enum FragmentDataTypes
	{
		FRAGMENT_DATA_UNKNOW,
		FRAGMENT_DATA_BASIC_LENGTH,
		FRAGMENT_DATA_PAYLOAD_LENGTH,
		FRAGMENT_DATA_PAYLOAD_MASKS,
		FRAGMENT_DATA_MESSAGE_BODY
	};

	uint32										web_pFragmentDatasRemain_;
	FragmentDataTypes							web_fragmentDatasFlag_;

	uint8										msg_opcode_;
	uint8										msg_fin_;
	uint8										msg_masked_;
	uint32										msg_mask_;
	int32										msg_length_field_;
	uint64										msg_payload_length_;
	websocket::WebSocketProtocol::FrameType		msg_frameType_;

	Channel*									pChannel_;

	// 解析出来的干净的数据包
	TCPPacket*									pTCPPacket_;
};


}
}

#endif // KBE_WEBSOCKET_PACKET_FILTER_H
