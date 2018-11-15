// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "udp_packet_receiver.h"
#ifndef CODE_INLINE
#include "udp_packet_receiver.inl"
#endif

#include "network/address.h"
#include "network/bundle.h"
#include "network/channel.h"
#include "network/endpoint.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/event_poller.h"
#include "network/error_reporter.h"

namespace KBEngine { 
namespace Network
{

//-------------------------------------------------------------------------------------
static ObjectPool<UDPPacketReceiver> _g_objPool("UDPPacketReceiver");
ObjectPool<UDPPacketReceiver>& UDPPacketReceiver::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
UDPPacketReceiver* UDPPacketReceiver::createPoolObject(const std::string& logPoint)
{
	return _g_objPool.createObject(logPoint);
}

//-------------------------------------------------------------------------------------
void UDPPacketReceiver::reclaimPoolObject(UDPPacketReceiver* obj)
{
	_g_objPool.reclaimObject(obj);
}

//-------------------------------------------------------------------------------------
void UDPPacketReceiver::destroyObjPool()
{
	DEBUG_MSG(fmt::format("UDPPacketReceiver::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
UDPPacketReceiver::SmartPoolObjectPtr UDPPacketReceiver::createSmartPoolObj(const std::string& logPoint)
{
	return SmartPoolObjectPtr(new SmartPoolObject<UDPPacketReceiver>(ObjPool().createObject(logPoint), _g_objPool));
}

//-------------------------------------------------------------------------------------
UDPPacketReceiver::UDPPacketReceiver(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	PacketReceiver(endpoint, networkInterface)
{
}

//-------------------------------------------------------------------------------------
UDPPacketReceiver::~UDPPacketReceiver()
{
}

//-------------------------------------------------------------------------------------
Channel* UDPPacketReceiver::findChannel(const Address& addr)
{
	return pNetworkInterface_->findChannel(addr);
}

//-------------------------------------------------------------------------------------
bool UDPPacketReceiver::processRecv(bool expectingPacket)
{	
	Address	srcAddr;
	UDPPacket* pChannelReceiveWindow = UDPPacket::createPoolObject(OBJECTPOOL_POINT);
	int len = pChannelReceiveWindow->recvFromEndPoint(*pEndpoint_, &srcAddr);

	if (len <= 0)
	{
		UDPPacket::reclaimPoolObject(pChannelReceiveWindow);
		PacketReceiver::RecvState rstate = this->checkSocketErrors(len, expectingPacket);
		return rstate == PacketReceiver::RECV_STATE_CONTINUE;
	}
	
	Channel* pSrcChannel = findChannel(srcAddr);

	if(pSrcChannel == NULL) 
	{
		EndPoint* pNewEndPoint = EndPoint::createPoolObject(OBJECTPOOL_POINT);
		pNewEndPoint->addr(srcAddr);
		pNewEndPoint->setSocketRef(pEndpoint_->socket());

		pSrcChannel = Network::Channel::createPoolObject(OBJECTPOOL_POINT);
		bool ret = pSrcChannel->initialize(*pNetworkInterface_, pNewEndPoint, Channel::EXTERNAL, PROTOCOL_UDP, protocolSubType());
		if(!ret)
		{
			ERROR_MSG(fmt::format("UDPPacketReceiver::processRecv: initialize({}) is failed!\n",
				pSrcChannel->c_str()));

			if (pSrcChannel->pEndPoint() != pNewEndPoint)
				EndPoint::reclaimPoolObject(pNewEndPoint);

			pSrcChannel->destroy();
			Network::Channel::reclaimPoolObject(pSrcChannel);
			UDPPacket::reclaimPoolObject(pChannelReceiveWindow);
			return false;
		}

		if(!pNetworkInterface_->registerChannel(pSrcChannel))
		{
			ERROR_MSG(fmt::format("UDPPacketReceiver::processRecv: registerChannel({}) is failed!\n",
				pSrcChannel->c_str()));

			UDPPacket::reclaimPoolObject(pChannelReceiveWindow);
			pSrcChannel->destroy();
			Network::Channel::reclaimPoolObject(pSrcChannel);
			return false;
		}
	}
	
	KBE_ASSERT(pSrcChannel != NULL);

	if (pSrcChannel->condemn() > 0)
	{
		UDPPacket::reclaimPoolObject(pChannelReceiveWindow);
		return false;
	}
	
	return ((UDPPacketReceiver*)pSrcChannel->pPacketReceiver())->processRecv(pChannelReceiveWindow);
}

//-------------------------------------------------------------------------------------
bool UDPPacketReceiver::processRecv(UDPPacket* pReceiveWindow)
{
	Reason ret = this->processPacket(getChannel(), pReceiveWindow);

	if (ret != REASON_SUCCESS)
		this->dispatcher().errorReporter().reportException(ret, pEndpoint_->addr());

	return true;
}

//-------------------------------------------------------------------------------------
Reason UDPPacketReceiver::processFilteredPacket(Channel* pChannel, Packet * pPacket)
{
	// 如果为None， 则可能是被过滤器过滤掉了(过滤器正在按照自己的规则组包解密)
	if(pPacket)
	{
		pChannel->addReceiveWindow(pPacket);
	}

	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
PacketReceiver::RecvState UDPPacketReceiver::checkSocketErrors(int len, bool expectingPacket)
{
	if (len == 0)
	{
		WARNING_MSG(fmt::format("PacketReceiver::processPendingEvents: "
			"Throwing REASON_GENERAL_NETWORK (1)- {}\n",
			strerror( errno )));

		this->dispatcher().errorReporter().reportException(
				REASON_GENERAL_NETWORK );

		return RECV_STATE_CONTINUE;
	}
	
#ifdef _WIN32
	DWORD wsaErr = WSAGetLastError();
#endif //def _WIN32

	if (
#ifdef _WIN32
		wsaErr == WSAEWOULDBLOCK && !expectingPacket
#else
		errno == EAGAIN && !expectingPacket
#endif
		)
	{
		return RECV_STATE_BREAK;
	}

#if KBE_PLATFORM == PLATFORM_UNIX
	if (errno == EAGAIN ||
		errno == ECONNREFUSED ||
		errno == EHOSTUNREACH)
	{
		Network::Address offender;

		if (pEndpoint_->getClosedPort(offender))
		{
			// If we got a NO_SUCH_PORT error and there is an internal
			// channel to this address, mark it as remote failed.  The logic
			// for dropping external channels that get NO_SUCH_PORT
			// exceptions is built into BaseApp::onClientNoSuchPort().
			if (errno == ECONNREFUSED)
			{
				// 未实现
			}

			this->dispatcher().errorReporter().reportException(
					REASON_NO_SUCH_PORT, offender);

			return RECV_STATE_CONTINUE;
		}
		else
		{
			WARNING_MSG("UDPPacketReceiver::processPendingEvents: "
				"getClosedPort() failed\n");
		}
	}
#else
	if (wsaErr == WSAECONNRESET)
	{
		return RECV_STATE_CONTINUE;
	}
#endif // unix

#ifdef _WIN32
	WARNING_MSG(fmt::format("UDPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - {}\n",
				wsaErr));
#else
	WARNING_MSG(fmt::format("UDPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - {}\n",
			kbe_strerror()));
#endif
	this->dispatcher().errorReporter().reportException(
			REASON_GENERAL_NETWORK);

	return RECV_STATE_CONTINUE;
}

//-------------------------------------------------------------------------------------
}
}
