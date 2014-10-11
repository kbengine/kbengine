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

#ifndef KBE_NETWORKCHANCEL_HPP
#define KBE_NETWORKCHANCEL_HPP

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/timestamp.hpp"
#include "cstdkbe/refcountable.hpp"
#include "cstdkbe/objectpool.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"
#include "network/event_dispatcher.hpp"
#include "network/endpoint.hpp"
#include "network/packet.hpp"
#include "network/common.hpp"
#include "network/bundle.hpp"
#include "network/interfaces.hpp"
#include "network/packet_filter.hpp"

namespace KBEngine { 
namespace Mercury
{

class Bundle;
class NetworkInterface;
class MessageHandlers;
class PacketReader;

class Channel : public TimerHandler, public RefCountable, public PoolObject
{
public:
	void onReclaimObject();
	bool destructorPoolObject();

	enum Traits
	{
		/// This describes the properties of channel from server to server.
		INTERNAL = 0,

		/// This describes the properties of a channel from client to server.
		EXTERNAL = 1,
	};
	
	enum ChannelTypes
	{
		/// ��ͨͨ��
		CHANNEL_NORMAL = 0,

		// �����webͨ��
		CHANNEL_WEB = 1,
	};

	enum AddToReceiveWindowResult
	{
		SHOULD_PROCESS,
		SHOULD_NOT_PROCESS,
		PACKET_IS_CORRUPT
	};

	typedef std::vector<Packet*> BufferedReceives;
public:
	Channel();

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
	
	void stopInactivityDetection();

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
	NetworkInterface* pNetworkInterface()			{ return pNetworkInterface_; }
	void pNetworkInterface(NetworkInterface* pNetworkInterface) { pNetworkInterface_ = pNetworkInterface; }

	INLINE const Address& addr() const;
	void endpoint(const EndPoint* endpoint);
	INLINE EndPoint * endpoint() const;

	typedef std::vector<Bundle*> Bundles;
	Bundles & bundles();
	
	int32 bundlesLength();

	void pushBundle(Bundle* pBundle);

	const Bundles & bundles() const;

	void clearBundle();

	void send(Bundle * pBundle = NULL);
	void delayedSend();

	void reset(const EndPoint* endpoint, bool warnOnDiscard = true);

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
		
	void addReceiveWindow(Packet* pPacket);
	
	BufferedReceives& bufferedReceives(){ return bufferedReceives_[bufferedReceivesIdx_]; }
		
	void processPackets(KBEngine::Mercury::MessageHandlers* pMsgHandlers);

	bool isCondemn()const { return isCondemn_; }
	void condemn();

	ENTITY_ID proxyID()const { return proxyID_; }
	void proxyID(ENTITY_ID pid){ proxyID_ = pid; }

	const std::string& extra()const { return strextra_; }
	void extra(const std::string& s){ strextra_ = s; }

	COMPONENT_ID componentID()const{ return componentID_; }
	void componentID(COMPONENT_ID cid){ componentID_ = cid; }

	virtual void handshake();

	KBEngine::Mercury::MessageHandlers* pMsgHandlers()const { return pMsgHandlers_; }
	void pMsgHandlers(KBEngine::Mercury::MessageHandlers* pMsgHandlers) { pMsgHandlers_ = pMsgHandlers; }

	void readDataToBuffer();
	bool waitSend();
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
	ProtocolType				protocoltype_;
		
	ChannelID					id_;
	
	TimerHandle					inactivityTimerHandle_;
	
	uint64						inactivityExceptionPeriod_;
	
	uint64						lastReceivedTime_;
	
	Bundles						bundles_;

	uint32						windowSize_;
	
	uint8						bufferedReceivesIdx_;
	BufferedReceives			bufferedReceives_[2];

	PacketReader*				pPacketReader_;

	bool						isDestroyed_;

	// Statistics
	uint32						numPacketsSent_;
	uint32						numPacketsReceived_;
	uint32						numBytesSent_;
	uint32						numBytesReceived_;
	uint32						lastTickBytesReceived_;

	PacketFilterPtr				pFilter_;
	
	EndPoint *					pEndPoint_;
	PacketReceiver*				pPacketReceiver_;

	// ���Ϊtrue�� ���Ƶ���Ѿ���ò��Ϸ�
	bool						isCondemn_;

	// ������ⲿͨ���Ҵ�����һ��ǰ������ǰ�˴���ID
	ENTITY_ID					proxyID_;

	// ��չ��
	std::string					strextra_;

	// ͨ�����
	ChannelTypes				channelType_;

	COMPONENT_ID				componentID_;

	// ֧��ָ��ĳ��ͨ��ʹ��ĳ����Ϣhandlers
	KBEngine::Mercury::MessageHandlers* pMsgHandlers_;
};

typedef SmartPointer<Channel> ChannelPtr;
}
}

#ifdef CODE_INLINE
#include "channel.ipp"
#endif
#endif // KBE_NETWORKCHANCEL_HPP
