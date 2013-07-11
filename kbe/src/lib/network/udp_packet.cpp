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


#include "udp_packet.hpp"
#ifndef CODE_INLINE
#include "udp_packet.ipp"
#endif
#include "network/bundle.hpp"
#include "network/endpoint.hpp"
#include "network/network_interface.hpp"
#include "network/message_handler.hpp"

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
static ObjectPool<UDPPacket> _g_objPool;
ObjectPool<UDPPacket>& UDPPacket::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
void UDPPacket::destroyObjPool()
{
	DEBUG_MSG(boost::format("UDPPacket::destroyObjPool(): size %1%.\n") % 
		_g_objPool.size());

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
UDPPacket::SmartPoolObjectPtr UDPPacket::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<UDPPacket>(ObjPool().createObject(), _g_objPool));
}

//-------------------------------------------------------------------------------------
UDPPacket::UDPPacket(MessageID msgID, size_t res):
Packet(msgID, false, res)
{
	data_resize(maxBufferSize());
	wpos(0);
}

//-------------------------------------------------------------------------------------
UDPPacket::~UDPPacket(void)
{
}

//-------------------------------------------------------------------------------------
size_t UDPPacket::maxBufferSize()
{
	return PACKET_MAX_SIZE_UDP;
}

//-------------------------------------------------------------------------------------
int UDPPacket::recvFromEndPoint(EndPoint & ep, Address* pAddr)
{
	KBE_ASSERT(maxBufferSize() > wpos());
	int len = ep.recvfrom(data() + wpos(), size() - wpos(),
		(u_int16_t*)&pAddr->port, (u_int32_t*)&pAddr->ip);

	wpos(wpos() + len);
	return len;
}

//-------------------------------------------------------------------------------------
}
}
