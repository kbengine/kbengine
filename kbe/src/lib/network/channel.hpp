/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __NETWORKCHANCEL__
#define __NETWORKCHANCEL__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"
#include "network/event_dispatcher.hpp"
#include "network/endpoint.hpp"
#include "network/packet.hpp"
#include "network/common.hpp"
#include "network/bundle.hpp"
#include "network/interfaces.hpp"
#include "network/circular_buffer.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/timestamp.hpp"
#include "cstdkbe/refcountable.hpp"
#include "network/packet_filter.hpp"

namespace KBEngine { 
namespace Mercury
{

class Bundle;
class NetworkInterface;

class Channel : public TimerHandler, public RefCountable
{
public:
	enum Traits
	{
		/// This describes the properties of channel from server to server.
		INTERNAL = 0,

		/// This describes the properties of a channel from client to server.
		EXTERNAL = 1,
	};
public:
	Channel(NetworkInterface & networkInterface, const EndPoint * endpoint, Traits traits, PacketFilterPtr pFilter = NULL, ChannelID id = CHANNEL_ID_NULL);

	virtual ~Channel();
	
	static Channel * get(NetworkInterface & networkInterface,
			const Address& addr);
	
	static Channel * get(NetworkInterface & networkInterface,
			const EndPoint* pSocket);
	
	void startInactivityDetection( float inactivityPeriod,
			float checkPeriod = 1.f );
	
	PacketFilterPtr pFilter() const { return pFilter_; }
	void pFilter(PacketFilterPtr pFilter) { pFilter_ = pFilter; }

	void destroy();
	bool isDestroyed() const { return isDestroyed_; }

	bool isDead() const
	{
		return this->isDestroyed();
	}
	
	bool isOwnedByInterface() const
		{ return !isDestroyed_; }

	NetworkInterface & networkInterface()			{ return *pNetworkInterface_; }
		
	INLINE const Address& addr() const;
	void endpoint(const EndPoint* endpoint);
	INLINE EndPoint * endpoint() const;
	Bundle & bundle();
	const Bundle & bundle() const;
	void clearBundle();

	void send(Bundle * pBundle = NULL);
	void delayedSend();

	void reset(const EndPoint* endpoint, bool warnOnDiscard = true);
	
	void dropNextSend() { shouldDropNextSend_ = true; }

	Traits traits() const { return traits_; }
	bool isExternal() const { return traits_ == EXTERNAL; }
	bool isInternal() const { return traits_ == INTERNAL; }
		
	void onPacketReceived(int bytes);

	const char * c_str() const;
	int windowSize() const;
	ChannelID id() const	{ return id_; }
	
	uint32	numPacketsSent() const		{ return numPacketsSent_; }
	uint32	numPacketsReceived() const	{ return numPacketsReceived_; }
	uint32	numBytesSent() const		{ return numBytesSent_; }
	uint32	numBytesReceived() const	{ return numBytesReceived_; }
		
	PacketReceiver* packetReceiver()const { return pPacketReceiver_; }
private:
	enum TimeOutType
	{
		TIMEOUT_INACTIVITY_CHECK
	};

	virtual void handleTimeout(TimerHandle, void * pUser);
	void clearState( bool warnOnDiscard = false );
	EventDispatcher & dispatcher();
private:
	NetworkInterface * 			pNetworkInterface_;
	Traits						traits_;

	ChannelID					id_;
	
	TimerHandle					inactivityTimerHandle_;
	
	uint64						inactivityExceptionPeriod_;
	
	uint64						lastReceivedTime_;
	
	Bundle*						pBundle_;
	uint32						windowSize_;
	uint64						roundTripTime_;
	
	CircularBuffer<PacketPtr>	bufferedReceives_;
	uint32						numBufferedReceives_;
	
	bool						isDestroyed_;
	bool						shouldDropNextSend_;
	
	// Statistics
	uint32						numPacketsSent_;
	uint32						numPacketsReceived_;
	uint32						numBytesSent_;
	uint32						numBytesReceived_;

	PacketFilterPtr				pFilter_;
	
	EndPoint *					pEndPoint_;
	PacketReceiver*				pPacketReceiver_;
};

typedef SmartPointer<Channel> ChannelPtr;
}
}

#ifdef CODE_INLINE
#include "channel.ipp"
#endif
#endif // __NETWORKINTERFACE__