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

#ifndef KBE_NETWORK_BUNDLE_HPP
#define KBE_NETWORK_BUNDLE_HPP

#include "common/common.hpp"
#include "common/timer.hpp"
#include "common/objectpool.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"
#include "network/event_dispatcher.hpp"
#include "network/endpoint.hpp"
#include "network/common.hpp"
#include "network/packet.hpp"
#include "network/interface_defs.hpp"

namespace KBEngine { 
namespace Network
{
class NetworkInterface;
class Channel;

#define PACKET_MAX_CHUNK_SIZE() isTCPPacket_ ? (PACKET_MAX_SIZE_TCP - ENCRYPTTION_WASTAGE_SIZE):			\
	(PACKET_MAX_SIZE_UDP - ENCRYPTTION_WASTAGE_SIZE);

#define PACKET_OUT_VALUE(v)																					\
	if(packets_.size() <= 0)																				\
		return *this;																						\
																											\
    (*packets_[0]) >> v;																					\
	if(packets_[0]->opsize() == 0)																			\
		packets_.erase(packets_.begin());																	\
																											\
	return *this;																							\


#define TRACE_BUNDLE_DATA(isrecv, bundle, pCurrMsgHandler, length, addr)									\
	if(Network::g_trace_packet > 0)																			\
	{																										\
		if(Network::g_trace_packet_use_logfile)																\
			DebugHelper::getSingleton().changeLogger("packetlogs");											\
																											\
		bool isprint = true;																				\
		if(pCurrMsgHandler)																					\
		{																									\
			std::vector<std::string>::iterator iter = std::find(Network::g_trace_packet_disables.begin(),	\
														Network::g_trace_packet_disables.end(),				\
															pCurrMsgHandler->name);							\
																											\
			if(iter != Network::g_trace_packet_disables.end())												\
			{																								\
				isprint = false;																			\
			}																								\
			else																							\
			{																								\
				DEBUG_MSG(fmt::format("{} {}:msgID:{}, currMsgLength:{}, addr:{}\n",						\
						((isrecv == true) ? "====>" : "<===="),												\
						pCurrMsgHandler->name.c_str(),														\
						pCurrMsgHandler->msgID,																\
						length,																				\
						addr));																				\
			}																								\
		}																									\
																											\
		if(isprint)																							\
		{																									\
			switch(Network::g_trace_packet)																	\
			{																								\
			case 1:																							\
				bundle->hexlike();																			\
				break;																						\
			case 2:																							\
				bundle->textlike();																			\
				break;																						\
			default:																						\
				bundle->print_storage();																	\
				break;																						\
			};																								\
		}																									\
																											\
		if(Network::g_trace_packet_use_logfile)																\
			DebugHelper::getSingleton().changeLogger(COMPONENT_NAME_EX(g_componentType));					\
	}																										\


// 从对象池中创建与回收
#define NEW_BUNDLE() Network::Bundle::ObjPool().createObject()
#define DELETE_BUNDLE(obj) { Network::Bundle::ObjPool().reclaimObject(obj); obj = NULL; }

class Bundle : public PoolObject
{
public:
	typedef KBEShared_ptr< SmartPoolObject< Bundle > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj();
	static ObjectPool<Bundle>& ObjPool();
	static void destroyObjPool();
	void onReclaimObject();
	virtual size_t getPoolObjectBytes();

	typedef std::vector<Packet*> Packets;
	
	Bundle(Channel * pChannel = NULL, ProtocolType pt = PROTOCOL_TCP);
	virtual ~Bundle();
	
	void newMessage(const MessageHandler& msgHandler);
	
	void finish(bool issend = true);

	void resend(NetworkInterface & networkInterface, Channel * pChannel);
	void send(NetworkInterface & networkInterface, Channel * pChannel);
	void send(EndPoint& ep);
	void sendto(EndPoint& ep, u_int16_t networkPort, u_int32_t networkAddr = BROADCAST);
	void onSendCompleted();
	
	void clearPackets(){packets_.clear();}

	MessageLength currMsgLength()const { return currMsgLength_; }
	
	void pCurrMsgHandler(const Network::MessageHandler* pMsgHandler){ pCurrMsgHandler_ = pMsgHandler; }

	/**
		计算所有包包括当前还未写完的包的总长度
	*/
	int32 packetsLength(bool calccurr = true);

	void clear(bool isRecl);
	bool isEmpty() const;
	int totalSize() const;
	int sizeInPackets();
	
	const Packets& packets() { return packets_; }
	Packet* pCurrPacket() { return pCurrPacket_; }
	void pCurrPacket(Packet* p) { pCurrPacket_ = p; }

	void finiCurrPacket(){ 
		packets_.push_back(pCurrPacket_); 
		pCurrPacket_ = NULL; 
	}

	Packet* newPacket();
	
	inline MessageID messageID() const { return currMsgID_; }

	bool reuse(){ return reuse_; } 
	void setreuse(bool v = true){ reuse_ = v; } 
public:
	int32 onPacketAppend(int32 addsize, bool inseparable = true);

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
		int32 addtotalsize = 0;

		while(len > 0)
		{
			int32 ilen = onPacketAppend(len, false);
			pCurrPacket_->append(value.c_str() + addtotalsize, ilen);
			addtotalsize += ilen;
			len -= ilen;
		}

        return *this;
    }
	
    Bundle &operator<<(const char *str)
    {
		int32 len = (int32)strlen(str) + 1;  // +1为字符串尾部的0位置
		int32 addtotalsize = 0;

		while(len > 0)
		{
			int32 ilen = onPacketAppend(len, false);
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
		if(s.opsize() > 0)
			return append(s.data() + s.rpos(), s.opsize());

		return *this;
	}

	Bundle &appendBlob(const std::string& str)
	{
		return appendBlob((const uint8 *)str.data(), str.size());
	}

	Bundle &appendBlob(const char* str, ArraySize n)
	{
		return appendBlob((const uint8 *)str, n);
	}

	Bundle &appendBlob(const uint8 *str, ArraySize n)
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
		int32 addtotalsize = 0;

		while(len > 0)
		{
			int32 ilen = onPacketAppend(len, false);
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
	uint32 currMsgPacketCount_;
	MessageLength1 currMsgLength_;	
	int32 currMsgHandlerLength_;
	size_t currMsgLengthPos_;

	Packets packets_;
	
	bool isTCPPacket_;

	const Network::MessageHandler* pCurrMsgHandler_;

	bool reuse_;

};

}
}

#ifdef CODE_INLINE
#include "bundle.ipp"
#endif
#endif // KBE_NETWORK_BUNDLE_HPP
