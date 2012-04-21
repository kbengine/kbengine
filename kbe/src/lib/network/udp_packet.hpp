/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __SOCKETUDPPACKET_H__
#define __SOCKETUDPPACKET_H__
	
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
class UDPPacket : public Packet
{
public:
    UDPPacket(PacketHeaders ph = PACKET_HEADER_UNKOWN, size_t res = 0);
	virtual ~UDPPacket(void);
	
	int recvFromEndPoint(EndPoint & ep);
};

typedef SmartPointer<UDPPacket> UDPPacketPtr;
}
}
#endif
