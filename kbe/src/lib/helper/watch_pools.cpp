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

#include "watcher.hpp"
#include "watch_pools.hpp"
#include "network/bundle.hpp"
#include "network/address.hpp"
#include "network/endpoint.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/tcp_packet_receiver.hpp"
#include "network/udp_packet_receiver.hpp"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
int32 watchBundlePool_size()
{
	return (int)Mercury::Bundle::ObjPool().objects().size();
}

int32 watchBundlePool_max()
{
	return (int)Mercury::Bundle::ObjPool().max();
}

int32 watchBundlePool_totalAlloc()
{
	return (int)Mercury::Bundle::ObjPool().totalAlloc();
}

bool watchBundlePool_isDestroyed()
{
	return Mercury::Bundle::ObjPool().isDestroyed();
}

uint32 watchBundlePool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Mercury::Bundle>::OBJECTS::const_iterator iter = Mercury::Bundle::ObjPool().objects().begin();
	for(; iter != Mercury::Bundle::ObjPool().objects().end(); iter++)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchAddressPool_size()
{
	return (int)Mercury::Address::ObjPool().objects().size();
}

int32 watchAddressPool_max()
{
	return (int)Mercury::Address::ObjPool().max();
}

int32 watchAddressPool_totalAlloc()
{
	return (int)Mercury::Address::ObjPool().totalAlloc();
}

bool watchAddressPool_isDestroyed()
{
	return Mercury::Address::ObjPool().isDestroyed();
}

uint32 watchAddressPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Mercury::Address>::OBJECTS::const_iterator iter = Mercury::Address::ObjPool().objects().begin();
	for(; iter != Mercury::Address::ObjPool().objects().end(); iter++)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchMemoryStreamPool_size()
{
	return (int)MemoryStream::ObjPool().objects().size();
}

int32 watchMemoryStreamPool_max()
{
	return (int)MemoryStream::ObjPool().max();
}

int32 watchMemoryStreamPool_totalAlloc()
{
	return (int)MemoryStream::ObjPool().totalAlloc();
}

bool watchMemoryStreamPool_isDestroyed()
{
	return MemoryStream::ObjPool().isDestroyed();
}

uint32 watchMemoryStreamPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<MemoryStream>::OBJECTS::const_iterator iter = MemoryStream::ObjPool().objects().begin();
	for(; iter != MemoryStream::ObjPool().objects().end(); iter++)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchTCPPacketPool_size()
{
	return (int)Mercury::TCPPacket::ObjPool().objects().size();
}

int32 watchTCPPacketPool_max()
{
	return (int)Mercury::TCPPacket::ObjPool().max();
}

int32 watchTCPPacketPool_totalAlloc()
{
	return (int)Mercury::TCPPacket::ObjPool().totalAlloc();
}

bool watchTCPPacketPool_isDestroyed()
{
	return Mercury::TCPPacket::ObjPool().isDestroyed();
}

uint32 watchTCPPacketPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Mercury::TCPPacket>::OBJECTS::const_iterator iter = Mercury::TCPPacket::ObjPool().objects().begin();
	for(; iter != Mercury::TCPPacket::ObjPool().objects().end(); iter++)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchTCPPacketReceiverPool_size()
{
	return (int)Mercury::TCPPacketReceiver::ObjPool().objects().size();
}

int32 watchTCPPacketReceiverPool_max()
{
	return (int)Mercury::TCPPacketReceiver::ObjPool().max();
}

int32 watchTCPPacketReceiverPool_totalAlloc()
{
	return (int)Mercury::TCPPacketReceiver::ObjPool().totalAlloc();
}

bool watchTCPPacketReceiverPool_isDestroyed()
{
	return Mercury::TCPPacketReceiver::ObjPool().isDestroyed();
}

uint32 watchTCPPacketReceiverPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Mercury::TCPPacketReceiver>::OBJECTS::const_iterator iter = Mercury::TCPPacketReceiver::ObjPool().objects().begin();
	for(; iter != Mercury::TCPPacketReceiver::ObjPool().objects().end(); iter++)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchUDPPacketPool_size()
{
	return (int)Mercury::UDPPacket::ObjPool().objects().size();
}

int32 watchUDPPacketPool_max()
{
	return (int)Mercury::UDPPacket::ObjPool().max();
}

int32 watchUDPPacketPool_totalAlloc()
{
	return (int)Mercury::UDPPacket::ObjPool().totalAlloc();
}

bool watchUDPPacketPool_isDestroyed()
{
	return Mercury::UDPPacket::ObjPool().isDestroyed();
}

uint32 watchUDPPacketPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Mercury::UDPPacket>::OBJECTS::const_iterator iter = Mercury::UDPPacket::ObjPool().objects().begin();
	for(; iter != Mercury::UDPPacket::ObjPool().objects().end(); iter++)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchUDPPacketReceiverPool_size()
{
	return (int)Mercury::UDPPacketReceiver::ObjPool().objects().size();
}

int32 watchUDPPacketReceiverPool_max()
{
	return (int)Mercury::UDPPacketReceiver::ObjPool().max();
}

int32 watchUDPPacketReceiverPool_totalAlloc()
{
	return (int)Mercury::UDPPacketReceiver::ObjPool().totalAlloc();
}

bool watchUDPPacketReceiverPool_isDestroyed()
{
	return Mercury::UDPPacketReceiver::ObjPool().isDestroyed();
}

uint32 watchUDPPacketReceiverPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Mercury::UDPPacketReceiver>::OBJECTS::const_iterator iter = Mercury::UDPPacketReceiver::ObjPool().objects().begin();
	for(; iter != Mercury::UDPPacketReceiver::ObjPool().objects().end(); iter++)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchEndPointPool_size()
{
	return (int)Mercury::EndPoint::ObjPool().objects().size();
}

int32 watchEndPointPool_max()
{
	return (int)Mercury::EndPoint::ObjPool().max();
}

int32 watchEndPointPool_totalAlloc()
{
	return (int)Mercury::EndPoint::ObjPool().totalAlloc();
}

bool watchEndPointPool_isDestroyed()
{
	return Mercury::EndPoint::ObjPool().isDestroyed();
}

uint32 watchEndPointPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Mercury::EndPoint>::OBJECTS::const_iterator iter = Mercury::EndPoint::ObjPool().objects().begin();
	for(; iter != Mercury::EndPoint::ObjPool().objects().end(); iter++)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
bool WatchPool::initWatchPools()
{
	WATCH_OBJECT("objectPools/Bundle/size", &watchBundlePool_size);
	WATCH_OBJECT("objectPools/Bundle/max", &watchBundlePool_max);
	WATCH_OBJECT("objectPools/Bundle/isDestroyed", &watchBundlePool_isDestroyed);
	WATCH_OBJECT("objectPools/Bundle/memory", &watchBundlePool_bytes);
	WATCH_OBJECT("objectPools/Bundle/totalAllocs", &watchBundlePool_totalAlloc);

	WATCH_OBJECT("objectPools/Address/size", &watchAddressPool_size);
	WATCH_OBJECT("objectPools/Address/max", &watchAddressPool_max);
	WATCH_OBJECT("objectPools/Address/isDestroyed", &watchAddressPool_isDestroyed);
	WATCH_OBJECT("objectPools/Address/memory", &watchAddressPool_bytes);
	WATCH_OBJECT("objectPools/Address/totalAllocs", &watchAddressPool_totalAlloc);

	WATCH_OBJECT("objectPools/MemoryStream/size", &watchMemoryStreamPool_size);
	WATCH_OBJECT("objectPools/MemoryStream/max", &watchMemoryStreamPool_max);
	WATCH_OBJECT("objectPools/MemoryStream/isDestroyed", &watchMemoryStreamPool_isDestroyed);
	WATCH_OBJECT("objectPools/MemoryStream/memory", &watchMemoryStreamPool_bytes);
	WATCH_OBJECT("objectPools/MemoryStream/totalAllocs", &watchMemoryStreamPool_totalAlloc);

	WATCH_OBJECT("objectPools/TCPPacket/size", &watchTCPPacketPool_size);
	WATCH_OBJECT("objectPools/TCPPacket/max", &watchTCPPacketPool_max);
	WATCH_OBJECT("objectPools/TCPPacket/isDestroyed", &watchTCPPacketPool_isDestroyed);
	WATCH_OBJECT("objectPools/TCPPacket/memory", &watchTCPPacketPool_bytes);
	WATCH_OBJECT("objectPools/TCPPacket/totalAllocs", &watchTCPPacketPool_totalAlloc);

	WATCH_OBJECT("objectPools/TCPPacketReceiver/size", &watchTCPPacketReceiverPool_size);
	WATCH_OBJECT("objectPools/TCPPacketReceiver/max", &watchTCPPacketReceiverPool_max);
	WATCH_OBJECT("objectPools/TCPPacketReceiver/isDestroyed", &watchTCPPacketReceiverPool_isDestroyed);
	WATCH_OBJECT("objectPools/TCPPacketReceiver/memory", &watchTCPPacketReceiverPool_bytes);
	WATCH_OBJECT("objectPools/TCPPacketReceiver/totalAllocs", &watchTCPPacketReceiverPool_totalAlloc);

	WATCH_OBJECT("objectPools/UDPPacket/size", &watchUDPPacketPool_size);
	WATCH_OBJECT("objectPools/UDPPacket/max", &watchUDPPacketPool_max);
	WATCH_OBJECT("objectPools/UDPPacket/isDestroyed", &watchUDPPacketPool_isDestroyed);
	WATCH_OBJECT("objectPools/UDPPacket/memory", &watchUDPPacketPool_bytes);
	WATCH_OBJECT("objectPools/UDPPacket/totalAllocs", &watchUDPPacketPool_totalAlloc);

	WATCH_OBJECT("objectPools/UDPPacketReceiver/size", &watchUDPPacketReceiverPool_size);
	WATCH_OBJECT("objectPools/UDPPacketReceiver/max", &watchUDPPacketReceiverPool_max);
	WATCH_OBJECT("objectPools/UDPPacketReceiver/isDestroyed", &watchUDPPacketReceiverPool_isDestroyed);
	WATCH_OBJECT("objectPools/UDPPacketReceiver/memory", &watchUDPPacketReceiverPool_bytes);
	WATCH_OBJECT("objectPools/UDPPacketReceiver/totalAllocs", &watchUDPPacketReceiverPool_totalAlloc);

	WATCH_OBJECT("objectPools/EndPoint/size", &watchEndPointPool_size);
	WATCH_OBJECT("objectPools/EndPoint/max", &watchEndPointPool_max);
	WATCH_OBJECT("objectPools/EndPoint/isDestroyed", &watchEndPointPool_isDestroyed);
	WATCH_OBJECT("objectPools/EndPoint/memory", &watchEndPointPool_bytes);
	WATCH_OBJECT("objectPools/EndPoint/totalAllocs", &watchEndPointPool_totalAlloc);
	return true;
}

//-------------------------------------------------------------------------------------
bool WatchPool::finiWatchPools()
{
	return true;
}

//-------------------------------------------------------------------------------------

}
