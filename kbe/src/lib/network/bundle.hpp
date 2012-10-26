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

#ifndef __NETWORK_BUNDLE__
#define __NETWORK_BUNDLE__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/objectpool.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"
#include "network/event_dispatcher.hpp"
#include "network/endpoint.hpp"
#include "network/common.hpp"
#include "network/packet.hpp"
#include "network/interface_defs.hpp"

namespace KBEngine { 
namespace Mercury
{
class NetworkInterface;
class Channel;

#define PACKET_MAX_CHUNK_SIZE() isTCPPacket_ ? PACKET_MAX_SIZE_TCP : PACKET_MAX_SIZE_UDP;

#define PACKET_OUT_VALUE(v)																					\
	if(packets_.size() <= 0)																				\
		return *this;																						\
																											\
    (*packets_[0]) >> v;																					\
	if(packets_[0]->opsize() == 0)																			\
		packets_.erase(packets_.begin());																	\
																											\
	return *this;																							\


#define TRACE_BUNDLE_DATA(isrecv, bundle, pCurrMsgHandler, length)											\
	if(Mercury::g_trace_packet > 0)																			\
	{																										\
		if(pCurrMsgHandler)																					\
		{																									\
			DEBUG_MSG("%s%s:msgID:%d, currMsgLength:%d\n",													\
				((isrecv == true) ? "====>" : "<===="), 													\
				pCurrMsgHandler->name.c_str(),																\
				bundle->messageID(), length);																\
		}																									\
																											\
		switch(Mercury::g_trace_packet)																		\
		{																									\
		case 1:																								\
			bundle->hexlike();																				\
			break;																							\
		case 2:																								\
			bundle->textlike();																				\
			break;																							\
		default:																							\
			bundle->print_storage();																		\
			break;																							\
		};																									\
	}																										\


class Bundle : public PoolObject
{
public:
	static ObjectPool<Bundle>& ObjPool();
	void onReclaimObject();

	typedef std::vector<Packet*> Packets;
	
	Bundle(Channel * pChannel = NULL, ProtocolType pt = PROTOCOL_TCP);
	virtual ~Bundle();
	
	void newMessage(const MessageHandler& msgHandler);
	
	void finish(bool issend = true);

	void send(NetworkInterface & networkInterface, Channel * pChannel);
	void send(EndPoint& ep);
	void sendto(EndPoint& ep, u_int16_t networkPort, u_int32_t networkAddr = BROADCAST);
	void onSendComplete();
	
	void clear(bool isRecl);
	bool isEmpty() const;
	int totalSize() const;
	int sizeInPackets();
	
	const Packets& packets() { return packets_; }
	Packet* pCurrPacket() { return pCurrPacket_; }
		
	Packet* newPacket();
	
	inline MessageID messageID() const { return currMsgID_; }
public:
	int32 onPacketAppend(int32 addsize)
	{
		if(pCurrPacket_ == NULL)
		{
			newPacket();
		}

		int32 packetmaxsize = PACKET_MAX_CHUNK_SIZE();
		int32 totalsize = (int32)pCurrPacket_->totalSize();

		if((int32)pCurrPacket_->wpos() >= packetmaxsize)
		{
			TRACE_BUNDLE_DATA(false, pCurrPacket_, pCurrMsgHandler_, totalsize);
			packets_.push_back(pCurrPacket_);
			currMsgPacketCount_++;
			newPacket();
			totalsize = 0;
		}

		int32 remainsize = packetmaxsize - totalsize;
		int32 taddsize = addsize;

		// 如果 当前包剩余空间小于要添加的字节则本次填满此包
		if(remainsize < addsize)
			taddsize = remainsize;
		
		currMsgLength_ += taddsize;
		return taddsize;
	}

    Bundle &operator<<(uint8 value)
    {
		onPacketAppend(sizeof(uint8));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(uint16 value)
    {
		onPacketAppend(sizeof(uint16));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(uint32 value)
    {
		onPacketAppend(sizeof(uint32));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(uint64 value)
    {
		onPacketAppend(sizeof(uint64));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(int8 value)
    {
		onPacketAppend(sizeof(int8));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(int16 value)
    {
		onPacketAppend(sizeof(int16));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(int32 value)
    {
		onPacketAppend(sizeof(int32));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(int64 value)
    {
		onPacketAppend(sizeof(int64));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(float value)
    {
		onPacketAppend(sizeof(float));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(double value)
    {
		onPacketAppend(sizeof(double));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(COMPONENT_TYPE value)
    {
		onPacketAppend(sizeof(int32));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(ENTITY_MAILBOX_TYPE value)
    {
		onPacketAppend(sizeof(int32));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(bool value)
    {
		onPacketAppend(sizeof(int8));
        (*pCurrPacket_) << value;
        return *this;
    }

    Bundle &operator<<(const std::string &value)
    {
		int32 len = (int32)value.size() + 1; // +1为字符串尾部的0位置
		int32 i = 0;
		int32 packetmaxsize = PACKET_MAX_CHUNK_SIZE();
		int32 addtotalsize = 0;

		while(len > 0)
		{
			int32 ilen = onPacketAppend(len);
			pCurrPacket_->append(value.c_str() + addtotalsize, ilen);
			addtotalsize += ilen;
			len -= ilen;
		}

        return *this;
    }
	
    Bundle &operator<<(const char *str)
    {
		int32 len = (int32)strlen(str) + 1;  // +1为字符串尾部的0位置
		int32 i = 0;
		int32 packetmaxsize = PACKET_MAX_CHUNK_SIZE();
		int32 addtotalsize = 0;

		while(len > 0)
		{
			int32 ilen = onPacketAppend(len);
			pCurrPacket_->append(str + addtotalsize, ilen);
			addtotalsize += ilen;
			len -= ilen;
		}

        return *this;
    }
    
	Bundle &append(Bundle* pBundle)
	{
		KBE_ASSERT(pBundle != NULL);
		return append(*pBundle);
	}

	Bundle &append(Bundle& bundle)
	{
		Packets::iterator iter = bundle.packets_.begin();
		for(; iter!=bundle.packets_.end(); iter++)
		{
			append((*iter)->data(), (*iter)->totalSize());
		}
		
		if(bundle.pCurrPacket_ == NULL)
			return *this;
		return append(bundle.pCurrPacket_->data(), bundle.pCurrPacket_->totalSize());
	}

	Bundle &append(MemoryStream* s)
	{
		KBE_ASSERT(s != NULL);
		return append(*s);
	}

	Bundle &append(MemoryStream& s)
	{
		if(s.wpos() > 0)
			return append(s.data() + s.rpos(), s.opsize());

		return *this;
	}

	Bundle &appendBlob(const uint8 *str, uint32 n)
	{
		(*this) << n;
		return assign((char*)str, n);
	}

	Bundle &append(const uint8 *str, int n)
	{
		return assign((char*)str, n);
	}

	Bundle &append(const char *str, int n)
	{
		return assign(str, n);
	}

	Bundle &assign(const char *str, int n)
	{
		int32 len = (int32)n;
		int32 i = 0;
		int32 packetmaxsize = PACKET_MAX_CHUNK_SIZE();
		int32 addtotalsize = 0;

		while(len > 0)
		{
			int32 ilen = onPacketAppend(len);
			pCurrPacket_->append((uint8*)(str + addtotalsize), ilen);
			addtotalsize += ilen;
			len -= ilen;
		}

		return *this;
	}

    Bundle &operator>>(bool &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(uint8 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(uint16 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(uint32 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(uint64 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(int8 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(int16 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(int32 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(int64 &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(float &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(double &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(COMPONENT_TYPE &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(ENTITY_MAILBOX_TYPE &value)
    {
        PACKET_OUT_VALUE(value);
    }

    Bundle &operator>>(std::string& value)
    {
        PACKET_OUT_VALUE(value);
    }
private:
	Channel * pChannel_;
	int		numMessages_;
	
	Packet* pCurrPacket_;
	MessageID currMsgID_;
	uint8 currMsgPacketCount_;
	MessageLength currMsgLength_;
	int32 currMsgHandlerLength_;
	size_t currMsgLengthPos_;

	Packets packets_;
	
	bool isTCPPacket_;

	const Mercury::MessageHandler* pCurrMsgHandler_;

};

}
}

#ifdef CODE_INLINE
#include "bundle.ipp"
#endif
#endif // __NETWORKINTERFACE__
