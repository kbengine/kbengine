// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "kcp_packet_reader.h"
#include "network/channel.h"
#include "network/message_handler.h"
#include "network/network_stats.h"

namespace KBEngine { 
namespace Network
{


//-------------------------------------------------------------------------------------
KCPPacketReader::KCPPacketReader(Channel* pChannel):
	PacketReader(pChannel)
{
}

//-------------------------------------------------------------------------------------
KCPPacketReader::~KCPPacketReader()
{
}

//-------------------------------------------------------------------------------------
void KCPPacketReader::reset()
{
	PacketReader::reset();
}

//-------------------------------------------------------------------------------------
void KCPPacketReader::processMessages(KBEngine::Network::MessageHandlers* pMsgHandlers, Packet* pPacket)
{
	PacketReader::processMessages(pMsgHandlers, pPacket);
}

//-------------------------------------------------------------------------------------
} 
}
