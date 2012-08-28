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

// 这个开关设置数据包是否总是携带长度信息， 这样在某些前端进行耦合时提供一些便利
// 如果为false则一些固定长度的数据包不携带长度信息， 由对端自行解析
extern bool g_packetAlwaysContainLength;

/*是否需要将任何接收和发送的包以文本输出到log中提供调试
		0: 不输出
		1: 16进制输出
		2: 字符流输出
		3: 10进制输出
*/
extern uint8 g_trace_packet;

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

/** kbe machine端口 */
#define KBE_PORT_START						20000
#define KBE_MACHINE_BRAODCAST_SEND_PORT		KBE_PORT_START + 86	// machine接收广播的端口
#define KBE_PORT_BROADCAST_DISCOVERY		KBE_PORT_START + 87
#define KBE_MACHINE_TCP_PORT				KBE_PORT_START + 88

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




}
}
#endif
