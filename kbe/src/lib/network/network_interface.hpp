/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __NETWORK_INTERFACE__
#define __NETWORK_INTERFACE__

#include "memorystream.hpp"
#include "network/common.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "network/endpoint.hpp"

namespace KBEngine { 
namespace Mercury
{
enum NetworkInterfaceType
{
	NETWORK_INTERFACE_INTERNAL,
	NETWORK_INTERFACE_EXTERNAL
};

class Address;
class Bundle;
class Channel;
class ChannelTimeOutHandler;
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
	
	NetworkInterface(EventDispatcher * pMainDispatcher, NetworkInterfaceType interfaceType,
		uint16 listeningPort = 0, const char * listeningInterface = "");
	~NetworkInterface();
	
	void attach(EventDispatcher & mainDispatcher);
	void detach();

	bool recreateListeningSocket(uint16 listeningPort, const char * listeningInterface);

	bool registerChannel(Channel* channel);
	bool deregisterChannel(Channel* channel);
	Channel * findChannel(const Address & addr);

	ChannelTimeOutHandler * pChannelTimeOutHandler() const
		{ return pChannelTimeOutHandler_; }
	void pChannelTimeOutHandler( ChannelTimeOutHandler * pHandler )
		{ pChannelTimeOutHandler_ = pHandler; }
		
	EventDispatcher & dispatcher()			{ return *pDispatcher_; }
	EventDispatcher & mainDispatcher()		{ return *pMainDispatcher_; }

	bool isExternal() const				{ return isExternal_; }

	INLINE const Address & addr() const;
	EndPoint & endpoint()				{ return endpoint_; }

	const char * c_str() const { return endpoint_.c_str(); }

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
	
	bool isGood() const{ return (endpoint_ != -1) && !address_.isNone();}

	void onPacketIn(const Packet & packet);
	void onPacketOut(const Packet & packet);

	void onChannelGone(Channel * pChannel);
	void onChannelTimeOut(Channel * pChannel);
	
	void handleChannels(KBEngine::Mercury::MessageHandlers* pMsgHandlers);
private:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	void closeSocket();
private:
	EndPoint								endpoint_;

	Address									address_;

	ChannelMap								channelMap_;

	const bool								isExternal_;

	EventDispatcher *						pDispatcher_;
	EventDispatcher *						pMainDispatcher_;

	void *									pExtensionData_;
	
	ListenerReceiver *						pListenerReceiver_;
	
	DelayedChannels * 						pDelayedChannels_;
	
	ChannelTimeOutHandler *					pChannelTimeOutHandler_;	// 超时的通道可被这个句柄捕捉， 例如告知上层client断开
};

}
}

#ifdef CODE_INLINE
#include "network_interface.ipp"
#endif
#endif // __NETWORK_INTERFACE__