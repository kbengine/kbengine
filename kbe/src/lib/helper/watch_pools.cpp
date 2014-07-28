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

bool watchBundlePool_isDestroyed()
{
	return Mercury::Bundle::ObjPool().isDestroyed();
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

bool watchAddressPool_isDestroyed()
{
	return Mercury::Address::ObjPool().isDestroyed();
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

bool watchMemoryStreamPool_isDestroyed()
{
	return MemoryStream::ObjPool().isDestroyed();
}

//-------------------------------------------------------------------------------------
/*
int32 watchWitnessPool_size()
{
	return (int)Witness::ObjPool().objects().size();
}

int32 watchWitnessPool_max()
{
	return (int)Witness::ObjPool().max();
}

bool watchWitnessPool_isDestroyed()
{
	return Witness::ObjPool().isDestroyed();
}
*/

//-------------------------------------------------------------------------------------
int32 watchTCPPacketPool_size()
{
	return (int)Mercury::TCPPacket::ObjPool().objects().size();
}

int32 watchTCPPacketPool_max()
{
	return (int)Mercury::TCPPacket::ObjPool().max();
}

bool watchTCPPacketPool_isDestroyed()
{
	return Mercury::TCPPacket::ObjPool().isDestroyed();
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

bool watchTCPPacketReceiverPool_isDestroyed()
{
	return Mercury::TCPPacketReceiver::ObjPool().isDestroyed();
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

bool watchUDPPacketPool_isDestroyed()
{
	return Mercury::UDPPacket::ObjPool().isDestroyed();
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

bool watchUDPPacketReceiverPool_isDestroyed()
{
	return Mercury::UDPPacketReceiver::ObjPool().isDestroyed();
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

bool watchEndPointPool_isDestroyed()
{
	return Mercury::EndPoint::ObjPool().isDestroyed();
}

//-------------------------------------------------------------------------------------
bool WatchPool::initWatchPools()
{
	WATCH_OBJECT("objectPools/Bundle/size", &watchBundlePool_size);
	WATCH_OBJECT("objectPools/Bundle/max", &watchBundlePool_max);
	WATCH_OBJECT("objectPools/Bundle/isDestroyed", &watchBundlePool_isDestroyed);

	WATCH_OBJECT("objectPools/Address/size", &watchAddressPool_size);
	WATCH_OBJECT("objectPools/Address/max", &watchAddressPool_max);
	WATCH_OBJECT("objectPools/Address/isDestroyed", &watchAddressPool_isDestroyed);

	WATCH_OBJECT("objectPools/MemoryStream/size", &watchMemoryStreamPool_size);
	WATCH_OBJECT("objectPools/MemoryStream/max", &watchMemoryStreamPool_max);
	WATCH_OBJECT("objectPools/MemoryStream/isDestroyed", &watchMemoryStreamPool_isDestroyed);
	
	/*
	WATCH_OBJECT("objectPools/Witness/size", &watchWitnessPool_size);
	WATCH_OBJECT("objectPools/Witness/max", &watchWitnessPool_max);
	WATCH_OBJECT("objectPools/Witness/isDestroyed", &watchWitnessPool_isDestroyed);
	*/

	WATCH_OBJECT("objectPools/TCPPacket/size", &watchTCPPacketPool_size);
	WATCH_OBJECT("objectPools/TCPPacket/max", &watchTCPPacketPool_max);
	WATCH_OBJECT("objectPools/TCPPacket/isDestroyed", &watchTCPPacketPool_isDestroyed);

	WATCH_OBJECT("objectPools/TCPPacketReceiver/size", &watchTCPPacketReceiverPool_size);
	WATCH_OBJECT("objectPools/TCPPacketReceiver/max", &watchTCPPacketReceiverPool_max);
	WATCH_OBJECT("objectPools/TCPPacketReceiver/isDestroyed", &watchTCPPacketReceiverPool_isDestroyed);

	WATCH_OBJECT("objectPools/UDPPacket/size", &watchUDPPacketPool_size);
	WATCH_OBJECT("objectPools/UDPPacket/max", &watchUDPPacketPool_max);
	WATCH_OBJECT("objectPools/UDPPacket/isDestroyed", &watchUDPPacketPool_isDestroyed);

	WATCH_OBJECT("objectPools/UDPPacketReceiver/size", &watchUDPPacketReceiverPool_size);
	WATCH_OBJECT("objectPools/UDPPacketReceiver/max", &watchUDPPacketReceiverPool_max);
	WATCH_OBJECT("objectPools/UDPPacketReceiver/isDestroyed", &watchUDPPacketReceiverPool_isDestroyed);

	WATCH_OBJECT("objectPools/EndPoint/size", &watchEndPointPool_size);
	WATCH_OBJECT("objectPools/EndPoint/max", &watchEndPointPool_max);
	WATCH_OBJECT("objectPools/EndPoint/isDestroyed", &watchEndPointPool_isDestroyed);
	return true;
}

//-------------------------------------------------------------------------------------
bool WatchPool::finiWatchPools()
{
	return true;
}

//-------------------------------------------------------------------------------------

}
