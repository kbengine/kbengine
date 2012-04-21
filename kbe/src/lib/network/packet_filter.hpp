/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __PACKET_FILTER_HPP__
#define __PACKET_FILTER_HPP__

#include "network/common.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/refcountable.hpp"

#include <string>
namespace KBEngine { 
namespace Mercury
{

class Channel;
class NetworkInterface;
class Packet;
class Address;
class PacketFilter;
class PacketReceiver;


class PacketFilter : public RefCountable
{
public:
	virtual ~PacketFilter() {}

	virtual Reason send(NetworkInterface & networkInterface, Channel * pChannel, Packet * pPacket);

	virtual Reason recv(PacketReceiver & receiver, Packet * pPacket);
};

typedef SmartPointer< PacketFilter > PacketFilterPtr;

}
}

#ifdef CODE_INLINE
#include "packet_filter.ipp"
#endif

#endif // __PACKET_FILTER_HPP__
