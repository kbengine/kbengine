// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "websocket_packet_reader.h"
#include "network/channel.h"
#include "network/message_handler.h"
#include "network/network_stats.h"

namespace KBEngine { 
namespace Network
{


//-------------------------------------------------------------------------------------
WebSocketPacketReader::WebSocketPacketReader(Channel* pChannel):
	PacketReader(pChannel)
{
}

//-------------------------------------------------------------------------------------
WebSocketPacketReader::~WebSocketPacketReader()
{
}

//-------------------------------------------------------------------------------------
void WebSocketPacketReader::reset()
{
	PacketReader::reset();
}

//-------------------------------------------------------------------------------------
void WebSocketPacketReader::processMessages(KBEngine::Network::MessageHandlers* pMsgHandlers, Packet* pPacket)
{
	PacketReader::processMessages(pMsgHandlers, pPacket);
}

//-------------------------------------------------------------------------------------
} 
}
