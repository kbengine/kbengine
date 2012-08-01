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


#include "tcp_packet.hpp"
#ifndef CODE_INLINE
#include "tcp_packet.ipp"
#endif
#include "network/bundle.hpp"
#include "network/endpoint.hpp"
#include "network/network_interface.hpp"
#include "network/message_handler.hpp"

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
TCPPacket::TCPPacket(MessageID msgID, size_t res):
Packet(msgID, res)
{
	data_resize(PACKET_MAX_SIZE_TCP);
	wpos(0);
}

//-------------------------------------------------------------------------------------
TCPPacket::~TCPPacket(void)
{
}

//-------------------------------------------------------------------------------------
int TCPPacket::recvFromEndPoint(EndPoint & ep, Address* pAddr)
{
	//KBE_ASSERT(MessageHandlers::pMainMessageHandlers != NULL && "Must set up a MainMessageHandlers!\n");

	int len = 0;
	size_t newchunksize = (PACKET_MAX_SIZE_TCP * 2);
	int i = size() / newchunksize;
	
	while(true)
	{
		len = ep.recv(data() + wpos(), PACKET_MAX_SIZE_TCP);
		if(len >= 0)
			wpos(wpos() + len);
		
		DEBUG_MSG("TCPPacket::recvFromEndPoint: datasize=%d.\n", len);
		if(len <= 0 || len != PACKET_MAX_SIZE_TCP)
			break;
		
		size_t resize = (++i + 1) * newchunksize;
		if(size() < resize)
			data_resize(resize);
	};
	
	return len;
}

//-------------------------------------------------------------------------------------
}
}
