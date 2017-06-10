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
		int32 extlisteningPort_min = -1, int32 extlisteningPort_max = -1, const char * extlisteningInterface = "",
		uint32 extrbuffer = 0, uint32 extwbuffer = 0, 
		int32 intlisteningPort = 0, const char * intlisteningInterface = "",
		uint32 intrbuffer = 0, uint32 intwbuffer = 0);

	~NetworkInterface();

	INLINE const Address & extaddr() const;
	INLINE const Address & intaddr() const;

	bool recreateListeningSocket(const char* pEndPointName, uint16 listeningPort_min, uint16 listeningPort_max, 
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

	/* �ⲿ������ڲ����� */
	EndPoint & extEndpoint()				{ return extEndpoint_; }
	EndPoint & intEndpoint()				{ return intEndpoint_; }
	
	bool isExternal() const				{ return isExternal_; }

	const char * c_str() const { return extEndpoint_.c_str(); }

	void * pExtensionData() const		{ return pExtensionData_; }
	void pExtensionData(void * pData)	{ pExtensionData_ = pData; }
	
	const ChannelMap& channels(void) { return channelMap_; }
		
	/** ������� */
	void sendIfDelayed(Channel & channel);
	void delayedSend(Channel & channel);
	
	bool good() const{ return (!isExternal() || extEndpoint_.good()) && (intEndpoint_.good()); }

	void onChannelTimeOut(Channel * pChannel);
	
	/* 
		��������channels  
	*/
	void processChannels(KBEngine::Network::MessageHandlers* pMsgHandlers);

	INLINE int32 numExtChannels() const;

private:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	void closeSocket();

private:
	EndPoint								extEndpoint_, intEndpoint_;

	ChannelMap								channelMap_;

	EventDispatcher *						pDispatcher_;

	void *									pExtensionData_;
	
	ListenerReceiver *						pExtListenerReceiver_;
	ListenerReceiver *						pIntListenerReceiver_;
	
	DelayedChannels * 						pDelayedChannels_;
	
	ChannelTimeOutHandler *					pChannelTimeOutHandler_;	// ��ʱ��ͨ���ɱ���������׽�� �����֪�ϲ�client�Ͽ�
	ChannelDeregisterHandler *				pChannelDeregisterHandler_;

	const bool								isExternal_;

	int32									numExtChannels_;
};

}
}

#ifdef CODE_INLINE
#include "network_interface.inl"
#endif
#endif // KBE_NETWORK_INTERFACE_H
