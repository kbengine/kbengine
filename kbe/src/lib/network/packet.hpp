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
#include "memorystream.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "network/common.hpp"
#include "cstdkbe/smartpointer.hpp"	

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

class Packet : public MemoryStream, public RefCountable
{
public:

	Packet(MessageID msgID = 0, size_t res = 200):
	MemoryStream(res),
	msgID_(msgID)
	{
	};
	
	virtual ~Packet(void)
	{
	};
	
	virtual int recvFromEndPoint(EndPoint & ep) = 0;
	
	virtual size_t totalSize() const { return wpos() - rpos(); }
    virtual bool empty() const { return totalSize() > 0; }
	
	void resetPacket(void)
	{
		wpos(0);
		rpos(0);
		// memset(data(), 0, size());
	};
	
	inline void messageID(MessageID msgID) { 
		msgID_ = msgID; 
	}

	inline MessageID messageID() const { return msgID_; }
protected:
	MessageID msgID_;

};

typedef SmartPointer<Packet> PacketPtr;
}
}
#endif
