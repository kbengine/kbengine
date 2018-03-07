/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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


#include "common.h"
#include "network/channel.h"
#include "network/bundle.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "network/tcp_packet_receiver.h"
#include "network/udp_packet_receiver.h"
#include "network/address.h"
#include "helper/watcher.h"

namespace KBEngine { 
namespace Network
{

float g_channelInternalTimeout = 60.f;
float g_channelExternalTimeout = 60.f;

int8 g_channelExternalEncryptType = 0;

uint32 g_SOMAXCONN = 5;

// network stats
uint64						g_numPacketsSent = 0;
uint64						g_numPacketsReceived = 0;
uint64						g_numBytesSent = 0;
uint64						g_numBytesReceived = 0;

uint32						g_receiveWindowMessagesOverflowCritical = 32;
uint32						g_intReceiveWindowMessagesOverflow = 65535;
uint32						g_extReceiveWindowMessagesOverflow = 256;
uint32						g_intReceiveWindowBytesOverflow = 0;
uint32						g_extReceiveWindowBytesOverflow = 65535;

uint32						g_sendWindowMessagesOverflowCritical = 32;
uint32						g_intSendWindowMessagesOverflow = 65535;
uint32						g_extSendWindowMessagesOverflow = 256;
uint32						g_intSendWindowBytesOverflow = 0;
uint32						g_extSendWindowBytesOverflow = 65535;
uint32						g_intSentWindowBytesOverflow = 0;
uint32						g_extSentWindowBytesOverflow = 0;

// 通道发送超时重试
uint32						g_intReSendInterval = 10;
uint32						g_intReSendRetries = 0;
uint32						g_extReSendInterval = 10;
uint32						g_extReSendRetries = 3;

bool initializeWatcher()
{
	WATCH_OBJECT("network/numPacketsSent", g_numPacketsSent);
	WATCH_OBJECT("network/numPacketsReceived", g_numPacketsReceived);
	WATCH_OBJECT("network/numBytesSent", g_numBytesSent);
	WATCH_OBJECT("network/numBytesReceived", g_numBytesReceived);
	
	std::vector<MessageHandlers*>::iterator iter = MessageHandlers::messageHandlers().begin();
	for(; iter != MessageHandlers::messageHandlers().end(); ++iter)
	{
		if(!(*iter)->initializeWatcher())
			return false;
	}

	return true;
}

void destroyObjPool()
{
	Bundle::destroyObjPool();
	Channel::destroyObjPool();
	TCPPacket::destroyObjPool();
	UDPPacket::destroyObjPool();
	EndPoint::destroyObjPool();
	Address::destroyObjPool();
	TCPPacketReceiver::destroyObjPool();
	UDPPacketReceiver::destroyObjPool();
}

void finalise(void)
{
#ifdef ENABLE_WATCHERS
	WatcherPaths::finalise();
#endif

	MessageHandlers::finalise();
	
	Network::destroyObjPool();
}

//-------------------------------------------------------------------------------------
}
}
