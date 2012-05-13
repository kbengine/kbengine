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
UDPPacket _g_receiveWindow;
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
	int len = _g_receiveWindow.recvFromEndPoint(endpoint_, &srcAddr);

	if (len <= 0)
	{
		return this->checkSocketErrors(len, expectingPacket);
	}
	
	Channel* pSrcChannel = networkInterface_.findChannel(srcAddr);
	if(pSrcChannel == NULL) 
	{
		EndPoint* pNewEndPoint = new EndPoint(srcAddr.ip, srcAddr.port);
		pSrcChannel = new Channel(networkInterface_, pNewEndPoint, Channel::EXTERNAL, PROTOCOL_UDP);
		if(!networkInterface_.registerChannel(pSrcChannel))
		{
			ERROR_MSG("UDPPacketReceiver::processSocket:registerChannel(%s) is failed!\n",
				pSrcChannel->c_str());
			return false;
		}
	}
	
	KBE_ASSERT(pSrcChannel != NULL);
	Packet* pChannelReceiveWindow = pSrcChannel->receiveWindow();
	pChannelReceiveWindow->append(_g_receiveWindow.data(), _g_receiveWindow.wpos());
	Reason ret = this->processPacket(pSrcChannel, pChannelReceiveWindow);

	if(ret != REASON_SUCCESS)
		this->dispatcher().errorReporter().reportException(ret, endpoint_.addr());
	
	return true;
}

//-------------------------------------------------------------------------------------
Reason UDPPacketReceiver::processFilteredPacket(Channel* pChannel, Packet * pPacket)
{
	networkInterface_.onPacketIn(*pPacket);
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
bool UDPPacketReceiver::checkSocketErrors(int len, bool expectingPacket)
{
	if (len == 0)
	{
		WARNING_MSG( "PacketReceiver::processPendingEvents: "
			"Throwing REASON_GENERAL_NETWORK (1)- %s\n",
			strerror( errno ) );

		this->dispatcher().errorReporter().reportException(
				REASON_GENERAL_NETWORK );

		return true;
	}
	
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
				// Î´ÊµÏÖ
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
			kbe_strerror());
#endif
	this->dispatcher().errorReporter().reportException(
			REASON_GENERAL_NETWORK);

	return true;
}

//-------------------------------------------------------------------------------------
}
}