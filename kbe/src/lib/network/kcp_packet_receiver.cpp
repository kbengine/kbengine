// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "kcp_packet_receiver.h"
#ifndef CODE_INLINE
#include "kcp_packet_receiver.inl"
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
static ObjectPool<KCPPacketReceiver> _g_objPool("KCPPacketReceiver");
ObjectPool<KCPPacketReceiver>& KCPPacketReceiver::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
KCPPacketReceiver* KCPPacketReceiver::createPoolObject(const std::string& logPoint)
{
	return _g_objPool.createObject(logPoint);
}

//-------------------------------------------------------------------------------------
void KCPPacketReceiver::reclaimPoolObject(KCPPacketReceiver* obj)
{
	_g_objPool.reclaimObject(obj);
}

//-------------------------------------------------------------------------------------
void KCPPacketReceiver::destroyObjPool()
{
	DEBUG_MSG(fmt::format("KCPPacketReceiver::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
KCPPacketReceiver::SmartPoolObjectPtr KCPPacketReceiver::createSmartPoolObj(const std::string& logPoint)
{
	return SmartPoolObjectPtr(new SmartPoolObject<KCPPacketReceiver>(ObjPool().createObject(logPoint), _g_objPool));
}

//-------------------------------------------------------------------------------------
KCPPacketReceiver::KCPPacketReceiver(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	UDPPacketReceiver(endpoint, networkInterface)
{
}

//-------------------------------------------------------------------------------------
KCPPacketReceiver::~KCPPacketReceiver()
{
}

//-------------------------------------------------------------------------------------
bool KCPPacketReceiver::processRecv(bool expectingPacket)
{	
	return UDPPacketReceiver::processRecv(expectingPacket);
}

//-------------------------------------------------------------------------------------
bool KCPPacketReceiver::processRecv(UDPPacket* pReceiveWindow)
{
	Channel* pChannel = getChannel();
	if (pChannel && pChannel->condemn() > 0)
	{
		return false;
	}

	Reason ret = this->processPacket(pChannel, pReceiveWindow);

	if (ret != REASON_SUCCESS)
		this->dispatcher().errorReporter().reportException(ret, pEndpoint_->addr());

	return true;
}

//-------------------------------------------------------------------------------------
Reason KCPPacketReceiver::processPacket(Channel* pChannel, Packet * pPacket)
{
	if (pChannel != NULL && pChannel->hasHandshake())
	{
		pChannel->addKcpUpdate();

		if (ikcp_input(pChannel->pKCP(), (const char*)pPacket->data(), pPacket->length()) < 0)
		{
			RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);
			return REASON_CHANNEL_LOST;
		}

		RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);

		while (true)
		{
			Packet* pRcvdUDPPacket = UDPPacket::createPoolObject(OBJECTPOOL_POINT);
			int bytes_recvd = ikcp_recv(pChannel->pKCP(), (char*)pRcvdUDPPacket->data(), pRcvdUDPPacket->size());
			if (bytes_recvd < 0)
			{
				//WARNING_MSG(fmt::format("KCPPacketReceiver::processPacket(): recvd_bytes({}) <= 0! addr={}\n", bytes_recvd, pChannel->c_str()));
				RECLAIM_PACKET(pRcvdUDPPacket->isTCPPacket(), pRcvdUDPPacket);
				return REASON_SUCCESS;
			}
			else
			{
				if (bytes_recvd >= (int)pRcvdUDPPacket->size())
				{
					ERROR_MSG(fmt::format("KCPPacketReceiver::processPacket(): recvd_bytes({}) >= maxBuf({})! addr={}\n", bytes_recvd, pRcvdUDPPacket->size(), pChannel->c_str()));
				}

				pRcvdUDPPacket->wpos(bytes_recvd);

				Reason r = PacketReceiver::processPacket(pChannel, pRcvdUDPPacket);
				if (r != REASON_SUCCESS)
				{
					RECLAIM_PACKET(pRcvdUDPPacket->isTCPPacket(), pRcvdUDPPacket);
					return r;
				}
			}
		}
	}
	else
	{
		return PacketReceiver::processPacket(pChannel, pPacket);
	}

	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
}
}
