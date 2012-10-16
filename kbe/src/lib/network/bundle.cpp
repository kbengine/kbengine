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


#include "bundle.hpp"
#include "network/network_interface.hpp"
#include "network/packet.hpp"
#include "network/channel.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"

#ifndef CODE_INLINE
#include "bundle.ipp"
#endif


#define BUNDLE_SEND_OP(op)																					\
	finish();																								\
																											\
	Packets::iterator iter = packets_.begin();																\
	for (; iter != packets_.end(); iter++)																	\
	{																										\
		Packet* pPacket = (*iter);																			\
		int retries = 0;																					\
		Reason reason;																						\
																											\
		while(true)																							\
		{																									\
			retries++;																						\
			int slen = op;																					\
																											\
			if(slen != (int)pPacket->totalSize())															\
			{																								\
				reason = NetworkInterface::getSendErrorReason(&ep, slen, pPacket->totalSize());				\
				/* 如果发送出现错误那么我们可以继续尝试一次， 超过3次退出	*/									\
				if (reason == REASON_NO_SUCH_PORT && retries <= 3)											\
				{																							\
					continue;																				\
				}																							\
																											\
				/* 如果系统发送缓冲已经满了，则我们等待10ms	*/												\
				if (reason == REASON_RESOURCE_UNAVAILABLE && retries <= 3)									\
				{																							\
					fd_set	fds;																			\
					struct timeval tv = { 0, 10000 };														\
					FD_ZERO( &fds );																		\
					FD_SET(ep, &fds);																		\
																											\
					WARNING_MSG( "%s: "																		\
						"Transmit queue full, waiting for space... (%d)\n",									\
						__FUNCTION__, retries );															\
																											\
					select(ep + 1, NULL, &fds, NULL, &tv);													\
					continue;																				\
				}																							\
																											\
				if(retries > 3 && reason != REASON_SUCCESS)													\
				{																							\
					ERROR_MSG("Bundle::send: packet discarded.\n");											\
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
																											\
	onSendComplete();																						\
																											\


namespace KBEngine { 
namespace Mercury
{

//-------------------------------------------------------------------------------------
static ObjectPool<Bundle> _g_objPool;
ObjectPool<Bundle>& Bundle::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
Bundle::Bundle(Channel * pChannel, ProtocolType pt):
	pChannel_(pChannel),
	numMessages_(0),
	pCurrPacket_(NULL),
	currMsgID_(0),
	currMsgPacketCount_(0),
	currMsgLength_(0),
	currMsgHandlerLength_(0),
	currMsgLengthPos_(0),
	packets_(),
	isTCPPacket_(pt == PROTOCOL_TCP),
	pCurrMsgHandler_(NULL)
{
	 newPacket();
}

//-------------------------------------------------------------------------------------
Bundle::~Bundle()
{
	clear(false);
}

//-------------------------------------------------------------------------------------
void Bundle::onReclaimObject()
{
	clear(true);
}

//-------------------------------------------------------------------------------------
Packet* Bundle::newPacket()
{
	if(isTCPPacket_)
		pCurrPacket_ = TCPPacket::ObjPool().createObject();
	else
		pCurrPacket_ = UDPPacket::ObjPool().createObject();
	
	return pCurrPacket_;
}

//-------------------------------------------------------------------------------------
void Bundle::finish(bool issend)
{
	KBE_ASSERT(pCurrPacket_ != NULL);

	if(issend)
	{
		currMsgPacketCount_++;
		packets_.push_back(pCurrPacket_);
	}

	// 此处对于非固定长度的消息来说需要设置它的最终长度信息
	if(currMsgHandlerLength_ < 0 || g_packetAlwaysContainLength)
	{
		Packet* pPacket = pCurrPacket_;
		if(currMsgPacketCount_ > 0)
			pPacket = packets_[packets_.size() - currMsgPacketCount_];

		currMsgLength_ -= MERCURY_MESSAGE_ID_SIZE;
		currMsgLength_ -= MERCURY_MESSAGE_LENGTH_SIZE;

		memcpy(&pPacket->data()[currMsgLengthPos_], 
			(uint8*)&currMsgLength_, MERCURY_MESSAGE_LENGTH_SIZE);

	}
	
	if(issend)
	{
		currMsgHandlerLength_ = 0;
		TRACE_BUNDLE_DATA(false, pCurrPacket_, pCurrMsgHandler_, this->totalSize());
		pCurrPacket_ = NULL;
	}

	currMsgID_ = 0;
	currMsgPacketCount_ = 0;
	currMsgLength_ = 0;
	currMsgLengthPos_ = 0;
}

//-------------------------------------------------------------------------------------
void Bundle::clear(bool isRecl)
{
	Packets::iterator iter = packets_.begin();
	for (; iter != packets_.end(); iter++)
	{
		if(!isRecl)
		{
			delete (*iter);
		}
		else
		{
			if(isTCPPacket_)
				TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>((*iter)));
			else
				UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket*>((*iter)));
		}
	}
	
	packets_.clear();

	if(pCurrPacket_)
	{
		if(!isRecl)
		{
			delete pCurrPacket_;
		}
		else
		{
			if(isTCPPacket_)
				TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>(pCurrPacket_));
			else
				UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket*>(pCurrPacket_));
		}

		pCurrPacket_ = NULL;
	}

	pChannel_ = NULL;
	numMessages_ = 0;

	currMsgID_ = 0;
	currMsgPacketCount_ = 0;
	currMsgLength_ = 0;
	currMsgLengthPos_ = 0;
	currMsgHandlerLength_ = 0;
	pCurrMsgHandler_ = NULL;
}

//-------------------------------------------------------------------------------------
void Bundle::send(NetworkInterface & networkInterface, Channel * pChannel)
{
	finish();
	networkInterface.send(*this, pChannel);
}

//-------------------------------------------------------------------------------------
void Bundle::send(EndPoint& ep)
{
	BUNDLE_SEND_OP(ep.send(pPacket->data(), pPacket->totalSize()));
}

//-------------------------------------------------------------------------------------
void Bundle::sendto(EndPoint& ep, u_int16_t networkPort, u_int32_t networkAddr)
{
	BUNDLE_SEND_OP(ep.sendto(pPacket->data(), pPacket->totalSize(), networkPort, networkAddr));
}

//-------------------------------------------------------------------------------------
void Bundle::onSendComplete()
{
	Packets::iterator iter = packets_.begin();
	for (; iter != packets_.end(); iter++)
	{
		if(isTCPPacket_)
			TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>((*iter)));
		else
			UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket*>((*iter)));
	}

	packets_.clear();
}

//-------------------------------------------------------------------------------------
void Bundle::newMessage(const MessageHandler& msgHandler)
{
	pCurrMsgHandler_ = &msgHandler;
	
	if(pCurrPacket_ == NULL)
		this->newPacket();

	finish(false);
	KBE_ASSERT(pCurrPacket_ != NULL);
	
	(*this) << msgHandler.msgID;
	pCurrPacket_->messageID(msgHandler.msgID);

	// 此处对于非固定长度的消息来说需要先设置它的消息长度位为0， 到最后需要填充长度
	if(msgHandler.msgLen == MERCURY_VARIABLE_MESSAGE)
	{
		MessageLength msglen = 0;
		currMsgLengthPos_ = pCurrPacket_->wpos();
		(*this) << msglen;
	}

	numMessages_++;
	currMsgID_ = msgHandler.msgID;
	currMsgPacketCount_ = 0;
	currMsgHandlerLength_ = msgHandler.msgLen;
}

//-------------------------------------------------------------------------------------
}
}
