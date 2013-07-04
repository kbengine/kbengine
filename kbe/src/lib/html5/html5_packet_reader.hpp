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

#ifndef __HTML5_PACKET_READER_HPP__
#define __HTML5_PACKET_READER_HPP__

#include "network/packet_reader.hpp"

namespace KBEngine{
namespace Mercury
{

class HTML5PacketReader : public PacketReader
{
public:
	HTML5PacketReader(Channel* pChannel);
	virtual ~HTML5PacketReader();

	virtual void reset();
	virtual void processMessages(KBEngine::Mercury::MessageHandlers* pMsgHandlers, Packet* pPacket);
protected:
	enum FragmentDataTypes
	{
		FRAGMENT_DATA_UNKNOW,
		FRAGMENT_DATA_BASIC_LENGTH,
		FRAGMENT_DATA_PAYLOAD_LENGTH,
		FRAGMENT_DATA_PAYLOAD_MASKS,
		FRAGMENT_DATA_MESSAGE_BODY
	};

	uint32						web_pFragmentDatasRemain_;
	FragmentDataTypes			web_fragmentDatasFlag_;

	uint8						basicSize_;
	uint64						payloadSize_;
	uint8						masks_[16];
	std::pair<uint8, uint8>		data_xy_;
};


}
}
#endif 
