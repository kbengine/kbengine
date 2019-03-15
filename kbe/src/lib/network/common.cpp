// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "common.h"
#include "common/ssl.h"
#include "network/http_utility.h"
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

// UDP参数
uint32						g_rudp_intWritePacketsQueueSize = 65535;
uint32						g_rudp_intReadPacketsQueueSize = 65535;
uint32						g_rudp_extWritePacketsQueueSize = 65535;
uint32						g_rudp_extReadPacketsQueueSize = 65535;
uint32						g_rudp_tickInterval = 10;
uint32						g_rudp_minRTO = 10;
uint32						g_rudp_missAcksResend = 1;
uint32						g_rudp_mtu = 0;
bool						g_rudp_congestionControl = false;
bool						g_rudp_nodelay = true;

const char*					UDP_HELLO = "62a559f3fa7748bc22f8e0766019d498";
const char*					UDP_HELLO_ACK = "1432ad7c829170a76dd31982c3501eca";

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

// Certificate file required for HTTPS/WSS/SSL communication
std::string					g_sslCertificate = "";
std::string					g_sslPrivateKey = "";

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

bool initialize()
{
	return KB_SSL::initialize() && Http::initialize();
}

void finalise(void)
{
	Http::finalise();
	KB_SSL::finalise();

#ifdef ENABLE_WATCHERS
	WatcherPaths::finalise();
#endif

	MessageHandlers::finalise();
	
	Network::destroyObjPool();
}

#if KBE_PLATFORM != PLATFORM_WIN32	
#include <sys/poll.h>
bool kbe_poll(int fd)
{
	int32 timeout = 100000;
	int maxi = 0;
	int icount = 1;
	struct pollfd clientfds[1024];

	clientfds[0].fd = fd;
	clientfds[0].events = POLLIN;

	for (int i = 1; i < 1024; i++)
		clientfds[i].fd = -1;

	while (1)
	{
		int nready = poll(clientfds, maxi + 1, timeout / 1000);

		if (nready == -1)
		{
			return false;
		}
		else if (nready == 0)
		{
			if (icount > 5)
				return false;

			icount++;
			continue;
		}
		else if (clientfds[0].revents & POLLIN)
		{
			return true;
		}
	}
}
#else
bool kbe_poll(int fd)
{
	fd_set	frds;
	struct timeval tv = { 0, 1000000 }; // 1s

	FD_ZERO(&frds);
	FD_SET(fd, &frds);

	int selgot = select(fd + 1, &frds, NULL, NULL, &tv);
	if (selgot <= 0)
		return false;
	else
		return true;
}
#endif

//-------------------------------------------------------------------------------------
}
}
