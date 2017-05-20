/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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

#ifndef KBE_NETWORKCHANCEL_H
#define KBE_NETWORKCHANCEL_H

#include "common/common.h"
#include "common/timer.h"
#include "common/smartpointer.h"
#include "common/timestamp.h"
#include "common/objectpool.h"
#include "helper/debug_helper.h"
#include "network/address.h"
#include "network/event_dispatcher.h"
#include "network/endpoint.h"
#include "network/packet.h"
#include "network/common.h"
#include "network/bundle.h"
#include "network/interfaces.h"
#include "network/packet_filter.h"

namespace KBEngine { 
namespace Network
{

class Bundle;
class NetworkInterface;
class MessageHandlers;
class PacketReader;
class PacketSender;

class Channel : public TimerHandler, public PoolObject
{
public:
	typedef KBEShared_ptr< SmartPoolObject< Channel > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj();
	static ObjectPool<Channel>& ObjPool();
	static Channel* createPoolObject();
	static void reclaimPoolObject(Channel* obj);
	static void destroyObjPool();
	virtual void onReclaimObject();
	virtual size_t getPoolObjectBytes();
	virtual void onEabledPoolObject();

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

	typedef std::vector<Packet*> BufferedReceives;

public:
	Channel();

	Channel(NetworkInterface & networkInterface, 
		const EndPoint * pEndPoint, 
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
	bool isDestroyed() const { return (flags_ & FLAG_DESTROYED) > 0; }

	NetworkInterface & networkInterface()			{ return *pNetworkInterface_; }
	NetworkInterface* pNetworkInterface()			{ return pNetworkInterface_; }
	void pNetworkInterface(NetworkInterface* pNetworkInterface) { pNetworkInterface_ = pNetworkInterface; }

	INLINE const Address& addr() const;
	void pEndPoint(const EndPoint* pEndPoint);
	INLINE EndPoint * pEndPoint() const;

	typedef std::vector<Bundle*> Bundles;
	Bundles & bundles();
	
	/**
		��������bundle����bundle�����Ǵ�send���뷢�Ͷ����л�ȡ�ģ��������Ϊ��
		�򴴽�һ���µ�
	*/
	Bundle* createSendBundle();
	
	int32 bundlesLength();

	const Bundles & bundles() const;
	INLINE void pushBundle(Bundle* pBundle);
	void clearBundle();

	bool sending() const { return (flags_ & FLAG_SENDING) > 0;}
	void stopSend();

	void send(Bundle * pBundle = NULL);
	void delayedSend();


	INLINE PacketReader* pPacketReader() const;
	INLINE PacketSender* pPacketSender() const;
	INLINE void pPacketSender(PacketSender* pPacketSender);
	INLINE PacketReceiver* pPacketReceiver() const;

	Traits traits() const { return traits_; }
	bool isExternal() const { return traits_ == EXTERNAL; }
	bool isInternal() const { return traits_ == INTERNAL; }
		
	void onPacketReceived(int bytes);
	void onPacketSent(int bytes, bool sentCompleted);
	void onSendCompleted();

	const char * c_str() const;
	ChannelID id() const	{ return id_; }
	
	uint32	numPacketsSent() const		{ return numPacketsSent_; }
	uint32	numPacketsReceived() const	{ return numPacketsReceived_; }
	uint32	numBytesSent() const		{ return numBytesSent_; }
	uint32	numBytesReceived() const	{ return numBytesReceived_; }
		
	uint64 lastReceivedTime() const		{ return lastReceivedTime_; }
	void updateLastReceivedTime()		{ lastReceivedTime_ = timestamp(); }
		
	void addReceiveWindow(Packet* pPacket);
	
	BufferedReceives& bufferedReceives(){ return bufferedReceives_; }
		
	void processPackets(KBEngine::Network::MessageHandlers* pMsgHandlers);

	bool isCondemn() const { return (flags_ & FLAG_CONDEMN) > 0; }
	void condemn();

	bool hasHandshake() const { return (flags_ & FLAG_HANDSHAKE) > 0; }

	ENTITY_ID proxyID() const { return proxyID_; }
	void proxyID(ENTITY_ID pid){ proxyID_ = pid; }

	const std::string& extra() const { return strextra_; }
	void extra(const std::string& s){ strextra_ = s; }

	COMPONENT_ID componentID() const{ return componentID_; }
	void componentID(COMPONENT_ID cid){ componentID_ = cid; }

	virtual void handshake();

	KBEngine::Network::MessageHandlers* pMsgHandlers() const { return pMsgHandlers_; }
	void pMsgHandlers(KBEngine::Network::MessageHandlers* pMsgHandlers) { pMsgHandlers_ = pMsgHandlers; }

	bool waitSend();

	bool initialize(NetworkInterface & networkInterface, 
		const EndPoint * pEndPoint, 
		Traits traits, 
		ProtocolType pt = PROTOCOL_TCP, 
		PacketFilterPtr pFilter = NULL, 
		ChannelID id = CHANNEL_ID_NULL);

	bool finalise();

	ChannelTypes type() const {
		return channelType_;;
	}

private:

	enum Flags
	{
		FLAG_SENDING	= 0x00000001,	// ������Ϣ��
		FLAG_DESTROYED	= 0x00000002,	// ͨ���Ѿ�����
		FLAG_HANDSHAKE	= 0x00000004,	// �Ѿ����ֹ�
		FLAG_CONDEMN	= 0x00000008,	// ��Ƶ���Ѿ���ò��Ϸ�
	};

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
	
	BufferedReceives			bufferedReceives_;

	PacketReader*				pPacketReader_;

	// Statistics
	uint32						numPacketsSent_;
	uint32						numPacketsReceived_;
	uint32						numBytesSent_;
	uint32						numBytesReceived_;
	uint32						lastTickBytesReceived_;
	uint32						lastTickBytesSent_;

	PacketFilterPtr				pFilter_;
	
	EndPoint *					pEndPoint_;
	PacketReceiver*				pPacketReceiver_;
	PacketSender*				pPacketSender_;

	// ������ⲿͨ���Ҵ�����һ��ǰ������ǰ�˴���ID
	ENTITY_ID					proxyID_;

	// ��չ��
	std::string					strextra_;

	// ͨ�����
	ChannelTypes				channelType_;

	COMPONENT_ID				componentID_;

	// ֧��ָ��ĳ��ͨ��ʹ��ĳ����Ϣhandlers
	KBEngine::Network::MessageHandlers* pMsgHandlers_;

	uint32						flags_;
};

}
}

#ifdef CODE_INLINE
#include "channel.inl"
#endif
#endif // KBE_NETWORKCHANCEL_H
