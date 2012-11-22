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


#include "udp_packet_receiver.hpp"
#ifndef CODE_INLINE
#include "udp_packet_receiver.ipp"
#endif

#include "network/address.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "network/endpoint.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/event_poller.hpp"
#include "network/error_reporter.hpp"

namespace KBEngine { 
namespace Mercury
{

//-------------------------------------------------------------------------------------
static ObjectPool<UDPPacketReceiver> _g_objPool;
ObjectPool<UDPPacketReceiver>& UDPPacketReceiver::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
UDPPacketReceiver::SmartPoolObjectPtr UDPPacketReceiver::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<UDPPacketReceiver>(ObjPool().createObject(), _g_objPool));
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
bool UDPPacketReceiver::processSocket(bool expectingPacket)
{
//	Channel* pChannel = networkInterface_.findChannel(endpoint_.addr());
//	KBE_ASSERT(pChannel != NULL);
	
	Address	srcAddr;
	UDPPacket* pChannelReceiveWindow = UDPPacket::ObjPool().createObject();
	int len = pChannelReceiveWindow->recvFromEndPoint(*pEndpoint_, &srcAddr);

	if (len <= 0)
	{
		UDPPacket::ObjPool().reclaimObject(pChannelReceiveWindow);
		PacketReceiver::RecvState rstate = this->checkSocketErrors(len, expectingPacket);
		return rstate == PacketReceiver::RECV_STATE_CONTINUE;
	}
	
	Channel* pSrcChannel = pNetworkInterface_->findChannel(srcAddr);

	if(pSrcChannel == NULL) 
	{
		EndPoint* pNewEndPoint = new EndPoint(srcAddr.ip, srcAddr.port);
		pSrcChannel = new Channel(*pNetworkInterface_, pNewEndPoint, Channel::EXTERNAL, PROTOCOL_UDP);

		if(!pNetworkInterface_->registerChannel(pSrcChannel))
		{
			ERROR_MSG("UDPPacketReceiver::processSocket:registerChannel(%s) is failed!\n",
				pSrcChannel->c_str());

			UDPPacket::ObjPool().reclaimObject(pChannelReceiveWindow);
			pSrcChannel->destroy();
			return false;
		}
	}
	
	KBE_ASSERT(pSrcChannel != NULL);

	if(pSrcChannel->isCondemn())
	{
		UDPPacket::ObjPool().reclaimObject(pChannelReceiveWindow);
		pNetworkInterface_->deregisterChannel(pSrcChannel);
		pSrcChannel->destroy();
		return false;
	}

	pSrcChannel->addReceiveWindow(pChannelReceiveWindow);
	Reason ret = this->processPacket(pSrcChannel, pChannelReceiveWindow);

	if(ret != REASON_SUCCESS)
		this->dispatcher().errorReporter().reportException(ret, pEndpoint_->addr());
	
	return true;
}

//-------------------------------------------------------------------------------------
Reason UDPPacketReceiver::processFilteredPacket(Channel* pChannel, Packet * pPacket)
{
	pNetworkInterface_->onPacketIn(*pPacket);
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
PacketReceiver::RecvState UDPPacketReceiver::checkSocketErrors(int len, bool expectingPacket)
{
	if (len == 0)
	{
		WARNING_MSG( "PacketReceiver::processPendingEvents: "
			"Throwing REASON_GENERAL_NETWORK (1)- %s\n",
			strerror( errno ) );

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

#ifdef unix
	if (errno == EAGAIN ||
		errno == ECONNREFUSED ||
		errno == EHOSTUNREACH)
	{
#if defined(PLAYSTATION3)
		this->dispatcher().errorReporter().reportException(
				REASON_NO_SUCH_PORT);

		return RECV_STATE_CONTINUE;
#else
		Mercury::Address offender;

		if (pEndpoint_->getClosedPort(offender))
		{
			// If we got a NO_SUCH_PORT error and there is an internal
			// channel to this address, mark it as remote failed.  The logic
			// for dropping external channels that get NO_SUCH_PORT
			// exceptions is built into BaseApp::onClientNoSuchPort().
			if (errno == ECONNREFUSED)
			{
				// Î´ÊµÏÖ
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
#endif
	}
#else
	if (wsaErr == WSAECONNRESET)
	{
		return RECV_STATE_CONTINUE;
	}
#endif // unix

#ifdef _WIN32
	WARNING_MSG("UDPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %d\n",
				wsaErr);
#else
	WARNING_MSG("UDPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %s\n",
			kbe_strerror());
#endif
	this->dispatcher().errorReporter().reportException(
			REASON_GENERAL_NETWORK);

	return RECV_STATE_CONTINUE;
}

//-------------------------------------------------------------------------------------
}
}
