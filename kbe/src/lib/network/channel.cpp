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


#include "channel.hpp"
#ifndef CODE_INLINE
#include "channel.ipp"
#endif
#include "network/bundle.hpp"
#include "network/network_interface.hpp"
#include "network/tcp_packet_receiver.hpp"
#include "network/udp_packet_receiver.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"

namespace KBEngine { 
namespace Mercury
{
const int EXTERNAL_CHANNEL_SIZE = 256;
const int INTERNAL_CHANNEL_SIZE = 4096;
const int INDEXED_CHANNEL_SIZE = 512;

const float INACTIVITY_TIMEOUT_DEFAULT = 60.0;

Channel::Channel(NetworkInterface & networkInterface,
		const EndPoint * endpoint, Traits traits, ProtocolType pt,
		PacketFilterPtr pFilter, ChannelID id):
	pNetworkInterface_(&networkInterface),
	traits_(traits),
	protocoltype_(pt),
	id_(id),
	inactivityTimerHandle_(),
	inactivityExceptionPeriod_(0),
	lastReceivedTime_(0),
	pBundle_(NULL),
	windowSize_(	(traits != INTERNAL)    ? EXTERNAL_CHANNEL_SIZE :
					(id == CHANNEL_ID_NULL) ? INTERNAL_CHANNEL_SIZE :
											  INDEXED_CHANNEL_SIZE),

	bufferedReceives_(),
	pFragmentDatas_(NULL),
	pFragmentDatasWpos_(0),
	pFragmentDatasRemain_(0),
	fragmentDatasFlag_(0),
	pFragmentStream_(NULL),
	currMsgID_(0),
	currMsgLen_(0),
	isDestroyed_(false),
	shouldDropNextSend_(false),
	// Stats
	numPacketsSent_(0),
	numPacketsReceived_(0),
	numBytesSent_(0),
	numBytesReceived_(0),
	pFilter_(pFilter),
	pEndPoint_(NULL),
	pPacketReceiver_(NULL),
	isCondemn_(false)
{
	this->incRef();
	this->clearBundle();
	this->endpoint(endpoint);
	
	if(protocoltype_ == PROTOCOL_TCP)
	{
		pPacketReceiver_ = new TCPPacketReceiver(*pEndPoint_, networkInterface);
		// UDP不需要注册描述符
		pNetworkInterface_->dispatcher().registerFileDescriptor(*pEndPoint_, pPacketReceiver_);
	}
	else
		pPacketReceiver_ = new UDPPacketReceiver(*pEndPoint_, networkInterface);
	
	startInactivityDetection(INACTIVITY_TIMEOUT_DEFAULT);
}

//-------------------------------------------------------------------------------------
Channel::~Channel()
{
	DEBUG_MSG("Channel::~Channel(): %s\n", this->c_str());
	pNetworkInterface_->onChannelGone(this);

	if(protocoltype_ == PROTOCOL_TCP)
	{
		pNetworkInterface_->dispatcher().deregisterFileDescriptor(*pEndPoint_);
		pEndPoint_->close();
	}

	this->clearState();
	
	SAFE_RELEASE(pPacketReceiver_);
	SAFE_RELEASE(pBundle_);
	SAFE_RELEASE(pEndPoint_);
}

//-------------------------------------------------------------------------------------
Channel * Channel::get(NetworkInterface & networkInterface,
			const Address& addr)
{
	return networkInterface.findChannel(addr);
}

//-------------------------------------------------------------------------------------
Channel * get(NetworkInterface & networkInterface,
		const EndPoint* pSocket)
{
	return networkInterface.findChannel(pSocket->addr());
}

//-------------------------------------------------------------------------------------
void Channel::startInactivityDetection( float period, float checkPeriod )
{
	inactivityTimerHandle_.cancel();

	inactivityExceptionPeriod_ = uint64( period * stampsPerSecond() );
	lastReceivedTime_ = timestamp();

	inactivityTimerHandle_ =
		this->dispatcher().addTimer( int( checkPeriod * 1000000 ),
									this, (void *)TIMEOUT_INACTIVITY_CHECK );
}

//-------------------------------------------------------------------------------------
void Channel::endpoint(const EndPoint* endpoint)
{
	if (pEndPoint_ != endpoint)
	{
		pEndPoint_ = const_cast<EndPoint*>(endpoint);
	}
	
	lastReceivedTime_ = timestamp();
}

//-------------------------------------------------------------------------------------
void Channel::destroy()
{
	if(isDestroyed_)
	{
		CRITICAL_MSG("is channel has Destroyed!");
		return;
	}

	isDestroyed_ = true;
	this->decRef();
}

//-------------------------------------------------------------------------------------
void Channel::clearState( bool warnOnDiscard /*=false*/ )
{
	// 清空未处理的接受包缓存
	if (bufferedReceives_.size() > 0)
	{
		BufferedReceives::iterator iter = bufferedReceives_.begin();
		int hasDiscard = 0;
		
		for(; iter != bufferedReceives_.end(); iter++)
		{
			Packet* pPacket = (*iter).get();
			if(pPacket->opsize() > 0)
				hasDiscard++;
			
			pPacket->clear(false);
		}

		if (hasDiscard > 0 && warnOnDiscard)
		{
			WARNING_MSG( "Channel::clearState( %s ): "
				"Discarding %u buffered packet(s)\n",
				this->c_str(), hasDiscard );
		}

		bufferedReceives_.clear();
	}
	
	lastReceivedTime_ = timestamp();
	roundTripTime_ =
		this->isInternal() ? stampsPerSecond() / 10 : stampsPerSecond();

	isCondemn_ = false;
	shouldDropNextSend_ = false;
	numPacketsSent_ = 0;
	numPacketsReceived_ = 0;
	numBytesSent_ = 0;
	numBytesReceived_ = 0;
	fragmentDatasFlag_ = 0;
	pFragmentDatasWpos_ = 0;
	pFragmentDatasRemain_ = 0;
	currMsgID_ = 0;
	currMsgLen_ = 0;
	proxyID_ = 0;
	SAFE_RELEASE_ARRAY(pFragmentDatas_);
	SAFE_RELEASE(pFragmentStream_);

	inactivityTimerHandle_.cancel();
	this->endpoint(NULL);
}

//-------------------------------------------------------------------------------------
Bundle & Channel::bundle()
{
	return *pBundle_;
}

//-------------------------------------------------------------------------------------
const Bundle & Channel::bundle() const
{
	return *pBundle_;
}

//-------------------------------------------------------------------------------------
void Channel::send(Bundle * pBundle)
{
	if (this->isDestroyed())
	{
		ERROR_MSG("Channel::send(%s): Channel is destroyed.", this->c_str());
		// TODO: Should we return here?
	}
	
	bool isSendingOwnBundle = (pBundle == NULL);

	if(isSendingOwnBundle)
		pBundle = pBundle_;

	pBundle->send(*pNetworkInterface_, this);

	// Update our stats
	++numPacketsSent_;
	numBytesSent_ += pBundle->totalSize();


	// Clear the bundle
	if (isSendingOwnBundle)
	{
		this->clearBundle();
	}
	else
	{
		pBundle->clear();
	}
}

//-------------------------------------------------------------------------------------
void Channel::delayedSend()
{
	this->networkInterface().delayedSend(*this);
}

//-------------------------------------------------------------------------------------
const char * Channel::c_str() const
{
	static char dodgyString[ 40 ];

	int length = pEndPoint_->addr().writeToString(dodgyString, sizeof(dodgyString));

	length += kbe_snprintf(dodgyString + length,
		sizeof(dodgyString) - length,	"/%d", id_);

	return dodgyString;
}

//-------------------------------------------------------------------------------------
void Channel::clearBundle()
{
	if(!pBundle_)
	{
		pBundle_ = new Bundle(this);
	}
	else
	{
		pBundle_->clear();
	}
}

//-------------------------------------------------------------------------------------
void Channel::handleTimeout(TimerHandle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_INACTIVITY_CHECK:
		{
			if (timestamp() - lastReceivedTime_ > inactivityExceptionPeriod_)
			{
				this->networkInterface().onChannelTimeOut(this);
			}
			break;
		}
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------
void Channel::reset(const EndPoint* endpoint, bool warnOnDiscard)
{
	// 如果地址没有改变则不重置
	if (endpoint == pEndPoint_)
	{
		return;
	}

	// 让网络接口下一个tick处理自己
	pNetworkInterface_->sendIfDelayed( *this );
	this->clearState(warnOnDiscard);
	this->endpoint(endpoint);
}

//-------------------------------------------------------------------------------------
void Channel::onPacketReceived(int bytes)
{
	lastReceivedTime_ = timestamp();
	++numPacketsReceived_;
	numBytesReceived_ += bytes;
}

//-------------------------------------------------------------------------------------
void Channel::addReceiveWindow(Packet* pPacket)
{
	bufferedReceives_.push_back(pPacket);

	if(bufferedReceives_.size() > 10)
	{
		WARNING_MSG("Channel::addReceiveWindow: buffered is overload.");
	}
}

//-------------------------------------------------------------------------------------
void Channel::handleMessage(KBEngine::Mercury::MessageHandlers* pMsgHandlers)
{
	if (this->isDestroyed())
	{
		ERROR_MSG("Channel::handleMessage(%s): Channel is destroyed.", this->c_str());
		return;
	}

	try
	{
		BufferedReceives::iterator packetIter = bufferedReceives_.begin();
		for(; packetIter != bufferedReceives_.end(); packetIter++)
		{
			Packet* pPacket = (*packetIter).get();

			while(pPacket->totalSize() > 0)
			{
				if(fragmentDatasFlag_ == 0)
				{
					if(MERCURY_MESSAGE_ID_SIZE > 1 && pPacket->opsize() < MERCURY_MESSAGE_ID_SIZE)
					{
						writeFragmentMessage(1, pPacket, MERCURY_MESSAGE_ID_SIZE);
						break;
					}
					
					if(currMsgID_ == 0)
					{
						(*pPacket) >> currMsgID_;
						pPacket->messageID(currMsgID_);
					}

					Mercury::MessageHandler* pMsgHandler = pMsgHandlers->find(currMsgID_);

					if(pMsgHandler == NULL)
					{
						TRACE_BUNDLE_DATA(true, pPacket, pMsgHandler);
						INFO_MSG("Channel::handleMessage: invalide msgID=%d, msglen=%d, from %s.\n", 
							currMsgID_, pPacket->totalSize(), c_str());

						condemn(true);
						break;
					}
					
					TRACE_BUNDLE_DATA(true, pPacket, pMsgHandler);

					// 如果没有可操作的数据了则退出等待下一个包处理。
					if(pPacket->opsize() == 0)
						break;
					
					if(currMsgLen_ == 0)
					{
						if(pMsgHandler->msgLen == MERCURY_VARIABLE_MESSAGE || g_packetAlwaysContainLength)
						{
							// 如果长度信息不完整， 则等待下一个包处理
							if(pPacket->opsize() < MERCURY_MESSAGE_LENGTH_SIZE)
							{
								writeFragmentMessage(2, pPacket, MERCURY_MESSAGE_LENGTH_SIZE);
								break;
							}
							else
								(*pPacket) >> currMsgLen_;
						}
						else
							currMsgLen_ = pMsgHandler->msgLen;
					}
					
					if(currMsgLen_ > MERCURY_MESSAGE_MAX_SIZE)
					{
						INFO_MSG("Channel::handleMessage[%s]: msglen is error! msgID=%d, msglen=(%d:%d), from %s.\n", 
							pMsgHandler->name.c_str(), currMsgID_, currMsgLen_, pPacket->totalSize(), c_str());

						condemn(true);
						break;
					}

					if(pPacket->opsize() < currMsgLen_)
					{
						writeFragmentMessage(3, pPacket, currMsgLen_);
						break;
					}
					
					if(pFragmentStream_ != NULL)
					{
						pMsgHandler->handle(this, *pFragmentStream_);
						SAFE_RELEASE(pFragmentStream_);
					}
					else
					{
						// 临时设置有效读取位， 防止接口中溢出操作
						size_t wpos = pPacket->wpos();
						size_t rpos = pPacket->rpos();
						size_t frpos = pPacket->rpos() + currMsgLen_;
						pPacket->wpos(frpos);
						if(rpos==4 && wpos==17)
						{
							int ifdsa=0;
ifdsa++;
						}
						pMsgHandler->handle(this, *pPacket);

						// 防止handle中没有将数据导出获取非法操作
						if(currMsgLen_ > 0)
						{
							if(frpos != pPacket->rpos())
							{
								CRITICAL_MSG("Channel::handleMessage[%s]: rpos(%d) invalid, expect=%d. msgID=%d, msglen=%d.\n",
									pMsgHandler->name.c_str(), pPacket->rpos(), frpos, currMsgID_, currMsgLen_);

								pPacket->rpos(frpos);
							}
						}

						pPacket->wpos(wpos);
					}

					currMsgID_ = 0;
					currMsgLen_ = 0;
				}
				else
				{
					mergeFragmentMessage(pPacket);
				}
			}
		}
	}catch(MemoryStreamException &)
	{
		ERROR_MSG("Channel::handleMessage: packet invalid. currMsgID=%d, currMsgLen=%d\n", 
																	currMsgID_, currMsgLen_);
		currMsgID_ = 0;
		currMsgLen_ = 0;
	}

	bufferedReceives_.clear();
}

//-------------------------------------------------------------------------------------
void Channel::writeFragmentMessage(uint8 fragmentDatasFlag, Packet* pPacket, uint32 datasize)
{
	KBE_ASSERT(pFragmentDatas_ == NULL);

	size_t opsize = pPacket->opsize();
	pFragmentDatasRemain_ = datasize - opsize;
	pFragmentDatas_ = new uint8[opsize + pFragmentDatasRemain_ + 1];
	memcpy(pFragmentDatas_, pPacket->data() + pPacket->rpos(), opsize);
	fragmentDatasFlag_ = fragmentDatasFlag;
	pFragmentDatasWpos_ = opsize;
	pPacket->clear(false);
	DEBUG_MSG("Channel::writeFragmentMessage: fragmentDatasFlag=%u, remainsize=%u.\n", 
		fragmentDatasFlag, pFragmentDatasRemain_);
}

//-------------------------------------------------------------------------------------
void Channel::mergeFragmentMessage(Packet* pPacket)
{
	size_t opsize = pPacket->opsize();

	if(pPacket->opsize() >= pFragmentDatasRemain_)
	{
		pPacket->rpos(pPacket->rpos() + pFragmentDatasRemain_);
		memcpy(pFragmentDatas_ + pFragmentDatasWpos_, pPacket->data(), pFragmentDatasRemain_);
		

		switch(fragmentDatasFlag_)
		{
		case 1:		// 消息ID信息不全
			memcpy(&currMsgLen_, pFragmentDatas_, MERCURY_MESSAGE_ID_SIZE);
			break;

		case 2:		// 消息长度信息不全
			memcpy(&currMsgLen_, pFragmentDatas_, MERCURY_MESSAGE_LENGTH_SIZE);
			break;

		case 3:		// 消息内容信息不全
			pFragmentStream_ = new MemoryStream;
			pFragmentStream_->data_resize(currMsgLen_);
			pFragmentStream_->wpos(currMsgLen_);
			memcpy(pFragmentStream_->data(), pFragmentDatas_, currMsgLen_);
			break;

		default:
			break;
		};

		fragmentDatasFlag_ = 0;
		pFragmentDatasRemain_ = 0;
	}
	else
	{
		memcpy(pFragmentDatas_ + pFragmentDatasWpos_, pPacket->data(), opsize);
		pFragmentDatasRemain_ -= opsize;
		pPacket->rpos(pPacket->rpos() + opsize);
	}

	if(fragmentDatasFlag_ == 0)
		SAFE_RELEASE_ARRAY(pFragmentDatas_);
}

//-------------------------------------------------------------------------------------
EventDispatcher & Channel::dispatcher()
{
	return pNetworkInterface_->mainDispatcher();
}

}
}
