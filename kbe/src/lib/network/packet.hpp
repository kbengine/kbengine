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
class Socket;
class Packet;
typedef SmartPointer<Packet> PacketPtr;

class Packet : public RefCountable
{
public:
    Packet();
	virtual ~Packet(void);
	
	int recvFromSocket( Socket & ep, Address & addr );

	int msgEndOffset() const	{ return msgEndOffset_; }
	void msgEndOffset( int offset )		{ msgEndOffset_ = offset; }
	int totalSize() const		{ return 1; }
protected:
	int			msgEndOffset_;


#ifdef _WIN32
	#pragma warning (push)
	#pragma warning (disable: 4200)
#endif
	/// The variable-length data follows the packet header in memory.
	char			data_[PACKET_MAX_SIZE];
#ifdef _WIN32
	#pragma warning (pop)
#endif
};
}
}
#endif
