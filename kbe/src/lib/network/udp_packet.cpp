// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "udp_packet.h"
#ifndef CODE_INLINE
#include "udp_packet.inl"
#endif
#include "network/bundle.h"
#include "network/endpoint.h"
#include "network/network_interface.h"
#include "network/message_handler.h"

namespace KBEngine { 
namespace Network
{
//-------------------------------------------------------------------------------------
static ObjectPool<UDPPacket> _g_objPool("UDPPacket");
ObjectPool<UDPPacket>& UDPPacket::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
UDPPacket* UDPPacket::createPoolObject(const std::string& logPoint)
{
	return _g_objPool.createObject(logPoint);
}

//-------------------------------------------------------------------------------------
void UDPPacket::reclaimPoolObject(UDPPacket* obj)
{
	_g_objPool.reclaimObject(obj);
}

//-------------------------------------------------------------------------------------
void UDPPacket::destroyObjPool()
{
	DEBUG_MSG(fmt::format("UDPPacket::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
UDPPacket::SmartPoolObjectPtr UDPPacket::createSmartPoolObj(const std::string& logPoint)
{
	return SmartPoolObjectPtr(new SmartPoolObject<UDPPacket>(ObjPool().createObject(logPoint), _g_objPool));
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
void UDPPacket::onReclaimObject()
{
	Packet::onReclaimObject();
	data_resize(maxBufferSize());
}

//-------------------------------------------------------------------------------------
int UDPPacket::recvFromEndPoint(EndPoint & ep, Address* pAddr)
{
	KBE_ASSERT(maxBufferSize() > wpos());

	// 当接收来的大小大于接收缓冲区的时候，recvfrom返回-1
	int len = ep.recvfrom(data() + wpos(), size() - wpos(),
		(u_int16_t*)&pAddr->port, (u_int32_t*)&pAddr->ip);

	if(len > 0)
		wpos(wpos() + len);

	return len;
}

//-------------------------------------------------------------------------------------
}
}
