/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
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
class MessageHandlers;

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
	
	enum AddToReceiveWindowResult
	{
		SHOULD_PROCESS,
		SHOULD_NOT_PROCESS,
		PACKET_IS_CORRUPT
	};

	typedef std::vector<PacketPtr> BufferedReceives;
public:
	Channel(NetworkInterface & networkInterface, 
		const EndPoint * endpoint, 
		Traits traits, 
		ProtocolType pt = PROTOCOL_TCP, 
		PacketFilterPtr pFilter = NULL, 
		ChannelID id = CHANNEL_ID_NULL);

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
		
	uint64 lastReceivedTime()const		{ return lastReceivedTime_; }
	void updateLastReceivedTime()		{ lastReceivedTime_ = timestamp(); }

	PacketReceiver* packetReceiver()const { return pPacketReceiver_; }
		
	Packet* receiveWindow();
	
	BufferedReceives& bufferedReceives(){ return bufferedReceives_; }
		
	void handleMessage(KBEngine::Mercury::MessageHandlers* pMsgHandlers);
private:
	enum TimeOutType
	{
		TIMEOUT_INACTIVITY_CHECK
	};

	virtual void handleTimeout(TimerHandle, void * pUser);
	void clearState( bool warnOnDiscard = false );
	EventDispatcher & dispatcher();

	void writeFragmentMessage(uint8 fragmentDatasFlag, Packet* pPacket, uint32 datasize);
	void mergeFragmentMessage(Packet* pPacket);
private:
	NetworkInterface * 			pNetworkInterface_;
	Traits						traits_;
	ProtocolType				protocoltype_;
		
	ChannelID					id_;
	
	TimerHandle					inactivityTimerHandle_;
	
	uint64						inactivityExceptionPeriod_;
	
	uint64						lastReceivedTime_;
	
	Bundle*						pBundle_;
	uint32						windowSize_;
	uint64						roundTripTime_;
	
	std::vector<PacketPtr>		bufferedReceives_;
	uint8						currbufferedIdx_;
	uint8*						pFragmentDatas_;
	uint32						pFragmentDatasWpos_;
	uint32						pFragmentDatasRemain_;
	uint8						fragmentDatasFlag_;
	MemoryStream*				pFragmentStream_;
	Mercury::MessageID			currMsgID_;
	Mercury::MessageLength		currMsgLen_;

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
