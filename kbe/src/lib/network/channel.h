// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
		/// 普通通道
		CHANNEL_NORMAL = 0,

		// 浏览器web通道
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
		创建发送bundle，该bundle可能是从send放入发送队列中获取的，如果队列为空
		则创建一个新的
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
		FLAG_SENDING	= 0x00000001,	// 发送信息中
		FLAG_DESTROYED	= 0x00000002,	// 通道已经销毁
		FLAG_HANDSHAKE	= 0x00000004,	// 已经握手过
		FLAG_CONDEMN	= 0x00000008,	// 该频道已经变得不合法
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

	// 如果是外部通道且代理了一个前端则会绑定前端代理ID
	ENTITY_ID					proxyID_;

	// 扩展用
	std::string					strextra_;

	// 通道类别
	ChannelTypes				channelType_;

	COMPONENT_ID				componentID_;

	// 支持指定某个通道使用某个消息handlers
	KBEngine::Network::MessageHandlers* pMsgHandlers_;

	uint32						flags_;
};

}
}

#ifdef CODE_INLINE
#include "channel.inl"
#endif
#endif // KBE_NETWORKCHANCEL_H
