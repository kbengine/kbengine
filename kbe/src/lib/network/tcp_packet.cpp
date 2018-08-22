// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "tcp_packet.h"
#ifndef CODE_INLINE
#include "tcp_packet.inl"
#endif
#include "network/bundle.h"
#include "network/endpoint.h"
#include "network/network_interface.h"
#include "network/message_handler.h"

namespace KBEngine { 
namespace Network
{
//-------------------------------------------------------------------------------------
static ObjectPool<TCPPacket> _g_objPool("TCPPacket");
ObjectPool<TCPPacket>& TCPPacket::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
TCPPacket* TCPPacket::createPoolObject(const std::string& logPoint)
{
	return _g_objPool.createObject(logPoint);
}

//-------------------------------------------------------------------------------------
void TCPPacket::reclaimPoolObject(TCPPacket* obj)
{
	_g_objPool.reclaimObject(obj);
}

//-------------------------------------------------------------------------------------
void TCPPacket::destroyObjPool()
{
	DEBUG_MSG(fmt::format("TCPPacket::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
TCPPacket::SmartPoolObjectPtr TCPPacket::createSmartPoolObj(const std::string& logPoint)
{
	return SmartPoolObjectPtr(new SmartPoolObject<TCPPacket>(ObjPool().createObject(logPoint), _g_objPool));
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
	return PACKET_MAX_SIZE_TCP;
}

//-------------------------------------------------------------------------------------
void TCPPacket::onReclaimObject()
{
	Packet::onReclaimObject();
	data_resize(maxBufferSize());
}

//-------------------------------------------------------------------------------------
int TCPPacket::recvFromEndPoint(EndPoint & ep, Address* pAddr)
{
	//KBE_ASSERT(MessageHandlers::pMainMessageHandlers != NULL && "Must set up a MainMessageHandlers!\n");
	KBE_ASSERT(maxBufferSize() > wpos());
	int len = ep.recv(data() + wpos(), (int)(size() - wpos()));
	
	if(len > 0) 
	{
		wpos((int)(wpos() + len));

		// 注意:必须在大于0的时候否则DEBUG_MSG将会导致WSAGetLastError返回0从而陷入死循环
		// DEBUG_MSG(fmt::format("TCPPacket::recvFromEndPoint: datasize={}, wpos={}.\n", len, wpos()));
	}

	return len; 
}

//-------------------------------------------------------------------------------------
}
}
