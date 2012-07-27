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

#define PACKET_OUT_VALUE(v)						\
		if(packets_.size() <= 0)				\
			return *this;						\
												\
        (*packets_[0]) >> v;					\
		if(packets_[0]->opsize() == 0)			\
			packets_.erase(packets_.begin());	\
												\
		return *this;							\

class Bundle
{
public:
	typedef std::vector<Packet*> Packets;

	Bundle(Channel * pChannel = NULL, ProtocolType pt = PROTOCOL_TCP);
	virtual ~Bundle();
	
	void newMessage(const MessageHandler& msgHandler);
	
	void finish(bool issend = true);

	void send(NetworkInterface & networkInterface, Channel * pChannel);
	void send(EndPoint& ep);
	void sendto(EndPoint& ep, u_int16_t networkPort, u_int32_t networkAddr = BROADCAST);
	void onSendComplete();
	
	void clear();
	bool isEmpty() const;
	int totalSize() const;
	int sizeInPackets();
	
	const Packets& packets() { return packets_; }
	Packet* pCurrPacket() { return pCurrPacket_; }
		
	Packet* newPacket();
	
public:
	int32 onPacketAppend(int32 size)
	{
		int32 packetmaxsize = PACKET_MAX_CHUNK_SIZE();
		
		int32 totalsize = (int32)pCurrPacket_->totalSize();
		if((totalsize > 0) && (totalsize + size > packetmaxsize))
		{
			packets_.push_back(pCurrPacket_);
			currMsgPacketCount_++;
			currMsgLength_ += pCurrPacket_->totalSize();
			newPacket();
		}

		return size - packetmaxsize;
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
		onPacketAppend(sizeof(int8));
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
		int32 len = (int32)value.size() + 1;
		int32 i = 0;
		int32 packetmaxsize = PACKET_MAX_CHUNK_SIZE();

		while(true)
		{
			len = onPacketAppend(len);
			if(len > 0)
			{
				pCurrPacket_->append(value.c_str() + (i++ * packetmaxsize), packetmaxsize);
			}
			else
			{
				pCurrPacket_->append(value.c_str() + (i++ * packetmaxsize), packetmaxsize + len);
				break;
			}
		}

        return *this;
    }
	
    Bundle &operator<<(const char *str)
    {
		int32 len = (int32)strlen(str) + 1;
		int32 i = 0;
		int32 packetmaxsize = PACKET_MAX_CHUNK_SIZE();

		while(true)
		{
			len = onPacketAppend(len);
			if(len > 0)
			{
				pCurrPacket_->append(str + (i++ * packetmaxsize), packetmaxsize);
			}
			else
			{
				pCurrPacket_->append(str + (i++ * packetmaxsize), packetmaxsize + len);
				break;
			}
		}

        return *this;
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

		while(true)
		{
			len = onPacketAppend(len);
			if(len > 0)
			{
				pCurrPacket_->append((uint8*)str + (i++ * packetmaxsize), packetmaxsize);
			}
			else
			{
				pCurrPacket_->append((uint8*)str + (i++ * packetmaxsize), packetmaxsize + len);
				break;
			}
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

};

}
}

#ifdef CODE_INLINE
#include "bundle.ipp"
#endif
#endif // __NETWORKINTERFACE__