/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#include "websocket_protocol.h"
#include "common/memorystream.h"
#include "common/memorystream_converter.h"
#include "network/channel.h"
#include "network/packet.h"
#include "common/base64.h"
#include "common/sha1.h"

#if KBE_PLATFORM == PLATFORM_WIN32
#ifdef _DEBUG
#pragma comment(lib, "libeay32_d.lib")
#pragma comment(lib, "ssleay32_d.lib")
#else
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif
#endif

namespace KBEngine{
namespace Network{
namespace websocket{

//-------------------------------------------------------------------------------------
bool WebSocketProtocol::isWebSocketProtocol(MemoryStream* s)
{
	KBE_ASSERT(s != NULL);

	std::string data;
	size_t rpos = s->rpos();
	size_t wpos = s->wpos();

	(*s) >> data;

	size_t fi = data.find_first_of("Sec-WebSocket-Key");
	if(fi == std::string::npos)
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	fi = data.find_first_of("GET");
	if(fi == std::string::npos)
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	std::vector<std::string> header_and_data;
	header_and_data = KBEngine::strutil::kbe_splits(data, "\r\n\r\n");
	
	if(header_and_data.size() != 2)
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	s->rpos(rpos);
	s->wpos(wpos);
	return true;
}

//-------------------------------------------------------------------------------------
bool WebSocketProtocol::handshake(Network::Channel* pChannel, MemoryStream* s)
{
	KBE_ASSERT(s != NULL);

	std::string data;
	size_t rpos = s->rpos();
	size_t wpos = s->wpos();

	(*s) >> data;

	std::vector<std::string> header_and_data;
	header_and_data = KBEngine::strutil::kbe_splits(data, "\r\n\r\n");
	
	if(header_and_data.size() != 2)
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	KBEUnordered_map<std::string, std::string> headers;
	std::vector<std::string> values;
	
	values = KBEngine::strutil::kbe_splits(header_and_data[0], "\r\n");
	std::vector<std::string>::iterator iter = values.begin();

	for(; iter != values.end(); ++iter)
	{
		header_and_data = KBEngine::strutil::kbe_splits((*iter), ": ");

		if(header_and_data.size() == 2)
			headers[header_and_data[0]] = header_and_data[1];
	}

	std::string szKey, szOrigin, szHost;

	KBEUnordered_map<std::string, std::string>::iterator findIter = headers.find("Sec-WebSocket-Origin");
	if(findIter == headers.end())
	{
		findIter = headers.find("Origin");
		if(findIter == headers.end())
		{
			//��Щapp���ͻ��˿���û������ֶ�
			//s->rpos(rpos);
			//s->wpos(wpos);
			//return false;
		}
	}

	if (findIter != headers.end())
		szOrigin = fmt::format("WebSocket-Origin: {}\r\n", findIter->second);

	findIter = headers.find("Sec-WebSocket-Key");
	if(findIter == headers.end())
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	szKey = findIter->second;

	findIter = headers.find("Host");
	if(findIter == headers.end())
	{
		s->rpos(rpos);
		s->wpos(wpos);
		return false;
	}

	szHost = findIter->second;


    std::string server_key = szKey;

	//RFC6544_MAGIC_KEY
    server_key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	SHA1 sha;
	unsigned int message_digest[5];

	sha.Reset();
	sha << server_key.c_str();
	sha.Result(message_digest);

	for (int i = 0; i < 5; ++i)
		message_digest[i] = htonl(message_digest[i]);

    server_key = base64_encode(reinterpret_cast<const unsigned char*>(message_digest), 20);

	std::string ackHandshake = fmt::format("HTTP/1.1 101 Switching Protocols\r\n"
								"Upgrade: websocket\r\n"
								"Connection: Upgrade\r\n"
								"Sec-WebSocket-Accept: {}\r\n"
								"{}"
								"WebSocket-Location: ws://{}/WebManagerSocket\r\n"
								"WebSocket-Protocol: WebManagerSocket\r\n\r\n", 
								server_key, szOrigin, szHost);

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle) << ackHandshake;
	(*pBundle).pCurrPacket()->wpos((*pBundle).pCurrPacket()->wpos() - 1);
	pChannel->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
int WebSocketProtocol::makeFrame(WebSocketProtocol::FrameType frame_type, 
	Packet * pInPacket, Packet * pOutPacket)
{
	uint64 size = pInPacket->length(); 

	// д��frame����
	(*pOutPacket) << ((uint8)frame_type); 

	if(size <= 125)
	{
		(*pOutPacket) << ((uint8)size);
	}
	else if (size <= 65535)
	{
		uint8 bytelength = 126;
		(*pOutPacket) << bytelength; 

		(*pOutPacket) << ((uint8)(( size >> 8 ) & 0xff));
		(*pOutPacket) << ((uint8)(( size ) & 0xff));
	}
	else
	{
		uint8 bytelength = 127;
		(*pOutPacket) << bytelength; 

		MemoryStreamConverter::apply<uint64>(&size);
		(*pOutPacket) << size;
	}

	return pOutPacket->length();
}

//-------------------------------------------------------------------------------------
int WebSocketProtocol::getFrame(Packet * pPacket, uint8& msg_opcode, uint8& msg_fin, uint8& msg_masked, uint32& msg_mask, 
		int32& msg_length_field, uint64& msg_payload_length, FrameType& frameType)
{
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

	// ����3�ֽڣ���Ҫ�����ȴ�
	if(pPacket->length() < 3) 
	{
		frameType = INCOMPLETE_FRAME;
		return 3;
	}

	// ��һ���ֽ�, ���λ����������Ϣ�Ƿ����, ���4λ����������Ϣ����
	uint8 bytedata;
	(*pPacket) >> bytedata;

	msg_opcode = bytedata & 0x0F;
	msg_fin = (bytedata >> 7) & 0x01;

	// �ڶ����ֽ�, ��Ϣ�ĵڶ����ֽ���Ҫ���������������Ϣ����, ���λ��0��1�������Ƿ������봦��
	(*pPacket) >> bytedata;
	msg_masked = (bytedata >> 7) & 0x01;

	// ��Ϣ����
	msg_length_field = bytedata & (~0x80);

	// ʣ�µĺ���7λ����������Ϣ����, ����7λ���ֻ������127�������ֵ������������
	// һ������Ϣ��������126�洢��Ϣ����, �����Ϣ��������UINT16�������ֵΪ126
	// ����Ϣ���ȴ���UINT16������´�ֵΪ127;
	// �������������Ϣ���ȴ洢����������byte[], �ֱ���UINT16(2λbyte)��UINT64(4λbyte)
	if(msg_length_field <= 125) 
	{
		msg_payload_length = msg_length_field;
	}
	else if(msg_length_field == 126) 
	{ 
		uint8 bytedata1, bytedata2;
		(*pPacket) >> bytedata1 >> bytedata2;
		msg_payload_length = (bytedata1 << 8) | bytedata2;
	}
	else if(msg_length_field == 127) 
	{ 
		msg_payload_length = ((uint64)(pPacket->data() + pPacket->rpos() + 0) << 56) |
                         ((uint64)(pPacket->data() + pPacket->rpos() + 1) << 48) |
                         ((uint64)(pPacket->data() + pPacket->rpos() + 2) << 40) |
                         ((uint64)(pPacket->data() + pPacket->rpos() + 3) << 32) |
                         ((uint64)(pPacket->data() + pPacket->rpos() + 4) << 24) |
                         ((uint64)(pPacket->data() + pPacket->rpos() + 5) << 16) |
                         ((uint64)(pPacket->data() + pPacket->rpos() + 6) << 8) |
                         ((uint64)(pPacket->data() + pPacket->rpos() + 7));

		pPacket->read_skip(8);
	}

	// ����ɶ����Ȳ���
	/* ���ﲻ����飬ֻ����Э��ͷ
	if(pPacket->length() < (size_t)msg_payload_length) {
		frameType = INCOMPLETE_FRAME;
		return (size_t)msg_payload_length - pPacket->length();
	}
	*/

	// ����������������»�ȡ4�ֽ�����ֵ
	if(msg_masked) 
	{
		(*pPacket) >> msg_mask;
	}
	
	if(NETWORK_MESSAGE_MAX_SIZE < msg_payload_length)
	{
		WARNING_MSG(fmt::format("WebSocketProtocol::getFrame: msglen exceeds the limit! msglen=({}), maxlen={}.\n", 
			msg_payload_length, NETWORK_MESSAGE_MAX_SIZE));

		frameType = ERROR_FRAME;
		return 0;
	}

	if(msg_opcode == 0x0) frameType = (msg_fin) ? BINARY_FRAME : INCOMPLETE_BINARY_FRAME; // continuation frame ?
	else if(msg_opcode == 0x1) frameType = (msg_fin) ? TEXT_FRAME : INCOMPLETE_TEXT_FRAME;
	else if(msg_opcode == 0x2) frameType = (msg_fin) ? BINARY_FRAME : INCOMPLETE_BINARY_FRAME;
	else if(msg_opcode == 0x8) frameType = CLOSE_FRAME;
	else if(msg_opcode == 0x9) frameType = PING_FRAME;
	else if(msg_opcode == 0xA) frameType = PONG_FRAME;
	else frameType = ERROR_FRAME;

	return 0;
}

//-------------------------------------------------------------------------------------
bool WebSocketProtocol::decodingDatas(Packet* pPacket, uint8 msg_masked, uint32 msg_mask)
{
	// ��������
	if(msg_masked) 
	{
		uint8* c = pPacket->data() + pPacket->rpos();
		for(int i=0; i<(int)pPacket->length(); i++) {
			c[i] = c[i] ^ ((uint8*)(&msg_mask))[i % 4];
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
}
}
}
