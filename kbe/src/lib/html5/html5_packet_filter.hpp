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


#ifndef __HTML5_PACKET_FILTER_HPP__
#define __HTML5_PACKET_FILTER_HPP__

#include "network/packet_filter.hpp"

namespace KBEngine { 
namespace Mercury
{
class TCPPacket;

class HTML5PacketFilter : public PacketFilter
{
public:
	HTML5PacketFilter(Channel* pChannel);
	virtual ~HTML5PacketFilter();

	virtual Reason send(NetworkInterface & networkInterface, Channel * pChannel, Packet * pPacket);

	virtual Reason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);

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
	uint64						payloadSize_, payloadTotalSize_;
	uint8						masks_[16];
	std::pair<uint8, uint8>		data_xy_;
	
	Channel* pChannel_;

	TCPPacket*					pTCPPacket_;
};


}
}

#endif // __HTML5_PACKET_FILTER_HPP__
