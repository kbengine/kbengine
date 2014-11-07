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

#ifndef KBE_HTML5_PACKET_READER_HPP
#define KBE_HTML5_PACKET_READER_HPP

#include "network/packet_reader.hpp"

namespace KBEngine{
namespace Network
{

class HTML5PacketReader : public PacketReader
{
public:
	HTML5PacketReader(Channel* pChannel);
	virtual ~HTML5PacketReader();

	virtual void reset();
	virtual void processMessages(KBEngine::Network::MessageHandlers* pMsgHandlers, Packet* pPacket);
protected:
};


}
}
#endif 
