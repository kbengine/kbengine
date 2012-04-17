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
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"
#include "network/event_dispatcher.hpp"
#include "network/packet_receiver.hpp"
#include "network/socket.hpp"
#include "network/common.hpp"
#include "network/channel.hpp"
#include "network/packet.hpp"

namespace KBEngine { 
namespace Mercury
{
enum NetworkInterfaceType
{
	NETWORK_INTERFACE_INTERNAL,
	NETWORK_INTERFACE_EXTERNAL
};

class NetworkInterface : public TimerHandler
{
public:
	static const int RECV_BUFFER_SIZE;
	static const char * USE_KBEMACHINED;
	
	NetworkInterface(EventDispatcher * pMainDispatcher, NetworkInterfaceType interfaceType,
		uint16 listeningPort = 0, const char * listeningInterface = "");
	~NetworkInterface();
	
	void attach(EventDispatcher & mainDispatcher);
	void detach();

	bool recreateListeningSocket(uint16 listeningPort,
							const char * listeningInterface);

	bool registerChannel(Channel & channel);
	bool deregisterChannel(Channel & channel);
	
	Channel * findChannel(const Address & addr);

	void onChannelGone(Channel * pChannel);
	void onChannelTimeOut(Channel * pChannel);

	Reason processPacketFromStream(const Address & addr,
		MemoryStream & data);

	EventDispatcher & dispatcher()			{ return *pDispatcher_; }
	EventDispatcher & mainDispatcher()		{ return *pMainDispatcher_; }

	INLINE const Address & address() const;
	
	void delayedSend(Channel & channel);

	bool isExternal() const				{ return isExternal_; }
	bool isVerbose() const				{ return isVerbose_; }
	void isVerbose(bool value)			{ isVerbose_ = value; }

	Socket & socket()					{ return socket_; }

	const char * c_str() const { return socket_.c_str(); }

	void * pExtensionData() const			{ return pExtensionData_; }
	void pExtensionData(void * pData)		{ pExtensionData_ = pData; }

	unsigned int numBytesReceivedForMessage(uint8 msgID) const;

	Reason send(Bundle & bundle, Channel * pChannel = NULL);
	
	Reason sendPacket(Packet * pPacket, Channel * pChannel = NULL);

	bool isGood() const
	{
		return (socket_ != -1) && !address_.isNone();
	}

	void onPacketIn(const Packet & packet);
	void onPacketOut(const Packet & packet);

private:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	void closeSocket();
private:
	Socket		socket_;

	Address	address_;

	typedef std::map< Address, Channel * >	ChannelMap;
	ChannelMap					channelMap_;

	const bool isExternal_;

	bool isVerbose_;

	EventDispatcher * pDispatcher_;
	EventDispatcher * pMainDispatcher_;

	void * pExtensionData_;
	
	PacketReceiver * pPacketReceiver_;
};

}
}

#ifdef CODE_INLINE
#include "network_interface.ipp"
#endif
#endif // __NETWORK_INTERFACE__