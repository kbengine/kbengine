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


#include "common.hpp"
#include "network/channel.hpp"
#include "network/bundle.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/tcp_packet_receiver.hpp"
#include "network/udp_packet_receiver.hpp"
#include "network/address.hpp"

namespace KBEngine { 
namespace Mercury
{

float g_channelInternalTimeout = 0.f;
float g_channelExternalTimeout = 0.f;

void destroyObjPool()
{
	Bundle::destroyObjPool();
	TCPPacket::destroyObjPool();
	EndPoint::destroyObjPool();
	Address::destroyObjPool();
	TCPPacketReceiver::destroyObjPool();
	UDPPacketReceiver::destroyObjPool();
}

//-------------------------------------------------------------------------------------
}
}
