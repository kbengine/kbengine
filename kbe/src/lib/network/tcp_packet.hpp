/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __SOCKETTCPPACKET_H__
#define __SOCKETTCPPACKET_H__
	
// common include
#include "network/packet.hpp"
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
class TCPPacket : public Packet
{
public:
    TCPPacket(MessageID msgID = 0, size_t res = 0);
	virtual ~TCPPacket(void);
	
	int recvFromEndPoint(EndPoint & ep, Address* pAddr = NULL);
};

typedef SmartPointer<TCPPacket> TCPPacketPtr;
}
}
#endif
