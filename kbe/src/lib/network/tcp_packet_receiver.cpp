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
TCPPacketReceiver::TCPPacketReceiver(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	PacketReceiver(endpoint, networkInterface)
{
}

//-------------------------------------------------------------------------------------
TCPPacketReceiver::~TCPPacketReceiver()
{
}


//-------------------------------------------------------------------------------------
bool TCPPacketReceiver::processSocket(bool expectingPacket)
{
	Channel* pChannel = networkInterface_.findChannel(endpoint_.addr());
	KBE_ASSERT(pChannel != NULL);
	
	Packet* pReceiveWindow = pChannel->receiveWindow();
	int len = pReceiveWindow->recvFromEndPoint(endpoint_);

	if (len < 0)
	{
		return this->checkSocketErrors(len, expectingPacket);
	}
	else if(len == 0) // 客户端正常退出
	{
		networkInterface_.deregisterChannel(pChannel);
		pChannel->destroy();
		return false;
	}
	
	Reason ret = this->processPacket(pChannel, pReceiveWindow);

	if(ret != REASON_SUCCESS)
		this->dispatcher().errorReporter().reportException(ret, endpoint_.addr());
	
	return true;
}

//-------------------------------------------------------------------------------------
Reason TCPPacketReceiver::processFilteredPacket(Channel* pChannel, Packet * pPacket)
{
	networkInterface_.onPacketIn(*pPacket);
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
bool TCPPacketReceiver::checkSocketErrors(int len, bool expectingPacket)
{
#ifdef _WIN32
	DWORD wsaErr = WSAGetLastError();
#endif //def _WIN32

	// is the buffer empty?
	if (
#ifdef _WIN32
		wsaErr == WSAEWOULDBLOCK
#else
		errno == EAGAIN && !expectingPacket
#endif
		)
	{
		return false;
	}

#ifdef unix
	// is it telling us there's an error?
	if (errno == EAGAIN ||
		errno == ECONNREFUSED ||
		errno == EHOSTUNREACH)
	{
		this->dispatcher().errorReporter().reportException(
				REASON_NO_SUCH_PORT);
		return false;
	}
#else
	if (wsaErr == WSAECONNRESET)
	{
		return false;
	}
#endif // unix

	// ok, I give up, something's wrong
#ifdef _WIN32
	WARNING_MSG("TCPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %d\n",
				wsaErr);
#else
	WARNING_MSG("TCPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %s\n",
			strerror(errno));
#endif
	this->dispatcher().errorReporter().reportException(
			REASON_GENERAL_NETWORK);

	return true;
}

//-------------------------------------------------------------------------------------
}
}