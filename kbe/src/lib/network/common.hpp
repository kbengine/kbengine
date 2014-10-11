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

#ifndef KBE_NETWORK_COMMON_HPP
#define KBE_NETWORK_COMMON_HPP

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
typedef uint16	MessageLength; // ���65535

typedef int32	ChannelID;
const ChannelID CHANNEL_ID_NULL = 0;

// ͨ����ʱʱ��
extern float g_channelInternalTimeout;
extern float g_channelExternalTimeout;

// ͨ�����ͳ�ʱ����
extern uint32 g_intReSendInterval;
extern uint32 g_intReSendRetries;
extern uint32 g_extReSendInterval;
extern uint32 g_extReSendRetries;

// �ⲿͨ���������
extern int8 g_channelExternalEncryptType;

// listen�����������ֵ
extern uint32 g_SOMAXCONN;

// ����ͨ����ʱ���
#define CLOSE_CHANNEL_INACTIVITIY_DETECTION()										\
{																					\
	Mercury::g_channelExternalTimeout = Mercury::g_channelInternalTimeout = -1.0f;	\
}																					\

	
namespace udp{
}

namespace tcp{
}

// ���ܶ���洢����Ϣռ���ֽ�(����+���)
#define ENCRYPTTION_WASTAGE_SIZE			(1 + 7)

#define PACKET_MAX_SIZE						1500
#define PACKET_MAX_SIZE_TCP					1460
#define PACKET_MAX_SIZE_UDP					1472

typedef uint16								PacketLength; // ���65535
#define PACKET_LENGTH_SIZE					sizeof(PacketLength)

#define MERCURY_MESSAGE_ID_SIZE				sizeof(Mercury::MessageID)
#define MERCURY_MESSAGE_LENGTH_SIZE			sizeof(Mercury::MessageLength)
#define MERCURY_MESSAGE_MAX_SIZE			65535

// ��Ϸ���ݿ��ð���С
#define GAME_PACKET_MAX_SIZE_TCP			PACKET_MAX_SIZE_TCP - MERCURY_MESSAGE_ID_SIZE - \
											MERCURY_MESSAGE_LENGTH_SIZE - ENCRYPTTION_WASTAGE_SIZE

/** kbe machine�˿� */
#define KBE_PORT_START						20000
#define KBE_MACHINE_BRAODCAST_SEND_PORT		KBE_PORT_START + 86	// machine���չ㲥�Ķ˿�
#define KBE_PORT_BROADCAST_DISCOVERY		KBE_PORT_START + 87
#define KBE_MACHINE_TCP_PORT				KBE_PORT_START + 88

#define KBE_BILLING_TCP_PORT				30099

/*
	������Ϣ���ͣ� �������߱䳤��
	�����Ҫ�Զ��峤������NETWORK_INTERFACE_DECLARE_BEGIN������ʱ���볤�ȼ��ɡ�
*/
#ifndef MERCURY_FIXED_MESSAGE
#define MERCURY_FIXED_MESSAGE 0
#endif

#ifndef MERCURY_VARIABLE_MESSAGE
#define MERCURY_VARIABLE_MESSAGE -1
#endif

// ������Ϣ���
enum MERCURY_MESSAGE_TYPE
{
	MERCURY_MESSAGE_TYPE_COMPONENT = 0,	// �����Ϣ
	MERCURY_MESSAGE_TYPE_ENTITY = 1,	// entity��Ϣ
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
	REASON_HTML5_ERROR = -13,		 ///< html5 error.
	REASON_CHANNEL_CONDEMN = -14,	 ///< condemn error.
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
		"REASON_SHUTTING_DOWN",
		"REASON_HTML5_ERROR",
		"REASON_CHANNEL_CONDEMN"
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
			/* ������ͳ��ִ�����ô���ǿ��Լ�������һ�Σ� ����3���˳�	*/								\
			if (reason == Mercury::REASON_NO_SUCH_PORT && retries <= 3)									\
			{																							\
				continue;																				\
			}																							\
																										\
			/* ���ϵͳ���ͻ����Ѿ����ˣ������ǵȴ�10ms	*/												\
			if ((reason == REASON_RESOURCE_UNAVAILABLE || reason == REASON_GENERAL_NETWORK)				\
															&& retries <= 3)							\
			{																							\
				WARNING_MSG(fmt::format("{}: "															\
					"Transmit queue full, waiting for space... ({})\n",									\
					__FUNCTION__, retries));															\
																										\
				KBEngine::sleep(10);																	\
				continue;																				\
			}																							\
																										\
			if(retries > 3 && reason != Mercury::REASON_SUCCESS)										\
			{																							\
				ERROR_MSG(fmt::format("MERCURY_SEND::send: packet discarded(reason={}).\n",				\
															(reasonToString(reason))));					\
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

// �����մ������
extern uint32						g_receiveWindowMessagesOverflowCritical;
extern uint32						g_intReceiveWindowMessagesOverflow;
extern uint32						g_extReceiveWindowMessagesOverflow;
extern uint32						g_intReceiveWindowBytesOverflow;
extern uint32						g_extReceiveWindowBytesOverflow;

bool initializeWatcher();
void finalise(void);

}
}

#endif // KBE_NETWORK_COMMON_HPP
