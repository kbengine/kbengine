/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
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

class Bundle
{
public:
	enum PacketType{
		UDP_PACKET,
		TCP_PACKET,
	};
	
	typedef std::vector<Packet*> Packets;

	Bundle(Channel * pChannel = NULL, PacketType pt = TCP_PACKET);
	virtual ~Bundle();
	
	void newMessage(const MessageHandler& msgHandler);
	
	void finish(void);

	void send(NetworkInterface & networkInterface, Channel * pChannel);
	void send(EndPoint& ep);
	
	void clear();
	bool isEmpty() const;
	int totalSize() const;
	int sizeInPackets();
	
	const Packets& packets() { return packets_; }
		
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
        
    Bundle &operator<<(const std::string &value)
    {
		int32 len = (int32)value.size();
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
		int32 len = (int32)strlen(str);
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
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(uint8 &value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(uint16 &value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(uint32 &value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(uint64 &value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(int8 &value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(int16 &value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(int32 &value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(int64 &value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(float &value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(double &value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }

    Bundle &operator>>(std::string& value)
    {
        (*pCurrPacket_) >> value;
        return *this;
    }
private:
	Channel * pChannel_;
	int		numMessages_;
	
	Packet* pCurrPacket_;
	MessageID currMsgID_;
	uint8 currMsgPacketCount_;
	MessageLength currMsgLength_;
	int32 currMsgHandlerLength_;

	Packets packets_;
	
	bool isTCPPacket_;

};

}
}

#ifdef CODE_INLINE
#include "bundle.ipp"
#endif
#endif // __NETWORKINTERFACE__