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
		PACKET_READER_TYPE_WEBSOCKET = 1
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
