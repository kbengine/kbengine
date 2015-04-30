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

	virtual PacketReader::PACKET_READER_TYPE type()const { return PACKET_READER_TYPE_WEBSOCKET; }

protected:
};


}
}
#endif 
