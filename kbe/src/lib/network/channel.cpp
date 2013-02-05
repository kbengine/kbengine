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

#include "html5/websocket_protocol.hpp"
#include "network/bundle.hpp"
#include "network/network_interface.hpp"
#include "network/tcp_packet_receiver.hpp"
#include "network/udp_packet_receiver.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "network/mercurystats.hpp"

namespace KBEngine { 
namespace Mercury
{
const int EXTERNAL_CHANNEL_SIZE = 256;
const int INTERNAL_CHANNEL_SIZE = 4096;
const int INDEXED_CHANNEL_SIZE = 512;

//-------------------------------------------------------------------------------------
void Channel::onReclaimObject()
{
	this->clearState();
}

//-------------------------------------------------------------------------------------
bool Channel::destructorPoolObject()
{
	this->decRef();
	return true;
}

//-------------------------------------------------------------------------------------
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
	bundles_(),
	windowSize_(	(traits != INTERNAL)    ? EXTERNAL_CHANNEL_SIZE :
					(id == CHANNEL_ID_NULL) ? INTERNAL_CHANNEL_SIZE :
											  INDEXED_CHANNEL_SIZE),

	bufferedReceives_(),
	pFragmentDatas_(NULL),
	pFragmentDatasWpos_(0),
	pFragmentDatasRemain_(0),
	fragmentDatasFlag_(FRAGMENT_DATA_UNKNOW),
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
	isCondemn_(false),
	proxyID_(0),
	isHandshake_(false),
	channelType_(CHANNEL_NORMAL),
	componentID_(UNKNOWN_COMPONENT_TYPE),
	pMsgHandlers_(NULL)
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
	
	startInactivityDetection((traits == INTERNAL) ? g_channelInternalTimeout : g_channelExternalTimeout);
}

//-------------------------------------------------------------------------------------
Channel::Channel():
	pNetworkInterface_(NULL),
	traits_(EXTERNAL),
	protocoltype_(PROTOCOL_TCP),
	id_(0),
	inactivityTimerHandle_(),
	inactivityExceptionPeriod_(0),
	lastReceivedTime_(0),
	bundles_(),
	windowSize_(EXTERNAL_CHANNEL_SIZE),
	bufferedReceives_(),
	pFragmentDatas_(NULL),
	pFragmentDatasWpos_(0),
	pFragmentDatasRemain_(0),
	fragmentDatasFlag_(FRAGMENT_DATA_UNKNOW),
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
	pFilter_(NULL),
	pEndPoint_(NULL),
	pPacketReceiver_(NULL),
	isCondemn_(false),
	proxyID_(0),
	isHandshake_(false),
	channelType_(CHANNEL_NORMAL),
	componentID_(UNKNOWN_COMPONENT_TYPE)
{
	this->incRef();
	this->clearBundle();
	this->endpoint(NULL);
}

//-------------------------------------------------------------------------------------
Channel::~Channel()
{
	DEBUG_MSG(boost::format("Channel::~Channel(): %1%\n") % this->c_str());
	if(pNetworkInterface_ != NULL && pEndPoint_ != NULL && !isDestroyed_)
	{
		pNetworkInterface_->onChannelGone(this);

		if(protocoltype_ == PROTOCOL_TCP)
		{
			pNetworkInterface_->dispatcher().deregisterFileDescriptor(*pEndPoint_);
			pEndPoint_->close();
		}
	}

	this->clearState();
	
	SAFE_RELEASE(pPacketReceiver_);
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
	stopInactivityDetection();

	// 如果周期为负数则不检查
	if(period > 0.f)
	{
		inactivityExceptionPeriod_ = uint64( period * stampsPerSecond() );
		lastReceivedTime_ = timestamp();

		inactivityTimerHandle_ =
			this->dispatcher().addTimer( int( checkPeriod * 1000000 ),
										this, (void *)TIMEOUT_INACTIVITY_CHECK );
	}
}

//-------------------------------------------------------------------------------------
void Channel::stopInactivityDetection()
{
	inactivityTimerHandle_.cancel();
}

//-------------------------------------------------------------------------------------
void Channel::endpoint(const EndPoint* endpoint)
{
	if (pEndPoint_ != endpoint)
	{
		delete pEndPoint_;
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

	if(pNetworkInterface_ != NULL && pEndPoint_ != NULL)
	{
		pNetworkInterface_->onChannelGone(this);

		if(protocoltype_ == PROTOCOL_TCP)
		{
			pNetworkInterface_->dispatcher().deregisterFileDescriptor(*pEndPoint_);
			pEndPoint_->close();
		}
	}

	stopInactivityDetection();
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
			Packet* pPacket = (*iter);
			if(pPacket->opsize() > 0)
				hasDiscard++;

			if(pPacket->isTCPPacket())
				TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>(pPacket));
			else
				UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket*>(pPacket));
		}

		if (hasDiscard > 0 && warnOnDiscard)
		{
			WARNING_MSG(boost::format("Channel::clearState( %1% ): "
				"Discarding %2% buffered packet(s)\n") %
				this->c_str() % hasDiscard );
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
	fragmentDatasFlag_ = FRAGMENT_DATA_UNKNOW;
	pFragmentDatasWpos_ = 0;
	pFragmentDatasRemain_ = 0;
	currMsgID_ = 0;
	currMsgLen_ = 0;
	proxyID_ = 0;
	isHandshake_ = false;
	channelType_ = CHANNEL_NORMAL;

	SAFE_RELEASE_ARRAY(pFragmentDatas_);
	MemoryStream::ObjPool().reclaimObject(pFragmentStream_);
	pFragmentStream_ = NULL;

	stopInactivityDetection();
	this->endpoint(NULL);
}

//-------------------------------------------------------------------------------------
Channel::Bundles & Channel::bundles()
{
	return bundles_;
}

//-------------------------------------------------------------------------------------
const Channel::Bundles & Channel::bundles() const
{
	return bundles_;
}

//-------------------------------------------------------------------------------------
void Channel::send(Bundle * pBundle)
{
	if (this->isDestroyed())
	{
		ERROR_MSG(boost::format("Channel::send(%1%): Channel is destroyed.") % this->c_str());
	}
	
	if(pBundle)
	{
		pBundle->send(*pNetworkInterface_, this);

		++numPacketsSent_;
		++g_numPacketsSent;
		numBytesSent_ += pBundle->totalSize();
		g_numBytesSent += pBundle->totalSize();

		pBundle->clear(true);
	}
	else
	{
		Bundles::iterator iter = bundles_.begin();
		for(; iter != bundles_.end(); iter++)
		{
			(*iter)->send(*pNetworkInterface_, this);

			++numPacketsSent_;
			++g_numPacketsSent;
			numBytesSent_ += (*iter)->totalSize();
			g_numBytesSent += (*iter)->totalSize();
		}

		this->clearBundle();
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
	static char dodgyString[ MAX_BUF ] = {"None"};
	char tdodgyString[ MAX_BUF ] = {0};

	if(pEndPoint_ && !pEndPoint_->addr().isNone())
		pEndPoint_->addr().writeToString(tdodgyString, MAX_BUF);

	kbe_snprintf(dodgyString, MAX_BUF, "%s/%d", tdodgyString, id_);

	return dodgyString;
}

//-------------------------------------------------------------------------------------
void Channel::clearBundle()
{
	Bundles::iterator iter = bundles_.begin();
	for(; iter != bundles_.end(); iter++)
	{
		Bundle::ObjPool().reclaimObject((*iter));
	}

	bundles_.clear();
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
	if(pNetworkInterface_)
		pNetworkInterface_->sendIfDelayed(*this);

	this->clearState(warnOnDiscard);
	this->endpoint(endpoint);
}

//-------------------------------------------------------------------------------------
void Channel::onPacketReceived(int bytes)
{
	lastReceivedTime_ = timestamp();
	++numPacketsReceived_;
	++g_numPacketsReceived;

	numBytesReceived_ += bytes;
	g_numBytesReceived += bytes;
}

//-------------------------------------------------------------------------------------
void Channel::addReceiveWindow(Packet* pPacket)
{
	bufferedReceives_.push_back(pPacket);

	if(bufferedReceives_.size() > 32)
	{
		if(!this->isExternal())
		{
			if(bufferedReceives_.size() >  Mercury::g_extReceiveWindowOverflow)
			{
				WARNING_MSG(boost::format("Channel::addReceiveWindow[%1%]: internal channel(%2%), buffered is overload(%3%).\n") % 
					this % this->c_str() % (int)bufferedReceives_.size());
			}
		}
		else
		{
			WARNING_MSG(boost::format("Channel::addReceiveWindow[%1%]: external channel(%2%), buffered is overload(%3%).\n") % 
				this % this->c_str() % (int)bufferedReceives_.size());

			if(bufferedReceives_.size() > Mercury::g_intReceiveWindowOverflow)
			{
				this->condemn();
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Channel::condemn()
{ 
	isCondemn_ = true; 
	ERROR_MSG(boost::format("Channel::condemn[%1%]: channel(%2%).\n") % this % this->c_str()); 
}

//-------------------------------------------------------------------------------------
void Channel::handshake()
{
	if(!isHandshake_ && bufferedReceives_.size() > 0)
	{
		isHandshake_ = true;

		BufferedReceives::iterator packetIter = bufferedReceives_.begin();
		Packet* pPacket = (*packetIter);
		
		// 此处判定是否为websocket或者其他协议的握手
		if(html5::WebSocketProtocol::isWebSocketProtocol(pPacket))
		{
			channelType_ = CHANNEL_WEB;
			if(html5::WebSocketProtocol::handshake(this, pPacket))
			{
				if(pPacket->totalSize() == 0)
				{
					bufferedReceives_.erase(packetIter);
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Channel::handleMessage(KBEngine::Mercury::MessageHandlers* pMsgHandlers)
{
	if(pMsgHandlers_ != NULL)
	{
		pMsgHandlers = pMsgHandlers_;
	}

	if (this->isDestroyed())
	{
		ERROR_MSG(boost::format("Channel::handleMessage(%1%): channel[%2%] is destroyed.\n") % this->c_str() % this);
		return;
	}

	if(this->isCondemn())
	{
		ERROR_MSG(boost::format("Channel::handleMessage(%1%): channel[%2%] is condemn.\n") % this->c_str() % this);
		//this->destroy();
		return;
	}

	try
	{
		BufferedReceives::iterator packetIter = bufferedReceives_.begin();
		for(; packetIter != bufferedReceives_.end(); packetIter++)
		{
			Packet* pPacket = (*packetIter);

			while(pPacket->totalSize() > 0 || pFragmentStream_ != NULL)
			{
				if(fragmentDatasFlag_ == FRAGMENT_DATA_UNKNOW)
				{
					if(currMsgID_ == 0)
					{
						if(MERCURY_MESSAGE_ID_SIZE > 1 && pPacket->opsize() < MERCURY_MESSAGE_ID_SIZE)
						{
							writeFragmentMessage(FRAGMENT_DATA_MESSAGE_ID, pPacket, MERCURY_MESSAGE_ID_SIZE);
							break;
						}

						(*pPacket) >> currMsgID_;
						pPacket->messageID(currMsgID_);
					}

					Mercury::MessageHandler* pMsgHandler = pMsgHandlers->find(currMsgID_);

					if(pMsgHandler == NULL)
					{
						MemoryStream* pPacket1 = pFragmentStream_ != NULL ? pFragmentStream_ : pPacket;
						TRACE_BUNDLE_DATA(true, pPacket1, pMsgHandler, pPacket1->opsize(), this->c_str());
						
						// 用作调试时比对
						uint32 rpos = pPacket1->rpos();
						pPacket1->rpos(0);
						TRACE_BUNDLE_DATA(true, pPacket1, pMsgHandler, pPacket1->opsize(), this->c_str());
						pPacket1->rpos(rpos);

						WARNING_MSG(boost::format("Channel::handleMessage: invalide msgID=%1%, msglen=%2%, from %3%.\n") %
							currMsgID_ % pPacket1->opsize() % c_str());

						currMsgID_ = 0;
						currMsgLen_ = 0;
						condemn();
						break;
					}

					// 如果没有可操作的数据了则退出等待下一个包处理。
					//if(pPacket->opsize() == 0)	// 可能是一个无参数数据包
					//	break;
					
					if(currMsgLen_ == 0)
					{
						if(pMsgHandler->msgLen == MERCURY_VARIABLE_MESSAGE || g_packetAlwaysContainLength)
						{
							// 如果长度信息不完整， 则等待下一个包处理
							if(pPacket->opsize() < MERCURY_MESSAGE_LENGTH_SIZE)
							{
								writeFragmentMessage(FRAGMENT_DATA_MESSAGE_LENGTH, pPacket, MERCURY_MESSAGE_LENGTH_SIZE);
								break;
							}
							else
							{
								(*pPacket) >> currMsgLen_;
								MercuryStats::getSingleton().trackMessage(MercuryStats::RECV, *pMsgHandler, 
									currMsgLen_ + MERCURY_MESSAGE_ID_SIZE + MERCURY_MESSAGE_LENGTH_SIZE);
							}
						}
						else
						{
							currMsgLen_ = pMsgHandler->msgLen;
							MercuryStats::getSingleton().trackMessage(MercuryStats::RECV, *pMsgHandler, 
								currMsgLen_ + MERCURY_MESSAGE_LENGTH_SIZE);
						}
					}
					
					if(currMsgLen_ > pMsgHandler->msglenMax())
					{
						MemoryStream* pPacket1 = pFragmentStream_ != NULL ? pFragmentStream_ : pPacket;
						TRACE_BUNDLE_DATA(true, pPacket1, pMsgHandler, pPacket1->opsize(), this->c_str());

						// 用作调试时比对
						uint32 rpos = pPacket1->rpos();
						pPacket1->rpos(0);
						TRACE_BUNDLE_DATA(true, pPacket1, pMsgHandler, pPacket1->opsize(), this->c_str());
						pPacket1->rpos(rpos);

						WARNING_MSG(boost::format("Channel::handleMessage(%1%): msglen is error! msgID=%2%, msglen=(%3%:%4%), from %5%.\n") % 
							pMsgHandler->name.c_str() % currMsgID_ % currMsgLen_ % pPacket1->opsize() % c_str());

						currMsgLen_ = 0;
						condemn();
						break;
					}

					if(pFragmentStream_ != NULL)
					{
						TRACE_BUNDLE_DATA(true, pFragmentStream_, pMsgHandler, currMsgLen_, this->c_str());
						pMsgHandler->handle(this, *pFragmentStream_);
						MemoryStream::ObjPool().reclaimObject(pFragmentStream_);
						pFragmentStream_ = NULL;
					}
					else
					{
						if(pPacket->opsize() < currMsgLen_)
						{
							writeFragmentMessage(FRAGMENT_DATA_MESSAGE_BODY, pPacket, currMsgLen_);
							break;
						}

						// 临时设置有效读取位， 防止接口中溢出操作
						size_t wpos = pPacket->wpos();
						// size_t rpos = pPacket->rpos();
						size_t frpos = pPacket->rpos() + currMsgLen_;
						pPacket->wpos(frpos);

						TRACE_BUNDLE_DATA(true, pPacket, pMsgHandler, currMsgLen_, this->c_str());
						pMsgHandler->handle(this, *pPacket);

						// 防止handle中没有将数据导出获取非法操作
						if(currMsgLen_ > 0)
						{
							if(frpos != pPacket->rpos())
							{
								CRITICAL_MSG(boost::format("Channel::handleMessage(%s): rpos(%d) invalid, expect=%d. msgID=%d, msglen=%d.\n") %
									pMsgHandler->name.c_str() % pPacket->rpos() % frpos % currMsgID_ % currMsgLen_);

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

			if(pPacket->isTCPPacket())
				TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>(pPacket));
			else
				UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket*>(pPacket));

		}
	}catch(MemoryStreamException &)
	{
		WARNING_MSG(boost::format("Channel::handleMessage(%1%): packet invalid. currMsgID=%2%, currMsgLen=%3%\n") %
																this->c_str() % currMsgID_ % currMsgLen_);

		currMsgID_ = 0;
		currMsgLen_ = 0;
		condemn();
	}

	bufferedReceives_.clear();
}

//-------------------------------------------------------------------------------------
void Channel::writeFragmentMessage(FragmentDataTypes fragmentDatasFlag, Packet* pPacket, uint32 datasize)
{
	KBE_ASSERT(pFragmentDatas_ == NULL);

	size_t opsize = pPacket->opsize();
	pFragmentDatasRemain_ = datasize - opsize;
	pFragmentDatas_ = new uint8[opsize + pFragmentDatasRemain_ + 1];

	fragmentDatasFlag_ = fragmentDatasFlag;
	pFragmentDatasWpos_ = opsize;

	if(pPacket->opsize() > 0)
	{
		memcpy(pFragmentDatas_, pPacket->data() + pPacket->rpos(), opsize);
		pPacket->opfini();
	}

	DEBUG_MSG(boost::format("Channel::writeFragmentMessage(%1%): channel[%2%], fragmentDatasFlag=%3%, remainsize=%4%.\n") % 
		this->c_str() % this % fragmentDatasFlag % pFragmentDatasRemain_);
}

//-------------------------------------------------------------------------------------
void Channel::mergeFragmentMessage(Packet* pPacket)
{
	size_t opsize = pPacket->opsize();
	if(opsize == 0)
		return;

	if(pPacket->opsize() >= pFragmentDatasRemain_)
	{
		pPacket->rpos(pPacket->rpos() + pFragmentDatasRemain_);
		memcpy(pFragmentDatas_ + pFragmentDatasWpos_, pPacket->data(), pFragmentDatasRemain_);
		
		KBE_ASSERT(pFragmentStream_ == NULL);

		switch(fragmentDatasFlag_)
		{
		case FRAGMENT_DATA_MESSAGE_ID:			// 消息ID信息不全
			memcpy(&currMsgID_, pFragmentDatas_, MERCURY_MESSAGE_ID_SIZE);
			break;

		case FRAGMENT_DATA_MESSAGE_LENGTH:		// 消息长度信息不全
			memcpy(&currMsgLen_, pFragmentDatas_, MERCURY_MESSAGE_LENGTH_SIZE);
			break;

		case FRAGMENT_DATA_MESSAGE_BODY:		// 消息内容信息不全
			pFragmentStream_ = MemoryStream::ObjPool().createObject();
			pFragmentStream_->data_resize(currMsgLen_);
			pFragmentStream_->wpos(currMsgLen_);
			memcpy(pFragmentStream_->data(), pFragmentDatas_, currMsgLen_);
			break;

		default:
			break;
		};

		fragmentDatasFlag_ = FRAGMENT_DATA_UNKNOW;
		pFragmentDatasRemain_ = 0;
		SAFE_RELEASE_ARRAY(pFragmentDatas_);
		DEBUG_MSG(boost::format("Channel::mergeFragmentMessage(%1%): channel[%2%], completed!\n") % 
			this->c_str() % this);
	}
	else
	{
		memcpy(pFragmentDatas_ + pFragmentDatasWpos_, pPacket->data(), opsize);
		pFragmentDatasRemain_ -= opsize;
		pPacket->rpos(pPacket->rpos() + opsize);

		DEBUG_MSG(boost::format("Channel::writeFragmentMessage(%1%): channel[%2%], fragmentDatasFlag=%3%, remainsize=%4%.\n") %
			this->c_str() % this % fragmentDatasFlag_ % pFragmentDatasRemain_);
	}	
}

//-------------------------------------------------------------------------------------
EventDispatcher & Channel::dispatcher()
{
	return pNetworkInterface_->mainDispatcher();
}

}
}
