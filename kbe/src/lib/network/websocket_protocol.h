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
#include "common/common.h"
#include "helper/debug_helper.h"

namespace KBEngine{

class MemoryStream;

namespace Network
{
	class Channel;
}

namespace websocket{


class WebSocketProtocol
{
public:
	/*
	 	  0                   1                   2                   3
	 	  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 +-+-+-+-+-------+-+-------------+-------------------------------+
		 |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
		 |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
		 |N|V|V|V|       |S|             |   (if payload len==126/127)   |
		 | |1|2|3|       |K|             |                               |
		 +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
		 |     Extended payload length continued, if payload len == 127  |
		 + - - - - - - - - - - - - - - - +-------------------------------+
		 |                               |Masking-key, if MASK set to 1  |
		 +-------------------------------+-------------------------------+
		 | Masking-key (continued)       |          Payload Data         |
		 +-------------------------------- - - - - - - - - - - - - - - - +
		 :                     Payload Data continued ...                :
		 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
		 |                     Payload Data continued ...                |
		 +---------------------------------------------------------------+
	 */

    enum opcode {
        continuation = 0x0,
        text = 0x1,
        binary = 0x2,
        rsv3 = 0x3,
        rsv4 = 0x4,
        rsv5 = 0x5,
        rsv6 = 0x6,
        rsv7 = 0x7,
        close = 0x8,
        ping = 0x9,
        pong = 0xA,
        control_rsvb = 0xB,
        control_rsvc = 0xC,
        control_rsvd = 0xD,
        control_rsve = 0xE,
        control_rsvf = 0xF,

        CONTINUATION = 0x0,
        TEXT = 0x1,
        BINARY = 0x2,
        RSV3 = 0x3,
        RSV4 = 0x4,
        RSV5 = 0x5,
        RSV6 = 0x6,
        RSV7 = 0x7,
        CLOSE = 0x8,
        PING = 0x9,
        PONG = 0xA,
        CONTROL_RSVB = 0xB,
        CONTROL_RSVC = 0xC,
        CONTROL_RSVD = 0xD,
        CONTROL_RSVE = 0xE,
        CONTROL_RSVF = 0xF
    };

	// masks for fields in the basic header
	static uint8_t const BHB0_OPCODE = 0x0F;
	static uint8_t const BHB0_RSV3 = 0x10;
	static uint8_t const BHB0_RSV2 = 0x20;
	static uint8_t const BHB0_RSV1 = 0x40;
	static uint8_t const BHB0_FIN = 0x80;

	static uint8_t const BHB1_PAYLOAD = 0x7F;
	static uint8_t const BHB1_MASK = 0x80;

	static uint8_t const payload_size_code_16bit = 0x7E; // 126
	static uint8_t const payload_size_code_64bit = 0x7F; // 127

	/**
		是否是websocket协议
	*/
	static bool isWebSocketProtocol(MemoryStream* s);
	
	/**
		websocket协议握手
	*/
	static bool handshake(Network::Channel* pChannel, MemoryStream* s);
};

}
}

