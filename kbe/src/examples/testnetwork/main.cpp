/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#include "cstdkbe/cstdkbe.hpp"
#include "network/address.hpp"
#include "network/socket.hpp"
#include "network/event_poller.hpp"
#include "helper/debug_helper.hpp"
#include "network/event_dispatcher.hpp"
#include "network/interfaces.hpp"
#include "network/Packet.hpp"

using namespace KBEngine;
using namespace KBEngine::Mercury;
Address address;
Socket mysocket;
EventDispatcher gdispatcher;


class MyPacketReceiver : public InputNotificationHandler
{
public:
	MyPacketReceiver(Socket & mysocket):
	socket_(mysocket),
	pNextPacket_(new Packet())
	{
	}
	
	~MyPacketReceiver()
	{
	}
	
	EventDispatcher& dispatcher(){return gdispatcher;}
private:
	virtual int handleInputNotification(int fd)
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
	
	bool processSocket(bool expectingPacket)
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

		if (ret != REASON_SUCCESS)
		{
			this->dispatcher().errorReporter().reportException(ret, srcAddr);
		}
		return true;
	}

	Reason processPacket(const Address & addr, Packet * p)
	{
		return REASON_SUCCESS;
	}
	
	bool checkSocketErrors(int len, bool expectingPacket)
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
private:
	Socket & socket_;
	PacketPtr pNextPacket_;
};

MyPacketReceiver* packetReceiver;

void init_network(void)
{
	mysocket.socket(SOCK_STREAM);
	if (!mysocket.good())
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket: couldn't create a socket\n");
		return;
	}
	
	mysocket.setnodelay(true);
	packetReceiver = new MyPacketReceiver(mysocket);
	gdispatcher.registerFileDescriptor(mysocket, packetReceiver);
	
	if(mysocket.connect(htons(49312), inet_addr("192.168.1.104")) == -1)
	{
		ERROR_MSG("NetworkInterface::recreateListeningSocket: connect server is error!\n");
		return;
	}
}

int main(int argc, char* argv[])
{
	init_network();
	gdispatcher.processUntilBreak();
	getchar();
	return 0; 
}
