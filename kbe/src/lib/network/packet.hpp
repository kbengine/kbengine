/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __SOCKETPACKET_H__
#define __SOCKETPACKET_H__
	
// common include
#include "cstdkbe/memorystream.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/objectpool.hpp"
#include "cstdkbe/smartpointer.hpp"	
#include "network/common.hpp"

//#define NDEBUG
#include <assert.h>
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{
namespace Mercury
{
class EndPoint;
class Address;

class Packet : public MemoryStream, public RefCountable
{
public:
	Packet(MessageID msgID = 0, bool isTCPPacket = true, size_t res = 200):
	MemoryStream(res),
	msgID_(msgID),
	isTCPPacket_(isTCPPacket),
	encrypted_(false),
	sentSize(0)
	{
	};
	
	virtual ~Packet(void)
	{
	};
	
	virtual void onReclaimObject()
	{
		MemoryStream::onReclaimObject();
		resetPacket();
	}

	virtual int recvFromEndPoint(EndPoint & ep, Address* pAddr = NULL) = 0;
	
	virtual size_t totalSize() const { return wpos() - rpos(); }
    virtual bool empty() const { return totalSize() > 0; }

	void resetPacket(void)
	{
		wpos(0);
		rpos(0);
		encrypted_ = false;
		sentSize = 0;
		msgID_ = 0;
		// memset(data(), 0, size());
	};
	
	inline void messageID(MessageID msgID) { 
		msgID_ = msgID; 
	}

	inline MessageID messageID() const { return msgID_; }

	bool isTCPPacket()const { return isTCPPacket_; }

	bool encrypted()const { return encrypted_; }

	void encrypted(bool v) { encrypted_ = v; }
protected:
	MessageID msgID_;
	bool isTCPPacket_;
	bool encrypted_;
public:
	uint32 sentSize;

};

typedef SmartPointer<Packet> PacketPtr;
}
}
#endif
