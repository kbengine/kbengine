#include "packet_receiver.hpp"
#ifndef CODE_INLINE
#include "packet_receiver.ipp"
#endif

#include "network/address.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "network/socket.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"

namespace KBEngine { 
namespace Mercury
{

PacketReceiver::PacketReceiver(Socket & socket,
	   NetworkInterface & networkInterface	) :
	socket_(socket),
	networkInterface_(networkInterface),
	pNextPacket_(new Packet())
{
}


/**
 *	Destructor.
 */
PacketReceiver::~PacketReceiver()
{
}


/**
 *	This method is called when their is data on the socket.
 */
int PacketReceiver::handleInputNotification(int fd)
{
	if (this->processSocket(/*expectingPacket:*/true))
	{
		while (this->processSocket(/*expectingPacket:*/false))
		{
			/* pass */;
		}
	}

	return 0;
}


/**
 *	This method will read and process any pending data on this object's socket.
 */
bool PacketReceiver::processSocket(bool expectingPacket)
{
	int len = pNextPacket_->recvFromEndPoint(socket_);
	if (len <= 0)
	{
		return this->checkSocketErrors(len, expectingPacket);
	}

	PacketPtr curPacket = pNextPacket_;
	pNextPacket_ = new Packet();
	Address srcAddr = socket_.getRemoteAddress();
	Reason ret = this->processPacket(srcAddr, curPacket.get());

	if ((ret != REASON_SUCCESS) &&
			networkInterface_.isVerbose())
	{
		this->dispatcher().errorReporter().reportException(ret, srcAddr);
	}

	return true;
}


/**
 *	This method checks whether an error was received from a call to
 */
bool PacketReceiver::checkSocketErrors(int len, bool expectingPacket)
{
	// is len weird?
	if (len == 0)
	{
		WARNING_MSG("PacketReceiver::processPendingEvents: "
			"Throwing REASON_GENERAL_NETWORK (1)- %s\n",
			strerror(errno));

		this->dispatcher().errorReporter().reportException(
				REASON_GENERAL_NETWORK);

		return true;
	}
		// I'm not quite sure what it means if len is 0
		// (0 => 'end of file', but with dgram sockets?)

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

		if (socket_.getClosedPort(offender))
		{
			// If we got a NO_SUCH_PORT error and there is an internal
			// channel to this address, mark it as remote failed.  The logic
			// for dropping external channels that get NO_SUCH_PORT
			// exceptions is built into BaseApp::onClientNoSuchPort().
			if (errno == ECONNREFUSED)
			{
				Channel * pDeadChannel = 
					networkInterface_.findChannel(offender);

				if (pDeadChannel &&
						pDeadChannel->isInternal())
				{
					INFO_MSG("PacketReceiver::processPendingEvents: "
						"Marking channel to %s as dead (%s)\n",
						pDeadChannel->c_str(),
						reasonToString(REASON_NO_SUCH_PORT));
				}
			}

			this->dispatcher().errorReporter().reportException(
					REASON_NO_SUCH_PORT, offender);

			return true;
		}
		else
		{
			WARNING_MSG("PacketReceiver::processPendingEvents: "
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
	WARNING_MSG("PacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %d\n",
				wsaErr);
#else
	WARNING_MSG("PacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - %s\n",
			strerror(errno));
#endif
	this->dispatcher().errorReporter().reportException(
			REASON_GENERAL_NETWORK);

	return true;
}


/**
 *	This is the entrypoint for new packets, which just gives it to the filter.
 */
Reason PacketReceiver::processPacket(const Address & addr, Packet * p)
{
	// Packets arriving on external interface will probably be encrypted, so
	// there's no point examining their header flags right now.
	Channel * pChannel = networkInterface_.findChannel(addr);

	if (pChannel != NULL)
	{
		// We update received times for addressed channels here.  Indexed
		// channels are done in processFilteredPacket().
		pChannel->onPacketReceived(p->totalSize());

		if (pChannel->pFilter())
		{
			// let the filter decide what to do with it
			return pChannel->pFilter()->recv(*this, addr, p);
		}
	}

	return this->processFilteredPacket(addr, p);
}


/**
 *	This function has to be very robust, if we intend to use this transport over
 *	the big bad internet. We basically have to assume it'll be complete garbage.
 */
Reason PacketReceiver::processFilteredPacket(const Address & addr,
		Packet * p)
{
	return REASON_SUCCESS;
}

/**
*	This method returns the dispatcher that is used by this receiver.
*/
EventDispatcher & PacketReceiver::dispatcher()
{
	return networkInterface_.dispatcher();
}

}
}