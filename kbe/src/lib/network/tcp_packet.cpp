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
static ObjectPool<TCPPacket> _g_objPool("TCPPacket");
ObjectPool<TCPPacket>& TCPPacket::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
void TCPPacket::destroyObjPool()
{
	DEBUG_MSG(fmt::format("TCPPacket::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
TCPPacket::SmartPoolObjectPtr TCPPacket::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<TCPPacket>(ObjPool().createObject(), _g_objPool));
}

//-------------------------------------------------------------------------------------
TCPPacket::TCPPacket(MessageID msgID, size_t res):
Packet(msgID, true, res)
{
	data_resize(maxBufferSize());
	wpos(0);
}

//-------------------------------------------------------------------------------------
TCPPacket::~TCPPacket(void)
{
}

//-------------------------------------------------------------------------------------
size_t TCPPacket::maxBufferSize()
{
	return PACKET_MAX_SIZE_TCP * 4;
}

//-------------------------------------------------------------------------------------
int TCPPacket::recvFromEndPoint(EndPoint & ep, Address* pAddr)
{
	//KBE_ASSERT(MessageHandlers::pMainMessageHandlers != NULL && "Must set up a MainMessageHandlers!\n");
	KBE_ASSERT(maxBufferSize() > wpos());
	int len = ep.recv(data() + wpos(), size() - wpos());
	wpos(wpos() + len);
	
	/*
	// ע��:�����ڴ���0��ʱ�����DEBUG_MSG���ᵼ��WSAGetLastError����0�Ӷ�������ѭ��
	if(len > 0) 
	{
		DEBUG_MSG(fmt::format("TCPPacket::recvFromEndPoint: datasize={}, wpos={}.\n", len, wpos()));
	}
	*/
	return len; 
}

//-------------------------------------------------------------------------------------
}
}
