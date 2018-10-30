// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "packet_sender.h"
#ifndef CODE_INLINE
#include "packet_sender.inl"
#endif

#include "network/address.h"
#include "network/bundle.h"
#include "network/channel.h"
#include "network/endpoint.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/event_poller.h"
#include <openssl/err.h>

namespace KBEngine { 
namespace Network
{
//-------------------------------------------------------------------------------------
PacketSender::PacketSender() :
	pEndpoint_(NULL),
	pChannel_(NULL),
	pNetworkInterface_(NULL)
{
}

//-------------------------------------------------------------------------------------
PacketSender::PacketSender(EndPoint & endpoint,
	   NetworkInterface & networkInterface):
	pEndpoint_(&endpoint),
	pChannel_(NULL),
	pNetworkInterface_(&networkInterface)
{
}

//-------------------------------------------------------------------------------------
PacketSender::~PacketSender()
{
}

//-------------------------------------------------------------------------------------
Channel* PacketSender::getChannel()
{
	if (pChannel_)
	{
		if (pChannel_->isDestroyed())
			return NULL;

		return pChannel_;
	}

	pChannel_ = pNetworkInterface_->findChannel(pEndpoint_->addr());
	return pChannel_;
}

//-------------------------------------------------------------------------------------
int PacketSender::handleOutputNotification(int fd)
{
	processSend(NULL, 0);
	return 0;
}

//-------------------------------------------------------------------------------------
Reason PacketSender::processPacket(Channel* pChannel, Packet * pPacket, int userarg)
{
	if (pChannel != NULL)
	{
		if (pChannel->pFilter())
		{
			return pChannel->pFilter()->send(pChannel, *this, pPacket, userarg);
		}
	}

	return this->processFilterPacket(pChannel, pPacket, userarg);
}

//-------------------------------------------------------------------------------------
EventDispatcher & PacketSender::dispatcher()
{
	return pNetworkInterface_->dispatcher();
}

//-------------------------------------------------------------------------------------
Reason PacketSender::checkSocketErrors(const EndPoint * pEndpoint)
{
	int err;
	Reason reason;

#if KBE_PLATFORM == PLATFORM_UNIX
		err = errno;

		switch (err)
		{
			case ECONNREFUSED:	reason = REASON_NO_SUCH_PORT; break;
			case EAGAIN:		reason = REASON_RESOURCE_UNAVAILABLE; break;
			case EPIPE:			reason = REASON_CLIENT_DISCONNECTED; break;
			case ECONNRESET:	reason = REASON_CLIENT_DISCONNECTED; break;
			case ENOBUFS:		reason = REASON_TRANSMIT_QUEUE_FULL; break;
			default:			reason = REASON_GENERAL_NETWORK; break;
		}
#else
		err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK || err == WSAEINTR)
		{
			reason = REASON_RESOURCE_UNAVAILABLE;
		}
		else
		{
			switch (err)
			{
				case WSAECONNREFUSED:	reason = REASON_NO_SUCH_PORT; break;
				case WSAECONNRESET:	reason = REASON_CLIENT_DISCONNECTED; break;
				case WSAECONNABORTED:	reason = REASON_CLIENT_DISCONNECTED; break;
				default:reason = REASON_GENERAL_NETWORK;break;
			}
		}
#endif

	if (err == 0 && pEndpoint->isSSL())
	{
		long sslerr = ERR_get_error();
		if (sslerr > 0)
		{
			return REASON_WEBSOCKET_ERROR;
		}
	}

	return reason;
}

//-------------------------------------------------------------------------------------
}
}
