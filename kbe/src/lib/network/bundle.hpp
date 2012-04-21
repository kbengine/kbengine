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

namespace KBEngine { 
namespace Mercury
{
class NetworkInterface;
class Channel;

class Bundle
{
public:
	typedef std::vector<Packet*> Packets;

	Bundle(Channel * pChannel = NULL);

	Bundle(Packet * packetChain);
	virtual ~Bundle();

	void send(NetworkInterface & networkInterface, Channel * pChannel);

	void clear();
	bool isEmpty() const;
	int size() const;
	int sizeInPackets();
	
	const Packets& packets() { return packets_; }
private:
	Channel * pChannel_;
	int		numMessages_;

	Packets packets_;

};

}
}

#ifdef CODE_INLINE
#include "bundle.ipp"
#endif
#endif // __NETWORKINTERFACE__