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

#ifndef KBE_NETWORK_INTERFACE_HPP
#define KBE_NETWORK_INTERFACE_HPP

#include "cstdkbe/memorystream.hpp"
#include "network/common.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "network/endpoint.hpp"

namespace KBEngine { 
namespace Mercury
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
	static const int RECV_BUFFER_SIZE;
	static const char * USE_KBEMACHINED;
	typedef std::map<Address, Channel *>	ChannelMap;
	
	NetworkInterface(EventDispatcher * pMainDispatcher,
		int32 extlisteningPort_min = -1, int32 extlisteningPort_max = -1, const char * extlisteningInterface = "",
		uint32 extrbuffer = 0, uint32 extwbuffer = 0, 
		int32 intlisteningPort = 0, const char * intlisteningInterface = "",
		uint32 intrbuffer = 0, uint32 intwbuffer = 0);

	~NetworkInterface();
	
	void attach(EventDispatcher & mainDispatcher);
	void detach();

	INLINE const Address & extaddr() const;
	INLINE const Address & intaddr() const;

	bool recreateListeningSocket(const char* pEndPointName, uint16 listeningPort_min, uint16 listeningPort_max, 
		const char * listeningInterface, EndPoint* pEP, ListenerReceiver* pLR, uint32 rbuffer = 0, uint32 wbuffer = 0);

	bool registerChannel(Channel* pChannel);
	bool deregisterChannel(Channel* pChannel);
	bool deregisterChannel(const Address & addr);
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

	EventDispatcher & dispatcher()			{ return *pDispatcher_; }
	EventDispatcher & mainDispatcher()		{ return *pMainDispatcher_; }

	/* 外部网点和内部网点 */
	EndPoint & extEndpoint()				{ return extEndpoint_; }
	EndPoint & intEndpoint()				{ return intEndpoint_; }
	
	bool isExternal() const				{ return isExternal_; }

	const char * c_str() const { return extEndpoint_.c_str(); }

	void * pExtensionData() const		{ return pExtensionData_; }
	void pExtensionData(void * pData)	{ pExtensionData_ = pData; }
	
	const ChannelMap& channels(void) { return channelMap_; }
		
	/** 发送相关 */
	Reason send(Bundle & bundle, Channel * pChannel = NULL);
	Reason sendPacket(Packet * pPacket, Channel * pChannel = NULL);
	void sendIfDelayed(Channel & channel);
	void delayedSend(Channel & channel);
	Reason basicSendSingleTry(Channel * pChannel, Packet * pPacket);
	Reason basicSendWithRetries(Channel * pChannel, Packet * pPacket);
	
	bool good() const{ return (!isExternal() || extEndpoint_.good()) && (intEndpoint_.good()); }

	void onPacketIn(const Packet & packet);
	void onPacketOut(const Packet & packet);

	void onChannelGone(Channel * pChannel);
	void onChannelTimeOut(Channel * pChannel);
	
	/* 
		处理所有消息包  
	*/
	void processAllChannelPackets(KBEngine::Mercury::MessageHandlers* pMsgHandlers);

	/* 
		获取一次send或者sendto操作产生错误的原因 
	*/
	static Reason getSendErrorReason(const EndPoint * endpoint, int retSendSize, int packetTotalSize);

	INLINE int32 numExtChannels() const;
private:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	void closeSocket();
private:
	EndPoint								extEndpoint_, intEndpoint_;

	ChannelMap								channelMap_;

	EventDispatcher *						pDispatcher_;
	EventDispatcher *						pMainDispatcher_;

	void *									pExtensionData_;
	
	ListenerReceiver *						pExtListenerReceiver_;
	ListenerReceiver *						pIntListenerReceiver_;
	
	DelayedChannels * 						pDelayedChannels_;
	
	ChannelTimeOutHandler *					pChannelTimeOutHandler_;	// 超时的通道可被这个句柄捕捉， 例如告知上层client断开
	ChannelDeregisterHandler *				pChannelDeregisterHandler_;

	const bool								isExternal_;

	int32									numExtChannels_;
};

}
}

#ifdef CODE_INLINE
#include "network_interface.ipp"
#endif
#endif // KBE_NETWORK_INTERFACE_HPP
