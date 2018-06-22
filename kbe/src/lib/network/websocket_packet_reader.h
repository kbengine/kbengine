// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_WEBSOCKET_PACKET_READER_H
#define KBE_WEBSOCKET_PACKET_READER_H

#include "network/packet_reader.h"

namespace KBEngine{
namespace Network
{

class WebSocketPacketReader : public PacketReader
{
public:
	WebSocketPacketReader(Channel* pChannel);
	virtual ~WebSocketPacketReader();

	virtual void reset();
	virtual void processMessages(KBEngine::Network::MessageHandlers* pMsgHandlers, Packet* pPacket);

	virtual PacketReader::PACKET_READER_TYPE type() const { return PACKET_READER_TYPE_WEBSOCKET; }

protected:
};


}
}
#endif 
