// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_PACKET_READER_H
#define KBE_PACKET_READER_H

#include "common/memorystream.h"
#include "helper/debug_helper.h"
#include "network/common.h"

namespace KBEngine{
namespace Network
{
class Channel;
class MessageHandlers;

class PacketReader
{
public:
	enum PACKET_READER_TYPE
	{
		PACKET_READER_TYPE_SOCKET = 0,
		PACKET_READER_TYPE_WEBSOCKET = 1,
		PACKET_READER_TYPE_KCP = 2
	};

	PacketReader(Channel* pChannel);
	virtual ~PacketReader();

	virtual void reset();
	
	virtual void processMessages(KBEngine::Network::MessageHandlers* pMsgHandlers, Packet* pPacket);
	
	Network::MessageID	currMsgID() const{return currMsgID_;}
	Network::MessageLength	currMsgLen() const{return currMsgLen_;}
	
	void currMsgID(Network::MessageID id){currMsgID_ = id;}
	void currMsgLen(Network::MessageLength len){currMsgLen_ = len;}

	virtual PacketReader::PACKET_READER_TYPE type()const { return PACKET_READER_TYPE_SOCKET; }


protected:
	enum FragmentDataTypes
	{
		FRAGMENT_DATA_UNKNOW,
		FRAGMENT_DATA_MESSAGE_ID,
		FRAGMENT_DATA_MESSAGE_LENGTH,
		FRAGMENT_DATA_MESSAGE_LENGTH1,
		FRAGMENT_DATA_MESSAGE_BODY
	};
	
	virtual void writeFragmentMessage(FragmentDataTypes fragmentDatasFlag, Packet* pPacket, uint32 datasize);
	virtual void mergeFragmentMessage(Packet* pPacket);

protected:
	uint8*						pFragmentDatas_;
	uint32						pFragmentDatasWpos_;
	uint32						pFragmentDatasRemain_;
	FragmentDataTypes			fragmentDatasFlag_;
	MemoryStream*				pFragmentStream_;

	Network::MessageID			currMsgID_;
	Network::MessageLength1		currMsgLen_;
	
	Channel*					pChannel_;
};


}
}
#endif 
