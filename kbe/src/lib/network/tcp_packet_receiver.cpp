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


#include "tcp_packet_receiver.hpp"
#ifndef CODE_INLINE
#include "tcp_packet_receiver.ipp"
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
static ObjectPool<TCPPacketReceiver> _g_objPool;
ObjectPool<TCPPacketReceiver>& TCPPacketReceiver::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
TCPPacketReceiver::TCPPacketReceiver(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	PacketReceiver(endpoint, networkInterface)
{
}

//-------------------------------------------------------------------------------------
TCPPacketReceiver::~TCPPacketReceiver()
{
	//DEBUG_MSG("TCPPacketReceiver::~TCPPacketReceiver()\n");
}


//-------------------------------------------------------------------------------------
bool TCPPacketReceiver::processSocket(bool expectingPacket)
{
	Channel* pChannel = pNetworkInterface_->findChannel(pEndpoint_->addr());
	KBE_ASSERT(pChannel != NULL);
	
	TCPPacket* pReceiveWindow = TCPPacket::ObjPool().createObject();
	int len = pReceiveWindow->recvFromEndPoint(*pEndpoint_);

	if (len < 0)
	{
		TCPPacket::ObjPool().reclaimObject(pReceiveWindow);
		return this->checkSocketErrors(len, expectingPacket);
	}
	else if(len == 0) // 客户端正常退出
	{
		TCPPacket::ObjPool().reclaimObject(pReceiveWindow);
		pNetworkInterface_->deregisterChannel(pChannel);
		pChannel->destroy();
		return false;
	}
	
	pChannel->addReceiveWindow(pReceiveWindow);
	Reason ret = this->processPacket(pChannel, pReceiveWindow);

	if(ret != REASON_SUCCESS)
		this->dispatcher().errorReporter().reportException(ret, pEndpoint_->addr());
	
	return true;
}

//-------------------------------------------------------------------------------------
Reason TCPPacketReceiver::processFilteredPacket(Channel* pChannel, Packet * pPacket)
{
	pNetworkInterface_->onPacketIn(*pPacket);
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
bool TCPPacketReceiver::checkSocketErrors(int len, bool expectingPacket)
{
#ifdef _WIN32
	DWORD wsaErr = WSAGetLastError();
#endif //def _WIN32

	if (
#ifdef _WIN32
		wsaErr == WSAEWOULDBLOCK && !expectingPacket// send出错大概是缓冲区满了, recv出错已经无数据可读了
#else
		errno == EAGAIN && !expectingPacket			// recv缓冲区已经无数据可读了
#endif
		)
	{
		return false;
	}

#ifdef unix
	if (errno == EAGAIN ||							// 已经无数据可读了
		errno == ECONNREFUSED ||					// 连接被服务器拒绝
		errno == EHOSTUNREACH)						// 目的地址不可到达
	{
		this->dispatcher().errorReporter().reportException(
				REASON_NO_SUCH_PORT);
		return false;
	}
#else
	/*
	存在的连接被远程主机强制关闭。通常原因为：远程主机上对等方应用程序突然停止运行，或远程主机重新启动，
	或远程主机在远程方套接字上使用了“强制”关闭（参见setsockopt(SO_LINGER)）。
	另外，在一个或多个操作正在进行时，如果连接因“keep-alive”活动检测到一个失败而中断，也可能导致此错误。
	此时，正在进行的操作以错误码WSAENETRESET失败返回，后续操作将失败返回错误码WSAECONNRESET
	*/
	switch(wsaErr)
	{
	case WSAECONNRESET:
		WARNING_MSG("TCPPacketReceiver::processPendingEvents: "
					"Throwing REASON_GENERAL_NETWORK - WSAECONNRESET\n");
		return false;
	case WSAECONNABORTED:
		WARNING_MSG("TCPPacketReceiver::processPendingEvents: "
					"Throwing REASON_GENERAL_NETWORK - WSAECONNABORTED\n");
		return false;
	default:
		break;

	};

#endif // unix

#ifdef _WIN32
	WARNING_MSG("TCPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %d\n",
				wsaErr);
#else
	WARNING_MSG("TCPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %s\n",
			kbe_strerror());
#endif
	this->dispatcher().errorReporter().reportException(
			REASON_GENERAL_NETWORK);

	return true;
}

//-------------------------------------------------------------------------------------
}
}

