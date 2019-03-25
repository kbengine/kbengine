// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_WEBSOCKET_PROTOCOL_H
#define KBE_WEBSOCKET_PROTOCOL_H

#include "common/common.h"
#include "helper/debug_helper.h"

namespace KBEngine{

class MemoryStream;

namespace Network
{
	class Channel;
	class Packet;

namespace websocket{


/*	WebSocket FRC6544
*/

class WebSocketProtocol
{
public:
	enum FrameType 
	{
		// ��һ֡�����
		NEXT_FRAME = 0x0,
		END_FRAME = 0x80,

		ERROR_FRAME = 0xFF00,
		INCOMPLETE_FRAME = 0xFE00,

		OPENING_FRAME = 0x3300,
		CLOSING_FRAME = 0x3400,

		// δ��ɵ�֡
		INCOMPLETE_TEXT_FRAME = 0x01,
		INCOMPLETE_BINARY_FRAME = 0x02,

		// �ı�֡�������֡ END_FRAME + *_FRAME
		TEXT_FRAME = 0x81,
		BINARY_FRAME = 0x82,

		// END_FRAME + *_FRAME
		PING_FRAME = 0x89,
		PONG_FRAME = 0x8A,

		// �ر�����
		CLOSE_FRAME = 0x08
	};

	/**
		�Ƿ���websocketЭ��
	*/
	static bool isWebSocketProtocol(MemoryStream* s);
	
	/**
		websocketЭ������
	*/
	static bool handshake(Network::Channel* pChannel, MemoryStream* s);

	/**
		֡�������
	*/
	static int makeFrame(FrameType frame_type, Packet* pInPacket, Packet* pOutPacket);
	static int getFrame(Packet* pPacket, uint8& msg_opcode, uint8& msg_fin, uint8& msg_masked, uint32& msg_mask, 
		int32& msg_length_field, uint64& msg_payload_length, FrameType& frameType);

	static bool decodingDatas(Packet* pPacket, uint8 msg_masked, uint32 msg_mask);

	static std::string getFrameTypeName(FrameType frame_type);

};

}
}
}

#endif // KBE_WEBSOCKET_PROTOCOL_H

