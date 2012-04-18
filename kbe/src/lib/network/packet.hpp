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
#include "network/address.hpp"
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

class Packet : public RefCountable
{
public:
	Packet(){};
	virtual ~Packet(void){};
	
	virtual int recvFromEndPoint(EndPoint & ep) = 0;
	virtual int totalSize() const { return 0; }
};

typedef SmartPointer<Packet> PacketPtr;
}
}
#endif
