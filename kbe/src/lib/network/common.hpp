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

#ifndef __NETWORK_COMMON_H__
#define __NETWORK_COMMON_H__

// common include
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_option.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine { 
namespace Mercury
{
const uint32 BROADCAST = 0xFFFFFFFF;
const uint32 LOCALHOST = 0x0100007F;

typedef uint16	MessageID;
typedef uint16	MessageLength; // 最大65535

typedef int32	ChannelID;
const ChannelID CHANNEL_ID_NULL = 0;

// 频道超时时间
extern float g_channelInternalTimeout;
extern float g_channelExternalTimeout;

// listen监听队列最大值
extern uint32 g_SOMAXCONN;

// 不做频道超时检查
#define CLOSE_CHANNEL_INACTIVITIY_DETECTION()										\
{																					\
	Mercury::g_channelExternalTimeout = Mercury::g_channelInternalTimeout = -1.0f;	\
}																					\

	
namespace udp{
}

namespace tcp{
}

#define PACKET_MAX_SIZE						1500
#define PACKET_MAX_SIZE_TCP					1460
#define PACKET_MAX_SIZE_UDP					1472

#define MERCURY_MESSAGE_ID_SIZE				sizeof(Mercury::MessageID)
#define MERCURY_MESSAGE_LENGTH_SIZE			sizeof(Mercury::MessageLength)
#define MERCURY_MESSAGE_MAX_SIZE			65535

// 游戏内容可用包大小
#define GAME_PACKET_MAX_SIZE_TCP			PACKET_MAX_SIZE_TCP - MERCURY_MESSAGE_ID_SIZE - MERCURY_MESSAGE_LENGTH_SIZE

/** kbe machine端口 */
#define KBE_PORT_START						20000
#define KBE_MACHINE_BRAODCAST_SEND_PORT		KBE_PORT_START + 86	// machine接收广播的端口
#define KBE_PORT_BROADCAST_DISCOVERY		KBE_PORT_START + 87
#define KBE_MACHINE_TCP_PORT				KBE_PORT_START + 88

#define KBE_BILLING_TCP_PORT				30099

/*
	网络消息类型， 定长或者变长。
	如果需要自定义长度则在NETWORK_INTERFACE_DECLARE_BEGIN中声明时填入长度即可。
*/
#ifndef MERCURY_FIXED_MESSAGE
#define MERCURY_FIXED_MESSAGE 0
#endif

#ifndef MERCURY_VARIABLE_MESSAGE
#define MERCURY_VARIABLE_MESSAGE -1
#endif

// 网络消息类别
enum MERCURY_MESSAGE_TYPE
{
	MERCURY_MESSAGE_TYPE_COMPONENT = 0,	// 组件消息
	MERCURY_MESSAGE_TYPE_ENTITY = 1,	// entity消息
};

enum ProtocolType
{
	PROTOCOL_TCP = 0,
	PROTOCOL_UDP = 1,
};

enum Reason
{
	REASON_SUCCESS = 0,				 ///< No reason.
	REASON_TIMER_EXPIRED = -1,		 ///< Timer expired.
	REASON_NO_SUCH_PORT = -2,		 ///< Destination port is not open.
	REASON_GENERAL_NETWORK = -3,	 ///< The network is stuffed.
	REASON_CORRUPTED_PACKET = -4,	 ///< Got a bad packet.
	REASON_NONEXISTENT_ENTRY = -5,	 ///< Wanted to call a null function.
	REASON_WINDOW_OVERFLOW = -6,	 ///< Channel send window overflowed.
	REASON_INACTIVITY = -7,			 ///< Channel inactivity timeout.
	REASON_RESOURCE_UNAVAILABLE = -8,///< Corresponds to EAGAIN
	REASON_CLIENT_DISCONNECTED = -9, ///< Client disconnected voluntarily.
	REASON_TRANSMIT_QUEUE_FULL = -10,///< Corresponds to ENOBUFS
	REASON_CHANNEL_LOST = -11,		 ///< Corresponds to channel lost
	REASON_SHUTTING_DOWN = -12,		 ///< Corresponds to shutting down app.
};

inline
const char * reasonToString(Reason reason)
{
	const char * reasons[] =
	{
		"REASON_SUCCESS",
		"REASON_TIMER_EXPIRED",
		"REASON_NO_SUCH_PORT",
		"REASON_GENERAL_NETWORK",
		"REASON_CORRUPTED_PACKET",
		"REASON_NONEXISTENT_ENTRY",
		"REASON_WINDOW_OVERFLOW",
		"REASON_INACTIVITY",
		"REASON_RESOURCE_UNAVAILABLE",
		"REASON_CLIENT_DISCONNECTED",
		"REASON_TRANSMIT_QUEUE_FULL",
		"REASON_CHANNEL_LOST",
		"REASON_SHUTTING_DOWN"
	};

	unsigned int index = -reason;

	if (index < sizeof(reasons)/sizeof(reasons[0]))
	{
		return reasons[index];
	}

	return "REASON_UNKNOWN";
}


#define MERCURY_SEND_TO_ENDPOINT(ep, op, pPacket)														\
{																										\
	int retries = 0;																					\
	Mercury::Reason reason;																				\
																										\
	while(true)																							\
	{																									\
		retries++;																						\
		int slen = ep->op(pPacket->data(), pPacket->totalSize());										\
																										\
		if(slen != (int)pPacket->totalSize())															\
		{																								\
			reason = Mercury::NetworkInterface::getSendErrorReason(ep, slen, pPacket->totalSize());		\
			/* 如果发送出现错误那么我们可以继续尝试一次， 超过3次退出	*/								\
			if (reason == Mercury::REASON_NO_SUCH_PORT && retries <= 3)									\
			{																							\
				continue;																				\
			}																							\
																										\
			/* 如果系统发送缓冲已经满了，则我们等待10ms	*/												\
			if (reason == Mercury::REASON_RESOURCE_UNAVAILABLE && retries <= 3)							\
			{																							\
				WARNING_MSG( "%s: "																		\
					"Transmit queue full, waiting for space... (%d)\n",									\
					__FUNCTION__, retries );															\
																										\
				KBEngine::sleep(10);																	\
				continue;																				\
			}																							\
																										\
			if(retries > 3 && reason != Mercury::REASON_SUCCESS)										\
			{																							\
				ERROR_MSG("MERCURY_SEND::send: packet discarded.\n");									\
				break;																					\
			}																							\
		}																								\
		else																							\
		{																								\
			break;																						\
		}																								\
	}																									\
																										\
}																										\


void destroyObjPool();

// mercury stats
extern uint64						g_numPacketsSent;
extern uint64						g_numPacketsReceived;
extern uint64						g_numBytesSent;
extern uint64						g_numBytesReceived;

// 包接收窗口溢出
extern uint32						g_intReceiveWindowMessagesOverflow;
extern uint32						g_extReceiveWindowMessagesOverflow;
extern uint32						g_intReceiveWindowBytesOverflow;
extern uint32						g_extReceiveWindowBytesOverflow;

bool initializeWatcher();
void finalise(void);

}
}
#endif
