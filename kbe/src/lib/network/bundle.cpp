/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "bundle.h"
#include "network/network_stats.h"
#include "network/network_interface.h"
#include "network/channel.h"
#include "helper/profile.h"
#include "network/packet_sender.h"

#ifndef CODE_INLINE
#include "bundle.inl"
#endif

#include "common/blowfish.h"


namespace KBEngine { 
namespace Network
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
	size_t bytes = sizeof(pCurrMsgHandler_) + sizeof(isTCPPacket_) + 
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
	packetMaxSize_(0),
	pCurrMsgHandler_(NULL)
{
	_calcPacketMaxSize();
	 newPacket();
}

//-------------------------------------------------------------------------------------
Bundle::Bundle(const Bundle& bundle)
{
	// ��Щ������ǰ������
	// ������;����packet���ܴ���
	isTCPPacket_ = bundle.isTCPPacket_;
	pChannel_ = bundle.pChannel_;
	pCurrMsgHandler_ = bundle.pCurrMsgHandler_;
	currMsgID_ = bundle.currMsgID_;

	Packets::const_iterator iter = bundle.packets_.begin();
	for (; iter != bundle.packets_.end(); ++iter)
	{
		newPacket();
		pCurrPacket_->append(*static_cast<MemoryStream*>((*iter)));
		packets_.push_back(pCurrPacket_);
	}

	pCurrPacket_ = NULL;
	if(bundle.pCurrPacket_)
	{
		newPacket();
		pCurrPacket_->append(*static_cast<MemoryStream*>(bundle.pCurrPacket_));
	}

	numMessages_ = bundle.numMessages_;
	currMsgPacketCount_ = bundle.currMsgPacketCount_;
	currMsgLength_ = bundle.currMsgLength_;
	currMsgHandlerLength_ = bundle.currMsgHandlerLength_;
	currMsgLengthPos_ = bundle.currMsgLengthPos_;
	_calcPacketMaxSize();
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
void Bundle::_calcPacketMaxSize()
{
	// ���ʹ����openssl����ͨѶ�����Ǳ�֤һ��������ܱ�Blowfish::BLOCK_SIZE����
	// ���������ڼ���һ�����ذ�ʱ����Ҫ��������ֽ�
	if(g_channelExternalEncryptType == 1)
	{
		packetMaxSize_ = isTCPPacket_ ? (int)(TCPPacket::maxBufferSize() - ENCRYPTTION_WASTAGE_SIZE) :
			(PACKET_MAX_SIZE_UDP - ENCRYPTTION_WASTAGE_SIZE);

		packetMaxSize_ -= packetMaxSize_ % KBEngine::KBEBlowfish::BLOCK_SIZE;
	}
	else
	{
		packetMaxSize_ = isTCPPacket_ ? (int)TCPPacket::maxBufferSize() : PACKET_MAX_SIZE_UDP;
	}
}

//-------------------------------------------------------------------------------------
int32 Bundle::packetsLength(bool calccurr)
{
	int32 len = 0;

	Packets::iterator iter = packets_.begin();
	for (; iter != packets_.end(); ++iter)
	{
		len += (int)(*iter)->length();
	}

	if(calccurr && pCurrPacket_)
		len += (int)pCurrPacket_->length();

	return len;
}

//-------------------------------------------------------------------------------------
int32 Bundle::onPacketAppend(int32 addsize, bool inseparable)
{
	if(pCurrPacket_ == NULL)
	{
		newPacket();
	}

	int32 totalsize = (int32)pCurrPacket_->length();
	int32 fwpos = (int32)pCurrPacket_->wpos();

	if(inseparable)
		fwpos += addsize;

	// �����ǰ��װ���±���append�����ݣ�������䵽�°���
	if(fwpos >= packetMaxSize_)
	{
		packets_.push_back(pCurrPacket_);
		currMsgPacketCount_++;
		newPacket();
		totalsize = 0;
	}

	int32 remainsize = packetMaxSize_ - totalsize;
	int32 taddsize = addsize;

	// �����ǰ��ʣ��ռ�С��Ҫ��ӵ��ֽ��򱾴������˰�
	if(remainsize < addsize)
		taddsize = remainsize;
	
	currMsgLength_ += taddsize;
	return taddsize;
}

//-------------------------------------------------------------------------------------
Packet* Bundle::newPacket()
{
	MALLOC_PACKET(pCurrPacket_, isTCPPacket_);
	pCurrPacket_->pBundle(this);
	return pCurrPacket_;
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
	for (; iter != packets_.end(); ++iter)
	{
		if(!isRecl)
		{
			delete (*iter);
		}
		else
		{
			RECLAIM_PACKET(isTCPPacket_, (*iter));
		}
	}
	
	packets_.clear();

	pChannel_ = NULL;
	numMessages_ = 0;

	currMsgID_ = 0;
	currMsgPacketCount_ = 0;
	currMsgLength_ = 0;
	currMsgLengthPos_ = 0;
	currMsgHandlerLength_ = 0;
	pCurrMsgHandler_ = NULL;
	_calcPacketMaxSize();
}

//-------------------------------------------------------------------------------------
void Bundle::clearPackets()
{
	if(pCurrPacket_ != NULL)
	{
		packets_.push_back(pCurrPacket_);
		pCurrPacket_ = NULL;
	}

	Packets::iterator iter = packets_.begin();
	for (; iter != packets_.end(); ++iter)
	{
		RECLAIM_PACKET(isTCPPacket_, (*iter));
	}

	packets_.clear();
}

//-------------------------------------------------------------------------------------
void Bundle::newMessage(const MessageHandler& msgHandler)
{
	pCurrMsgHandler_ = &msgHandler;
	
	if(pCurrPacket_ == NULL)
		this->newPacket();

	finiMessage(false);
	KBE_ASSERT(pCurrPacket_ != NULL);
	
	(*this) << msgHandler.msgID;
	pCurrPacket_->messageID(msgHandler.msgID);

	// �˴����ڷǹ̶����ȵ���Ϣ��˵��Ҫ������������Ϣ����λΪ0�� �������Ҫ��䳤��
	if(msgHandler.msgLen == NETWORK_VARIABLE_MESSAGE)
	{
		MessageLength msglen = 0;
		currMsgLengthPos_ = pCurrPacket_->wpos();
		(*this) << msglen;
	}

	++numMessages_;
	currMsgID_ = msgHandler.msgID;
	currMsgPacketCount_ = 0;
	currMsgHandlerLength_ = msgHandler.msgLen;
}

//-------------------------------------------------------------------------------------
void Bundle::finiMessage(bool isSend)
{
	KBE_ASSERT(pCurrPacket_ != NULL);
	
	pCurrPacket_->pBundle(this);

	if(isSend)
	{
		currMsgPacketCount_++;
		packets_.push_back(pCurrPacket_);
	}

	// ����Ϣ���и���
	if(pCurrMsgHandler_){
		if(isSend || numMessages_ > 1)
		{
			NetworkStats::getSingleton().trackMessage(NetworkStats::SEND, 
									*pCurrMsgHandler_, currMsgLength_);
		}
	}

	// �˴����ڷǹ̶����ȵ���Ϣ��˵��Ҫ�����������ճ�����Ϣ
	if(currMsgHandlerLength_ < 0 || g_packetAlwaysContainLength)
	{
		Packet* pPacket = pCurrPacket_;
		if(currMsgPacketCount_ > 0)
			pPacket = packets_[packets_.size() - currMsgPacketCount_];

		currMsgLength_ -= NETWORK_MESSAGE_ID_SIZE;
		currMsgLength_ -= NETWORK_MESSAGE_LENGTH_SIZE;

		// �������һ�������Ҳ�����ܳ���NETWORK_MESSAGE_MAX_SIZE
		if(g_componentType == BOTS_TYPE || g_componentType == CLIENT_TYPE)
		{
			KBE_ASSERT(currMsgLength_ <= NETWORK_MESSAGE_MAX_SIZE);
		}

		// �����Ϣ���ȴ��ڵ���NETWORK_MESSAGE_MAX_SIZE
		// ʹ����չ��Ϣ���Ȼ��ƣ�����Ϣ���Ⱥ��������4�ֽ�
		// ������������ĳ���
		if(currMsgLength_ >= NETWORK_MESSAGE_MAX_SIZE)
		{
			MessageLength1 ex_msg_length = currMsgLength_;
			KBEngine::EndianConvert(ex_msg_length);

			MessageLength msgLen = NETWORK_MESSAGE_MAX_SIZE;
			KBEngine::EndianConvert(msgLen);

			memcpy(&pPacket->data()[currMsgLengthPos_], 
				(uint8*)&msgLen, NETWORK_MESSAGE_LENGTH_SIZE);

			pPacket->insert(currMsgLengthPos_ + NETWORK_MESSAGE_LENGTH_SIZE, (uint8*)&ex_msg_length, 
														NETWORK_MESSAGE_LENGTH1_SIZE);
		}
		else
		{
			MessageLength msgLen = (MessageLength)currMsgLength_;
			KBEngine::EndianConvert(msgLen);

			memcpy(&pPacket->data()[currMsgLengthPos_], 
				(uint8*)&msgLen, NETWORK_MESSAGE_LENGTH_SIZE);
		}
	}

	if(isSend)
	{
		currMsgHandlerLength_ = 0;
		pCurrPacket_ = NULL;

		if(Network::g_trace_packet > 0)
			_debugMessages();
	}

	currMsgID_ = 0;
	currMsgPacketCount_ = 0;
	currMsgLength_ = 0;
	currMsgLengthPos_ = 0;
}

//-------------------------------------------------------------------------------------
void Bundle::_debugMessages()
{
	if(!pCurrMsgHandler_)
		return;

	Packets packets;
	packets.insert(packets.end(), packets_.begin(), packets_.end());
	if(pCurrPacket_)
		packets.push_back(pCurrPacket_);

	MemoryStream* pMemoryStream = MemoryStream::ObjPool().createObject();
	MessageID msgid = 0;
	MessageLength msglen = 0;
	MessageLength1 msglen1 = 0;
	const Network::MessageHandler* pCurrMsgHandler = NULL;

	int state = 0; // 0:��ȡ��ϢID�� 1����ȡ��Ϣ���ȣ� 2����ȡ��Ϣ��չ����, 3:��ȡ����

	for(Packets::iterator iter = packets.begin(); iter != packets.end(); iter++)
	{
		Packet* pPacket = (*iter);
		if(pPacket->length() == 0)
			continue;

		size_t rpos = pPacket->rpos();
		size_t wpos = pPacket->wpos();

		while(pPacket->length() > 0)
		{
			if(state == 0)
			{
				// һЩsendto�����İ�����, �����Ҳ����Ҫ׷��
				if(pPacket->length() < NETWORK_MESSAGE_ID_SIZE)
				{
					pPacket->done();
					continue;
				}

				(*pPacket) >> msgid;
				(*pMemoryStream) << msgid;
				state = 1;
				continue;
			}
			else if(state == 1)
			{
				if(!pCurrMsgHandler_ || !pCurrMsgHandler_->pMessageHandlers)
				{
					pPacket->done();
					continue;
				}

				pCurrMsgHandler = pCurrMsgHandler_->pMessageHandlers->find(msgid);

				// һЩsendto�����İ������Ҳ���MsgHandler, �����Ҳ����Ҫ׷��
				if(!pCurrMsgHandler)
				{
					pPacket->done();
					continue;
				}

				if(pCurrMsgHandler->msgLen == NETWORK_VARIABLE_MESSAGE || g_packetAlwaysContainLength)
				{
					(*pPacket) >> msglen;
					(*pMemoryStream) << msglen;

					if(msglen == NETWORK_MESSAGE_MAX_SIZE)
						state = 2;
					else
						state = 3;
				}
				else
				{
					msglen = pCurrMsgHandler->msgLen;
					(*pMemoryStream) << msglen;
					state = 3;
				}

				continue;
			}
			else if(state == 2)
			{
				(*pPacket) >> msglen1;
				(*pMemoryStream) << msglen1;
				state = 3;
				continue;
			}
			else if(state == 3)
			{
				MessageLength1 totallen = msglen1 > 0 ? msglen1 : msglen;
				
				if(pPacket->length() >= totallen - pMemoryStream->length())
				{
					MessageLength1 len = totallen - (MessageLength1)pMemoryStream->length();
					pMemoryStream->append(pPacket->data() + pPacket->rpos(), len);
					pPacket->rpos((int)(pPacket->rpos() + len));
				}
				else
				{
					pMemoryStream->append(*static_cast<MemoryStream*>(pPacket));
					pPacket->done();
				}

				if(pMemoryStream->length() == totallen)
				{
					state = 0;
					msglen1 = 0;
					msglen = 0;
					msgid = 0;

					TRACE_MESSAGE_PACKET(false, pMemoryStream, pCurrMsgHandler, pMemoryStream->length(), 
						(pChannel_ != NULL ? pChannel_->c_str() : "None"));

					pMemoryStream->clear(false);
					continue;
				}
			}
		};

		pPacket->rpos((int)rpos);
		pPacket->wpos((int)wpos);
	}

	MemoryStream::ObjPool().reclaimObject(pMemoryStream);
}

//-------------------------------------------------------------------------------------
}
}
