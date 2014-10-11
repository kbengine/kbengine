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
#include "network/mercurystats.hpp"
#include "network/network_interface.hpp"
#include "network/packet.hpp"
#include "network/channel.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "helper/profile.hpp"

#ifndef CODE_INLINE
#include "bundle.ipp"
#endif

#include "cstdkbe/blowfish.hpp"


#define BUNDLE_SEND_OP(op)																					\
	finish();																								\
																											\
	Packets::iterator iter = packets_.begin();																\
	for (; iter != packets_.end(); iter++)																	\
	{																										\
		Packet* pPacket = (*iter);																			\
		int retries = 0;																					\
		Reason reason;																						\
		pPacket->sentSize = 0;																				\
																											\
		while(true)																							\
		{																									\
			retries++;																						\
			int slen = op;																					\
																											\
			if(slen > 0)																					\
				pPacket->sentSize += slen;																	\
																											\
			if(pPacket->sentSize != pPacket->totalSize())													\
			{																								\
				reason = NetworkInterface::getSendErrorReason(&ep, pPacket->sentSize, pPacket->totalSize());\
				/* ������ͳ��ִ�����ô���ǿ��Լ�������һ�Σ� ����60���˳�	*/								\
				if (reason == REASON_NO_SUCH_PORT && retries <= 3)											\
				{																							\
					continue;																				\
				}																							\
																											\
				/* ���ϵͳ���ͻ����Ѿ����ˣ������ǵȴ�10ms	*/												\
				if ((reason == REASON_RESOURCE_UNAVAILABLE || reason == REASON_GENERAL_NETWORK)				\
																					&& retries <= 60)		\
				{																							\
					WARNING_MSG(fmt::format("{}: "															\
						"Transmit queue full, waiting for space... ({})\n",									\
						__FUNCTION__, retries));															\
																											\
					ep.waitSend();																			\
					continue;																				\
				}																							\
																											\
				if(retries > 60 && reason != REASON_SUCCESS)												\
				{																							\
					ERROR_MSG(fmt::format("Bundle::basicSendWithRetries: packet discarded(reason={}).\n",	\
															(reasonToString(reason))));						\
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
	onSendCompleted();																						\
																											\


namespace KBEngine { 
namespace Mercury
{

//-------------------------------------------------------------------------------------
static ObjectPool<Bundle> _g_objPool("Bundle");
ObjectPool<Bundle>& Bundle::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
void Bundle::destroyObjPool()
{
	DEBUG_MSG(fmt::format("Bundle::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
size_t Bundle::getPoolObjectBytes()
{
	size_t bytes = sizeof(reuse_) + sizeof(pCurrMsgHandler_) + sizeof(isTCPPacket_) + 
		sizeof(currMsgLengthPos_) + sizeof(currMsgHandlerLength_) + sizeof(currMsgLength_) + 
		sizeof(currMsgPacketCount_) + sizeof(currMsgID_) + sizeof(numMessages_) + sizeof(pChannel_)
		+ (packets_.size() * sizeof(Packet*));

	return bytes;
}

//-------------------------------------------------------------------------------------
Bundle::SmartPoolObjectPtr Bundle::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<Bundle>(ObjPool().createObject(), _g_objPool));
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
	pCurrMsgHandler_(NULL),
	reuse_(false)
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
int32 Bundle::packetsLength(bool calccurr)
{
	int32 len = 0;

	Packets::iterator iter = packets_.begin();
	for (; iter != packets_.end(); iter++)
	{
		len += (*iter)->opsize();
	}

	if(calccurr && pCurrPacket_)
		len += pCurrPacket_->opsize();

	return len;
}

//-------------------------------------------------------------------------------------
int32 Bundle::onPacketAppend(int32 addsize, bool inseparable)
{
	if(pCurrPacket_ == NULL)
	{
		newPacket();
	}

	int32 packetmaxsize = PACKET_MAX_CHUNK_SIZE();

	// ���ʹ����openssl����ͨѶ�����Ǳ�֤һ��������ܱ�Blowfish::BLOCK_SIZE����
	// ���������ڼ���һ�����ذ�ʱ����Ҫ��������ֽ�
	if(g_channelExternalEncryptType == 1)
		packetmaxsize -=  packetmaxsize % KBEngine::KBEBlowfish::BLOCK_SIZE;

	int32 totalsize = (int32)pCurrPacket_->totalSize();
	int32 fwpos = (int32)pCurrPacket_->wpos();

	if(inseparable)
		fwpos += addsize;

	if(fwpos >= packetmaxsize)
	{
		TRACE_BUNDLE_DATA(false, pCurrPacket_, pCurrMsgHandler_, totalsize, "None");
		packets_.push_back(pCurrPacket_);
		currMsgPacketCount_++;
		newPacket();
		totalsize = 0;
	}

	int32 remainsize = packetmaxsize - totalsize;
	int32 taddsize = addsize;

	// ��� ��ǰ��ʣ��ռ�С��Ҫ��ӵ��ֽ��򱾴������˰�
	if(remainsize < addsize)
		taddsize = remainsize;
	
	currMsgLength_ += taddsize;
	return taddsize;
}

//-------------------------------------------------------------------------------------
Packet* Bundle::newPacket()
{
	if(isTCPPacket_)
		pCurrPacket_ = TCPPacket::ObjPool().createObject();
	else
		pCurrPacket_ = UDPPacket::ObjPool().createObject();
	
	pCurrPacket_->pBundle(this);
	return pCurrPacket_;
}

//-------------------------------------------------------------------------------------
void Bundle::finish(bool issend)
{
	KBE_ASSERT(pCurrPacket_ != NULL);
	
	pCurrPacket_->pBundle(this);

	if(issend)
	{
		currMsgPacketCount_++;
		packets_.push_back(pCurrPacket_);
	}

	// ����Ϣ���и���
	if(pCurrMsgHandler_){
		if(issend || numMessages_ > 1)
		{
			MercuryStats::getSingleton().trackMessage(MercuryStats::SEND, 
				*pCurrMsgHandler_, currMsgLength_);
		}
	}

	// �˴����ڷǹ̶����ȵ���Ϣ��˵��Ҫ�����������ճ�����Ϣ
	if(currMsgHandlerLength_ < 0 || g_packetAlwaysContainLength)
	{
		Packet* pPacket = pCurrPacket_;
		if(currMsgPacketCount_ > 0)
			pPacket = packets_[packets_.size() - currMsgPacketCount_];

		currMsgLength_ -= MERCURY_MESSAGE_ID_SIZE;
		currMsgLength_ -= MERCURY_MESSAGE_LENGTH_SIZE;

		MessageLength msgLen = currMsgLength_;
		KBEngine::EndianConvert(msgLen);

		memcpy(&pPacket->data()[currMsgLengthPos_], 
			(uint8*)&msgLen, MERCURY_MESSAGE_LENGTH_SIZE);
	}

	if(issend)
	{
		currMsgHandlerLength_ = 0;

		TRACE_BUNDLE_DATA(false, pCurrPacket_, pCurrMsgHandler_, this->totalSize(), 
			(pChannel_ != NULL ? pChannel_->c_str() : "None"));

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
	if(pCurrPacket_ != NULL)
	{
		packets_.push_back(pCurrPacket_);
		pCurrPacket_ = NULL;
	}

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

	reuse_ = false;
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
	//AUTO_SCOPED_PROFILE("sendBundle");
	pChannel_ = pChannel;
	finish();
	networkInterface.send(*this, pChannel);
}

//-------------------------------------------------------------------------------------
void Bundle::resend(NetworkInterface & networkInterface, Channel * pChannel)
{
	if(!reuse_)
	{
		MessageID msgid = currMsgID_;
		const Mercury::MessageHandler* pCurrMsgHandler = pCurrMsgHandler_;
		finish();
		currMsgID_ = msgid;
		pCurrMsgHandler_ = pCurrMsgHandler;
	}
	else
	{
		if(this->totalSize() == 0)
			return;

		TRACE_BUNDLE_DATA(false, packets_[0], pCurrMsgHandler_, this->totalSize(), 
			(pChannel != NULL ? pChannel->c_str() : "None"));
	}
	
	reuse_ = true;
	pChannel_ = pChannel;
	networkInterface.send(*this, pChannel);
}

//-------------------------------------------------------------------------------------
void Bundle::send(EndPoint& ep)
{
	//AUTO_SCOPED_PROFILE("sendBundle");
	BUNDLE_SEND_OP(ep.send(pPacket->data() + pPacket->sentSize, pPacket->totalSize() - pPacket->sentSize));
}

//-------------------------------------------------------------------------------------
void Bundle::sendto(EndPoint& ep, u_int16_t networkPort, u_int32_t networkAddr)
{
	//AUTO_SCOPED_PROFILE("sendToBundle");
	BUNDLE_SEND_OP(ep.sendto(pPacket->data() + pPacket->sentSize, pPacket->totalSize() - pPacket->sentSize, 
		networkPort, networkAddr));
}

//-------------------------------------------------------------------------------------
void Bundle::onSendCompleted()
{
	if(reuse_)
		return;

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

	// �˴����ڷǹ̶����ȵ���Ϣ��˵��Ҫ������������Ϣ����λΪ0�� �������Ҫ��䳤��
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
