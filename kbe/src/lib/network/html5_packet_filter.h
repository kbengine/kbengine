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


#ifndef KBE_HTML5_PACKET_FILTER_HPP
#define KBE_HTML5_PACKET_FILTER_HPP

#include "network/packet_filter.h"

namespace KBEngine { 
namespace Network
{
class TCPPacket;

class HTML5PacketFilter : public PacketFilter
{
public:
	enum WebSocketFrameType 
	{
		NEXT_FRAME = 0x0,
		END_FRAME = 0x80,

		BEGIN_TEXT_FRAME = 0x01,
		BEGIN_BINARY_FRAME = 0x02,

		TEXT_FRAME = 0x81,
		BINARY_FRAME = 0x82,

		PING_FRAME = 0x19,
		PONG_FRAME = 0x1A
	};

	HTML5PacketFilter(Channel* pChannel);
	virtual ~HTML5PacketFilter();

	virtual Reason send(NetworkInterface & networkInterface, Channel * pChannel, Packet * pPacket);

	virtual Reason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);

protected:
	enum FragmentDataTypes
	{
		FRAGMENT_DATA_UNKNOW,
		FRAGMENT_DATA_BASIC_LENGTH,
		FRAGMENT_DATA_PAYLOAD_LENGTH,
		FRAGMENT_DATA_PAYLOAD_MASKS,
		FRAGMENT_DATA_MESSAGE_BODY
	};

	uint32						web_pFragmentDatasRemain_;
	FragmentDataTypes			web_fragmentDatasFlag_;

	uint8						basicSize_;
	uint64						payloadSize_, payloadTotalSize_;
	uint8						masks_[16];
	std::pair<uint8, uint8>		data_xy_;
	
	Channel* pChannel_;

	TCPPacket*					pTCPPacket_;
};


}
}

#endif // KBE_HTML5_PACKET_FILTER_HPP
