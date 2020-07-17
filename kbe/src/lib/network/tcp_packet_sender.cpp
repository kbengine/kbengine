// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
TCPPacketSender* TCPPacketSender::createPoolObject(const std::string& logPoint)
{
	return _g_objPool.createObject(logPoint);
}

//-------------------------------------------------------------------------------------
void TCPPacketSender::reclaimPoolObject(TCPPacketSender* obj)
{
	_g_objPool.reclaimObject(obj);
}

//-------------------------------------------------------------------------------------
void TCPPacketSender::onReclaimObject()
{
	sendfailCount_ = 0;
}

//-------------------------------------------------------------------------------------
void TCPPacketSender::destroyObjPool()
{
	DEBUG_MSG(fmt::format("TCPPacketSender::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
TCPPacketSender::SmartPoolObjectPtr TCPPacketSender::createSmartPoolObj(const std::string& logPoint)
{
	return SmartPoolObjectPtr(new SmartPoolObject<TCPPacketSender>(ObjPool().createObject(logPoint), _g_objPool));
}

//-------------------------------------------------------------------------------------
TCPPacketSender::TCPPacketSender(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	PacketSender(endpoint, networkInterface),
	sendfailCount_(0)
{
}

//-------------------------------------------------------------------------------------
TCPPacketSender::~TCPPacketSender()
{
	//DEBUG_MSG("TCPPacketSender::~TCPPacketSender()\n");
}

//-------------------------------------------------------------------------------------
void TCPPacketSender::onGetError(Channel* pChannel, const std::string& err)
{
	pChannel->condemn(err);
	
	// 此处不必立即销毁，可能导致bufferedReceives_内部遍历迭代器破坏
	// 交给TCPPacketReceiver处理即可
	//pChannel->networkInterface().deregisterChannel(pChannel);
	//pChannel->destroy();
}

//-------------------------------------------------------------------------------------
bool TCPPacketSender::processSend(Channel* pChannel, int userarg)
{
	bool noticed = pChannel == NULL;

	// 如果是由poller通知的，我们需要通过地址找到channel
	if(noticed)
		pChannel = getChannel();

	KBE_ASSERT(pChannel != NULL);
	
	if(pChannel->condemn() == Channel::FLAG_CONDEMN_AND_DESTROY)
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
			reason = processPacket(pChannel, (*iter1), userarg);
			if(reason != REASON_SUCCESS)
				break; 
			else
				RECLAIM_PACKET((*iter)->isTCPPacket(), (*iter1));
		}

		if(reason == REASON_SUCCESS)
		{
			pakcets.clear();
			Network::Bundle::reclaimPoolObject((*iter));
			sendfailCount_ = 0;
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

				// 连续超过10次则通知出错
				if (++sendfailCount_ >= 10 && pChannel->isExternal())
				{
					onGetError(pChannel, "TCPPacketSender::processSend: sendfailCount >= 10");

					this->dispatcher().errorReporter().reportException(reason, pEndpoint_->addr(), 
						fmt::format("TCPPacketSender::processSend(external, sendfailCount({}) >= 10)", (int)sendfailCount_).c_str());
				}
				else
				{
					this->dispatcher().errorReporter().reportException(reason, pEndpoint_->addr(), 
						fmt::format("TCPPacketSender::processSend({}}, {})", (pChannel->isInternal() ? "internal" : "external"), (int)sendfailCount_).c_str());
				}
			}
			else
			{
				if (pChannel->isExternal())
				{
#if KBE_PLATFORM == PLATFORM_UNIX
					this->dispatcher().errorReporter().reportException(reason, pEndpoint_->addr(), "TCPPacketSender::processSend(external)",
						fmt::format(", errno: {}", errno).c_str());
#else
					this->dispatcher().errorReporter().reportException(reason, pEndpoint_->addr(), "TCPPacketSender::processSend(external)",
						fmt::format(", errno: {}", WSAGetLastError()).c_str());
#endif
				}
				else
				{
#if KBE_PLATFORM == PLATFORM_UNIX
					this->dispatcher().errorReporter().reportException(reason, pEndpoint_->addr(), "TCPPacketSender::processSend(internal)",
						fmt::format(", errno: {}, {}", errno, pChannel->c_str()).c_str());
#else
					this->dispatcher().errorReporter().reportException(reason, pEndpoint_->addr(), "TCPPacketSender::processSend(internal)",
						fmt::format(", errno: {}, {}", WSAGetLastError(), pChannel->c_str()).c_str());
#endif
				}

				onGetError(pChannel, fmt::format("TCPPacketSender::processSend: errno={}", kbe_lasterror()));
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
Reason TCPPacketSender::processFilterPacket(Channel* pChannel, Packet * pPacket, int userarg)
{
	if(pChannel->condemn() == Channel::FLAG_CONDEMN_AND_DESTROY)
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
	{
		return REASON_SUCCESS;
	}
	else
	{
		// 如果只发送了一部分数据，则认为是REASON_RESOURCE_UNAVAILABLE
		if (len > 0)
			return REASON_RESOURCE_UNAVAILABLE;
	}

	return checkSocketErrors(pEndpoint);
}

//-------------------------------------------------------------------------------------
}
}

