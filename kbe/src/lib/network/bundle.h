// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NETWORK_BUNDLE_H
#define KBE_NETWORK_BUNDLE_H

#include "common/common.h"
#include "common/timer.h"
#include "common/objectpool.h"
#include "helper/debug_helper.h"
#include "network/address.h"
#include "network/event_dispatcher.h"
#include "network/endpoint.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/interface_defs.h"

namespace KBEngine { 
namespace Network
{
class NetworkInterface;
class Channel;

#define PACKET_OUT_VALUE(v, expectSize)																		\
	KBE_ASSERT(packetsLength() >= (int32)expectSize);														\
																											\
	size_t currSize = 0;																					\
	size_t reclaimCount = 0;																				\
																											\
	Packets::iterator iter = packets_.begin();																\
	for (; iter != packets_.end(); ++iter)																	\
	{																										\
		Packet* pPacket = (*iter);																			\
		size_t remainSize = (size_t)expectSize - currSize;													\
																											\
		if(pPacket->length() >= remainSize)																	\
		{																									\
			memcpy(((uint8*)&v) + currSize, pPacket->data() + pPacket->rpos(), remainSize);					\
			pPacket->rpos((int)(pPacket->rpos() + remainSize));												\
																											\
			if(pPacket->length() == 0)																		\
			{																								\
				RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);											\
				++reclaimCount;																				\
			}																								\
																											\
			break;																							\
		}																									\
		else																								\
		{																									\
			memcpy(((uint8*)&v) + currSize, pPacket->data() + pPacket->rpos(), pPacket->length());			\
			currSize += pPacket->length();																	\
			pPacket->done();																				\
			RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);												\
			++reclaimCount;																					\
		}																									\
	}																										\
																											\
	if(reclaimCount > 0)																					\
		packets_.erase(packets_.begin(), packets_.begin() + reclaimCount);									\
																											\
	return *this;																							\


// 从对象池中创建与回收
#define MALLOC_BUNDLE() Network::Bundle::createPoolObject()
#define DELETE_BUNDLE(obj) { Network::Bundle::reclaimPoolObject(obj); obj = NULL; }
#define RECLAIM_BUNDLE(obj) { Network::Bundle::reclaimPoolObject(obj);}

class Bundle : public PoolObject
{
public:
	typedef KBEShared_ptr< SmartPoolObject< Bundle > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj(const std::string& logPoint);
	static ObjectPool<Bundle>& ObjPool();
	static Bundle* createPoolObject(const std::string& logPoint);
	static void reclaimPoolObject(Bundle* obj);
	static void destroyObjPool();
	virtual void onReclaimObject();
	virtual size_t getPoolObjectBytes();

	typedef std::vector<Packet*> Packets;
	
	Bundle(Channel * pChannel = NULL, ProtocolType pt = PROTOCOL_TCP);
	Bundle(const Bundle& bundle);
	virtual ~Bundle();
	
	void newMessage(const MessageHandler& msgHandler);
	void finiMessage(bool isSend = true);

	void clearPackets();
	
	INLINE void pCurrMsgHandler(const Network::MessageHandler* pMsgHandler);
	INLINE const Network::MessageHandler* pCurrMsgHandler() const;

	/**
		计算所有包包括当前还未写完的包的总长度
	*/
	int32 packetsLength(bool calccurr = true);

	INLINE bool isTCPPacket() const{ return isTCPPacket_; }
	INLINE void isTCPPacket(bool v){ isTCPPacket_ = v; }

	void clear(bool isRecl);
	bool empty() const;
	
	void copy(const Bundle& bundle);

	INLINE int32 packetMaxSize() const;
	int packetsSize() const;

	/**
		撤销一些消息字节
	*/
	bool revokeMessage(int32 size);
		
	/**
		计算packetMaxSize-最后一个包的length后剩余的可用空间
	*/
	INLINE int32 lastPacketSpace();
	INLINE bool packetHaveSpace();
	
	INLINE Packets& packets();
	INLINE Packet* pCurrPacket() const;
	INLINE void pCurrPacket(Packet* p);
	
	INLINE void finiCurrPacket();

	Packet* newPacket();
	
	INLINE void pChannel(Channel* p);
	INLINE Channel* pChannel();
	
	INLINE MessageID messageID() const;
	INLINE void messageID(MessageID id);
	
	INLINE int32 numMessages() const;

	INLINE void currMsgPacketCount(uint32 v);
	INLINE uint32 currMsgPacketCount() const;

	INLINE void currMsgLength(MessageLength1 v);
	INLINE MessageLength1 currMsgLength() const;

	INLINE void currMsgLengthPos(size_t v);
	INLINE size_t currMsgLengthPos() const;

	static void debugCurrentMessages(MessageID currMsgID, const Network::MessageHandler* pCurrMsgHandler, 
		Network::Packet* pCurrPacket, Network::Bundle::Packets& packets, Network::MessageLength1 currMsgLength,
		Network::Channel* pChannel);
	
protected:
	void _calcPacketMaxSize();
	int32 onPacketAppend(int32 addsize, bool inseparable = true);

public:
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

    Bundle &operator<<(ENTITYCALL_TYPE value)
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
		for(; iter!=bundle.packets_.end(); ++iter)
		{
			append((*iter)->data() + (*iter)->rpos(), (int)(*iter)->length());
		}
		
		if(bundle.pCurrPacket_ == NULL)
			return *this;

		return append(bundle.pCurrPacket_->data() + bundle.pCurrPacket_->rpos(), (int)bundle.pCurrPacket_->length());
	}

	Bundle &append(MemoryStream* s)
	{
		KBE_ASSERT(s != NULL);
		return append(*s);
	}

	Bundle &append(MemoryStream& s)
	{
		if(s.length() > 0)
			return append(s.data() + s.rpos(), (int)s.length());

		return *this;
	}

	Bundle &appendBlob(const std::string& str)
	{
		return appendBlob((const uint8 *)str.data(), (ArraySize)str.size());
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

	Bundle &appendPackAnyXYZ(float x, float y, float z, const float epsilon = 0.5f)
	{
		if(epsilon > 0.f)
		{
			x = floorf(x + epsilon);
			y = floorf(y + epsilon);
			z = floorf(z + epsilon);
		}
		
		*this << x << y << z;
		return (*this);
	}

	Bundle &appendPackAnyXZ(float x, float z, const float epsilon = 0.5f)
    {
		if(epsilon > 0.f)
		{
			x = floorf(x + epsilon);
			z = floorf(z + epsilon);
		}

        *this << x << z;
		return (*this);
	}

    Bundle &appendPackXYZ(float x, float y, float z, float minf = -256.f)
    {
		x -= minf;
		y -= minf / 2.f;
		z -= minf;

		// 最大值不要超过-256~256
		// y 不要超过-128~128
        uint32 packed = 0;
        packed |= ((int)(x / 0.25f) & 0x7FF);
        packed |= ((int)(z / 0.25f) & 0x7FF) << 11;
        packed |= ((int)(y / 0.25f) & 0x3FF) << 22;
        *this << packed;
        return (*this);
    }

    Bundle &appendPackXZ(float x, float z)
    {
		MemoryStream::PackFloatXType xPackData; 
		xPackData.fv = x;

		MemoryStream::PackFloatXType zPackData; 
		zPackData.fv = z;
		
		// 0-7位存放尾数, 8-10位存放指数, 11位存放标志
		// 由于使用了24位来存储2个float， 并且要求能够达到-512~512之间的数
		// 8位尾数只能放最大值256, 指数只有3位(决定浮点数最大值为2^(2^3)=256) 
		// 我们舍去第一位使范围达到(-512~-2), (2~512)之间
		// 因此这里我们保证最小数为-2.f或者2.f
		xPackData.fv += xPackData.iv < 0 ? -2.f : 2.f;
		zPackData.fv += zPackData.iv < 0 ? -2.f : 2.f;

		uint32 data = 0;

		// 0x7ff000 = 11111111111000000000000
		// 0x0007ff = 00000000000011111111111
		const uint32 xCeilingValues[] = { 0, 0x7ff000 };
		const uint32 zCeilingValues[] = { 0, 0x0007ff };

		// 这里如果这个浮点数溢出了则设置浮点数为最大数
		// 这里检查了指数高4位和标记位， 如果高四位不为0则肯定溢出， 如果低4位和8位尾数不为0则溢出
		// 0x7c000000 = 1111100000000000000000000000000
		// 0x40000000 = 1000000000000000000000000000000
		// 0x3ffc000  = 0000011111111111100000000000000
		data |= xCeilingValues[((xPackData.uv & 0x7c000000) != 0x40000000) || ((xPackData.uv & 0x3ffc000) == 0x3ffc000)];
		data |= zCeilingValues[((zPackData.uv & 0x7c000000) != 0x40000000) || ((zPackData.uv & 0x3ffc000) == 0x3ffc000)];
		
		// 复制8位尾数和3位指数， 如果浮点数剩余尾数最高位是1则+1四舍五入, 并且存放到data中
		// 0x7ff000 = 11111111111000000000000
		// 0x0007ff = 00000000000011111111111
		// 0x4000	= 00000000100000000000000
		data |= ((xPackData.uv >>  3) & 0x7ff000) + ((xPackData.uv & 0x4000) >> 2);
		data |= ((zPackData.uv >> 15) & 0x0007ff) + ((zPackData.uv & 0x4000) >> 14);
		
		// 确保值在范围内
		// 0x7ff7ff = 11111111111011111111111
		data &= 0x7ff7ff;

		// 复制标记位
		// 0x800000 = 100000000000000000000000
		// 0x000800 = 000000000000100000000000
		data |=  (xPackData.uv >>  8) & 0x800000;
		data |=  (zPackData.uv >> 20) & 0x000800;

		uint8 packs[3];
		packs[0] = (uint8)(data >> 16);
		packs[1] = (uint8)(data >> 8);
		packs[2] = (uint8)data;
		(*this).append(packs, 3);
		return (*this);
    }

	Bundle &appendPackY(float y)
	{
		MemoryStream::PackFloatXType yPackData; 
		yPackData.fv = y;

		yPackData.fv += yPackData.iv < 0 ? -2.f : 2.f;
		uint16 data = 0;
		data = (yPackData.uv >> 12) & 0x7fff;
 		data |= ((yPackData.uv >> 16) & 0x8000);

		(*this) << data;
		return (*this);
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
        PACKET_OUT_VALUE(value, sizeof(bool));
    }

    Bundle &operator>>(uint8 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(uint8));
    }

    Bundle &operator>>(uint16 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(uint16));
    }

    Bundle &operator>>(uint32 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(uint32));
    }

    Bundle &operator>>(uint64 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(uint64));
    }

    Bundle &operator>>(int8 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(int8));
    }

    Bundle &operator>>(int16 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(int16));
    }

    Bundle &operator>>(int32 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(int32));
    }

    Bundle &operator>>(int64 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(int64));
    }

    Bundle &operator>>(float &value)
    {
        PACKET_OUT_VALUE(value, sizeof(float));
    }

    Bundle &operator>>(double &value)
    {
        PACKET_OUT_VALUE(value, sizeof(double));
    }

    Bundle &operator>>(COMPONENT_TYPE &value)
    {
        PACKET_OUT_VALUE(value, sizeof(int32/*参考MemoryStream*/));
    }

    Bundle &operator>>(ENTITYCALL_TYPE &value)
    {
        PACKET_OUT_VALUE(value, sizeof(int32/*参考MemoryStream*/));
    }

    Bundle &operator>>(std::string& value)
    {
		KBE_ASSERT(packetsLength() > 0);
		size_t reclaimCount = 0;
		value.clear();

		Packets::iterator iter = packets_.begin();
		for (; iter != packets_.end(); ++iter)
		{
			Packet* pPacket = (*iter);

			while (pPacket->length() > 0)
			{
				char c = pPacket->read<char>();
				if (c == 0)
					break;

				value += c;
			}

			if(pPacket->data()[pPacket->rpos() - 1] == 0)
			{
				if(pPacket->length() == 0)
				{
					RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);
					++reclaimCount;
				}

				break;
			}
			else
			{
				KBE_ASSERT(pPacket->length() == 0);
				++reclaimCount;
				RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);
			}
		}

		if(reclaimCount > 0)
			packets_.erase(packets_.begin(), packets_.begin() + reclaimCount);

		return *this;
    }

	ArraySize readBlob(std::string& datas)
	{
		datas.clear();

		ArraySize rsize = 0;
		(*this) >> rsize;

		if((int32)rsize > packetsLength())
			return 0;

		size_t reclaimCount = 0;
		datas.reserve(rsize);

		Packets::iterator iter = packets_.begin();
		for (; iter != packets_.end(); ++iter)
		{
			Packet* pPacket = (*iter);

			if(pPacket->length() > rsize - datas.size())
			{
				datas.append((char*)pPacket->data() + pPacket->rpos(), rsize - datas.size());
				pPacket->rpos((int)(pPacket->rpos() + rsize - datas.size()));
				if(pPacket->length() == 0)
				{
					RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);
					++reclaimCount;
				}

				break;
			}
			else
			{
				datas.append((char*)pPacket->data() + pPacket->rpos(), (int)pPacket->length());
				pPacket->done();
				RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);
				++reclaimCount;
			}
		}

		if(reclaimCount > 0)
			packets_.erase(packets_.begin(), packets_.begin() + reclaimCount);

		return rsize;
	}

private:
	Channel* pChannel_;
	int32 numMessages_;

	Packet* pCurrPacket_;
	MessageID currMsgID_;
	uint32 currMsgPacketCount_;
	MessageLength1 currMsgLength_;	
	int32 currMsgHandlerLength_;
	size_t currMsgLengthPos_;

	Packets packets_;

	bool isTCPPacket_;
	int32 packetMaxSize_;

	const Network::MessageHandler* pCurrMsgHandler_;
};

}
}

#ifdef CODE_INLINE
#include "bundle.inl"
#endif
#endif // KBE_NETWORK_BUNDLE_H
