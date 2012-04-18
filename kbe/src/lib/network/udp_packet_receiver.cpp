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
namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
UDPPacketReceiver::UDPPacketReceiver(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	PacketReceiver(endpoint, networkInterface),
	pNextPacket_(new TCPPacket())
{
}

//-------------------------------------------------------------------------------------
UDPPacketReceiver::~UDPPacketReceiver()
{
}


//-------------------------------------------------------------------------------------
bool UDPPacketReceiver::processSocket(bool expectingPacket)
{
	int len = pNextPacket_->recvFromEndPoint(endpoint_);

	if (len < 0)
	{
		return this->checkSocketErrors(len, expectingPacket);
	}
	else if(len == 0) // 客户端正常退出
	{
		Channel* pChannel = networkInterface_.findChannel(endpoint_.addr());
		KBE_ASSERT(pChannel != NULL);
		networkInterface_.deregisterChannel(pChannel);
		pChannel->destroy();
		return false;
	}
	
	PacketPtr curPacket = pNextPacket_;
	pNextPacket_ = new TCPPacket();
	Reason ret = this->processPacket(curPacket.get());

	if(ret != REASON_SUCCESS)
		this->dispatcher().errorReporter().reportException(ret, endpoint_.addr());
	
	return true;
}

//-------------------------------------------------------------------------------------
Reason UDPPacketReceiver::processFilteredPacket(Packet * p)
{
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
bool UDPPacketReceiver::checkSocketErrors(int len, bool expectingPacket)
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
#if defined(PLAYSTATION3)
		this->dispatcher().errorReporter().reportException(
				REASON_NO_SUCH_PORT);
		return true;
#else
		Mercury::Address offender;

		if (endpoint_.getClosedPort(offender))
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

			return true;
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
		return true;
	}
#endif // unix

	// ok, I give up, something's wrong
#ifdef _WIN32
	WARNING_MSG("UDPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %d\n",
				wsaErr);
#else
	WARNING_MSG("UDPPacketReceiver::processPendingEvents: "
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