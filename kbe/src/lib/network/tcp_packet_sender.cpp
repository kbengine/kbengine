/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "tcp_packet_sender.h"
#ifndef CODE_INLINE
#include "tcp_packet_sender.inl"
#endif

#include "network/address.h"
#include "network/bundle.h"
#include "network/channel.h"
#include "network/endpoint.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/event_poller.h"
#include "network/error_reporter.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"

namespace KBEngine { 
namespace Network
{

//-------------------------------------------------------------------------------------
static ObjectPool<TCPPacketSender> _g_objPool("TCPPacketSender");
ObjectPool<TCPPacketSender>& TCPPacketSender::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
void TCPPacketSender::destroyObjPool()
{
	DEBUG_MSG(fmt::format("TCPPacketSender::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
TCPPacketSender::SmartPoolObjectPtr TCPPacketSender::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<TCPPacketSender>(ObjPool().createObject(), _g_objPool));
}

//-------------------------------------------------------------------------------------
TCPPacketSender::TCPPacketSender(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	PacketSender(endpoint, networkInterface)
{
}

//-------------------------------------------------------------------------------------
TCPPacketSender::~TCPPacketSender()
{
	//DEBUG_MSG("TCPPacketSender::~TCPPacketSender()\n");
}

//-------------------------------------------------------------------------------------
void TCPPacketSender::onGetError(Channel* pChannel)
{
	pChannel->condemn();
}

//-------------------------------------------------------------------------------------
bool TCPPacketSender::processSend(Channel* pChannel)
{
	bool noticed = pChannel == NULL;

	// 如果是有poller通知的，我们需要通过地址找到channel
	if(noticed)
		pChannel = getChannel();

	KBE_ASSERT(pChannel != NULL);
	
	if(pChannel->isCondemn())
	{
		return false;
	}
	
	Channel::Bundles& bundles = pChannel->bundles();
	Reason reason = REASON_SUCCESS;

	Channel::Bundles::iterator iter = bundles.begin();
	for(; iter != bundles.end(); ++iter)
	{
		Bundle::Packets& pakcets = (*iter)->packets();
		Bundle::Packets::iterator iter1 = pakcets.begin();
		for (; iter1 != pakcets.end(); ++iter1)
		{
			reason = processPacket(pChannel, (*iter1));
			if(reason != REASON_SUCCESS)
				break; 
			else
				RECLAIM_PACKET((*iter)->isTCPPacket(), (*iter1));
		}

		if(reason == REASON_SUCCESS)
		{
			pakcets.clear();
			Network::Bundle::ObjPool().reclaimObject((*iter));
		}
		else
		{
			pakcets.erase(pakcets.begin(), iter1);
			bundles.erase(bundles.begin(), iter);

			if (reason == REASON_RESOURCE_UNAVAILABLE)
			{
				/* 此处输出可能会造成debugHelper处死锁
					WARNING_MSG(fmt::format("TCPPacketSender::processSend: "
						"Transmit queue full, waiting for space(kbengine.xml->channelCommon->writeBufferSize->{})...\n",
						(pChannel->isInternal() ? "internal" : "external")));
				*/

				this->dispatcher().errorReporter().reportException(reason, pEndpoint_->addr());
			}
			else
			{
				this->dispatcher().errorReporter().reportException(reason, pEndpoint_->addr());
				onGetError(pChannel);
			}

			return false;
		}
	}

	bundles.clear();

	if(noticed)
		pChannel->onSendCompleted();

	return true;
}

//-------------------------------------------------------------------------------------
Reason TCPPacketSender::processFilterPacket(Channel* pChannel, Packet * pPacket)
{
	if(pChannel->isCondemn())
	{
		return REASON_CHANNEL_CONDEMN;
	}

	EndPoint* pEndpoint = pChannel->pEndPoint();
	int len = pEndpoint->send(pPacket->data() + pPacket->sentSize, pPacket->length() - pPacket->sentSize);

	if(len > 0)
	{
		pPacket->sentSize += len;
		// DEBUG_MSG(fmt::format("TCPPacketSender::processFilterPacket: sent={}, sentTotalSize={}.\n", len, pPacket->sentSize));
	}

	bool sentCompleted = pPacket->sentSize == pPacket->length();
	pChannel->onPacketSent(len, sentCompleted);

	if (sentCompleted)
		return REASON_SUCCESS;

	return checkSocketErrors(pEndpoint);
}

//-------------------------------------------------------------------------------------
}
}

