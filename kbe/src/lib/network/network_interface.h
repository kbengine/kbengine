// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NETWORK_INTERFACE_H
#define KBE_NETWORK_INTERFACE_H

#include "common/memorystream.h"
#include "network/common.h"
#include "common/common.h"
#include "common/timer.h"
#include "helper/debug_helper.h"
#include "network/endpoint.h"

namespace KBEngine { 
namespace Network
{
class Address;
class Bundle;
class Channel;
class ChannelTimeOutHandler;
class ChannelDeregisterHandler;
class DelayedChannels;
class ListenerReceiver;
class Packet;
class EventDispatcher;
class MessageHandlers;

class NetworkInterface : public TimerHandler
{
public:
	typedef std::map<Address, Channel *>	ChannelMap;
	
	NetworkInterface(EventDispatcher * pDispatcher,
		int32 extlisteningTcpPort_min = -1, int32 extlisteningTcpPort_max = -1, int32 extlisteningUdpPort_min = -1, int32 extlisteningUdpPort_max = -1, const char * extlisteningInterface = "",
		uint32 extrbuffer = 0, uint32 extwbuffer = 0, 
		int32 intlisteningPort_min = 0, int32 intlisteningPort_max = 0, const char * intlisteningInterface = "",
		uint32 intrbuffer = 0, uint32 intwbuffer = 0);

	~NetworkInterface();

	INLINE const Address & extTcpAddr() const;
	INLINE const Address & extUdpAddr() const;
	INLINE const Address & intTcpAddr() const;

	bool initialize(const char* pEndPointName, uint16 listeningPort_min, uint16 listeningPort_max,
		const char * listeningInterface, EndPoint* pEP, ListenerReceiver* pLR, uint32 rbuffer = 0, uint32 wbuffer = 0);

	bool registerChannel(Channel* pChannel);
	bool deregisterChannel(Channel* pChannel);
	bool deregisterAllChannels();
	Channel * findChannel(const Address & addr);
	Channel * findChannel(int fd);

	ChannelTimeOutHandler * pChannelTimeOutHandler() const
		{ return pChannelTimeOutHandler_; }
	void pChannelTimeOutHandler(ChannelTimeOutHandler * pHandler)
		{ pChannelTimeOutHandler_ = pHandler; }
		
	ChannelDeregisterHandler * pChannelDeregisterHandler() const
		{ return pChannelDeregisterHandler_; }
	void pChannelDeregisterHandler(ChannelDeregisterHandler * pHandler)
		{ pChannelDeregisterHandler_ = pHandler; }

	EventDispatcher & dispatcher()		{ return *pDispatcher_; }

	/* 外部网点和内部网点 */
	EndPoint & extEndpoint()				{ return extTcpEndpoint_; }
	EndPoint & intEndpoint()				{ return intTcpEndpoint_; }

	const char * c_str() const { return extTcpEndpoint_.c_str(); }
	
	const ChannelMap& channels(void) { return channelMap_; }
		
	/** 发送相关 */
	void sendIfDelayed(Channel & channel);
	void delayedSend(Channel & channel);
	
	bool good() const{ return (!pExtListenerReceiver_ || extTcpEndpoint_.good()) && (intTcpEndpoint_.good()); }

	void onChannelTimeOut(Channel * pChannel);
	
	/* 
		处理所有channels  
	*/
	void processChannels(KBEngine::Network::MessageHandlers* pMsgHandlers);

	INLINE int32 numExtChannels() const;

private:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	void closeSocket();

private:
	EndPoint								extTcpEndpoint_, extUdpEndpoint_, intTcpEndpoint_;

	ChannelMap								channelMap_;

	EventDispatcher *						pDispatcher_;
	
	ListenerReceiver *						pExtListenerReceiver_;
	ListenerReceiver *						pExtUdpListenerReceiver_;
	ListenerReceiver *						pIntListenerReceiver_;
	
	DelayedChannels * 						pDelayedChannels_;
	
	ChannelTimeOutHandler *					pChannelTimeOutHandler_;	// 超时的通道可被这个句柄捕捉， 例如告知上层client断开
	ChannelDeregisterHandler *				pChannelDeregisterHandler_;

	int32									numExtChannels_;
};

}
}

#ifdef CODE_INLINE
#include "network_interface.inl"
#endif
#endif // KBE_NETWORK_INTERFACE_H
