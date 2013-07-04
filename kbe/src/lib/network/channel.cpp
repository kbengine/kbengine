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

#include "network/websocket_protocol.hpp"
#include "network/html5_packet_filter.hpp"
#include "network/html5_packet_reader.hpp"
#include "network/bundle.hpp"
#include "network/packet_reader.hpp"
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
	pPacketReader_(0),
	isDestroyed_(false),
	shouldDropNextSend_(false),
	// Stats
	numPacketsSent_(0),
	numPacketsReceived_(0),
	numBytesSent_(0),
	numBytesReceived_(0),
	lastTickBytesReceived_(0),
	pFilter_(pFilter),
	pEndPoint_(NULL),
	pPacketReceiver_(NULL),
	isCondemn_(false),
	proxyID_(0),
	strextra_(),
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
	pPacketReader_(0),
	isDestroyed_(false),
	shouldDropNextSend_(false),
	// Stats
	numPacketsSent_(0),
	numPacketsReceived_(0),
	numBytesSent_(0),
	numBytesReceived_(0),
	lastTickBytesReceived_(0),
	pFilter_(NULL),
	pEndPoint_(NULL),
	pPacketReceiver_(NULL),
	isCondemn_(false),
	proxyID_(0),
	strextra_(),
	channelType_(CHANNEL_NORMAL),
	componentID_(UNKNOWN_COMPONENT_TYPE),
	pMsgHandlers_(NULL)
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
	SAFE_RELEASE(pPacketReader_);
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
	
	clearBundle();

	lastReceivedTime_ = timestamp();

	isCondemn_ = false;
	shouldDropNextSend_ = false;
	numPacketsSent_ = 0;
	numPacketsReceived_ = 0;
	numBytesSent_ = 0;
	numBytesReceived_ = 0;
	lastTickBytesReceived_ = 0;
	proxyID_ = 0;
	strextra_ = "";
	channelType_ = CHANNEL_NORMAL;

	SAFE_RELEASE(pPacketReader_);
	pFilter_ = NULL;

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
int32 Channel::bundlesLength()
{
	int32 len = 0;
	Bundles::iterator iter = bundles_.begin();
	for(; iter != bundles_.end(); iter++)
	{
		len += (*iter)->packetsLength();
	}

	return len;
}

//-------------------------------------------------------------------------------------
void Channel::pushBundle(Bundle* pBundle)
{
	bundles_.push_back(pBundle);
}

//-------------------------------------------------------------------------------------
void Channel::send(Bundle * pBundle)
{
	if (this->isDestroyed())
	{
		ERROR_MSG(boost::format("Channel::send(%1%): Channel is destroyed.\n") % 
			this->c_str());
		
		this->clearBundle();
		return;
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

	kbe_snprintf(dodgyString, MAX_BUF, "%s/%d/%d/%d", tdodgyString, id_, 
		this->isCondemn(), this->isDead());

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
	lastTickBytesReceived_ += bytes;
	g_numBytesReceived += bytes;

	if(this->isExternal())
	{
		if(g_extReceiveWindowBytesOverflow > 0 && 
			lastTickBytesReceived_ >= g_extReceiveWindowBytesOverflow)
		{
			WARNING_MSG(boost::format("Channel::onPacketReceived[%1%]: external channel(%2%), bufferedBytes is overflow(%3% > %4%).\n") % 
				this % this->c_str() % lastTickBytesReceived_ % g_extReceiveWindowBytesOverflow);

			this->condemn();
		}
	}
	else
	{
		if(g_intReceiveWindowBytesOverflow > 0 && 
			lastTickBytesReceived_ >= g_intReceiveWindowBytesOverflow)
		{
			WARNING_MSG(boost::format("Channel::onPacketReceived[%1%]: internal channel(%2%), bufferedBytes is overflow(%3% > %4%).\n") % 
				this % this->c_str() % lastTickBytesReceived_ % g_intReceiveWindowBytesOverflow);
		}
	}
}

//-------------------------------------------------------------------------------------
void Channel::addReceiveWindow(Packet* pPacket)
{
	bufferedReceives_.push_back(pPacket);

	if(Mercury::g_receiveWindowMessagesOverflowCritical > 0 && bufferedReceives_.size() > Mercury::g_receiveWindowMessagesOverflowCritical)
	{
		if(this->isExternal())
		{
			WARNING_MSG(boost::format("Channel::addReceiveWindow[%1%]: external channel(%2%), bufferedMessages is overflow(%3% > %4%).\n") % 
				this % this->c_str() % (int)bufferedReceives_.size() % Mercury::g_receiveWindowMessagesOverflowCritical);

			if(Mercury::g_extReceiveWindowMessagesOverflow > 0 && 
				bufferedReceives_.size() >  Mercury::g_extReceiveWindowMessagesOverflow)
			{
				ERROR_MSG(boost::format("Channel::addReceiveWindow[%1%]: external channel(%2%), bufferedMessages is overflow(%3% > %4%).\n") % 
					this % this->c_str() % (int)bufferedReceives_.size() % Mercury::g_extReceiveWindowMessagesOverflow);

				this->condemn();
			}
		}
		else
		{
			if(Mercury::g_intReceiveWindowMessagesOverflow > 0 && 
				bufferedReceives_.size() > Mercury::g_intReceiveWindowMessagesOverflow)
			{
				WARNING_MSG(boost::format("Channel::addReceiveWindow[%1%]: internal channel(%2%), bufferedMessages is overflow(%3% > %4%).\n") % 
					this % this->c_str() % (int)bufferedReceives_.size() % Mercury::g_intReceiveWindowMessagesOverflow);
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
	if(bufferedReceives_.size() > 0)
	{
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

				pPacketReader_ = new HTML5PacketReader(this);
				pFilter_ = new HTML5PacketFilter(this);
				DEBUG_MSG(boost::format("Channel::handshake: websocket(%1%) successfully!") % this->c_str());
				return;
			}
			else
			{
				DEBUG_MSG(boost::format("Channel::handshake: websocket(%1%) error!") % this->c_str());
			}
		}

		pPacketReader_ = new PacketReader(this);
	}
}

//-------------------------------------------------------------------------------------
void Channel::processPackets(KBEngine::Mercury::MessageHandlers* pMsgHandlers)
{
	lastTickBytesReceived_ = 0;

	if(pMsgHandlers_ != NULL)
	{
		pMsgHandlers = pMsgHandlers_;
	}

	if (this->isDestroyed())
	{
		ERROR_MSG(boost::format("Channel::processPackets(%1%): channel[%2%] is destroyed.\n") % 
			this->c_str() % this);

		return;
	}

	if(this->isCondemn())
	{
		ERROR_MSG(boost::format("Channel::processPackets(%1%): channel[%2%] is condemn.\n") % 
			this->c_str() % this);

		//this->destroy();
		return;
	}
	
	if(pPacketReader_ == NULL)
	{
		handshake();
	}

	try
	{
		BufferedReceives::iterator packetIter = bufferedReceives_.begin();
		for(; packetIter != bufferedReceives_.end(); packetIter++)
		{
			Packet* pPacket = (*packetIter);

			pPacketReader_->processMessages(pMsgHandlers, pPacket);

			if(pPacket->isTCPPacket())
				TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>(pPacket));
			else
				UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket*>(pPacket));

		}
	}catch(MemoryStreamException &)
	{
		Mercury::MessageHandler* pMsgHandler = pMsgHandlers->find(pPacketReader_->currMsgID());
		WARNING_MSG(boost::format("Channel::processPackets(%1%): packet invalid. currMsg=(name=%2%, id=%3%, len=%4%), currMsgLen=%5%\n") %
			this->c_str() 
			% (pMsgHandler == NULL ? "unknown" : pMsgHandler->name) 
			% pPacketReader_->currMsgID() 
			% (pMsgHandler == NULL ? -1 : pMsgHandler->msgLen) 
			% pPacketReader_->currMsgLen());

		pPacketReader_->currMsgID(0);
		pPacketReader_->currMsgLen(0);
		condemn();
	}

	bufferedReceives_.clear();
}

//-------------------------------------------------------------------------------------
EventDispatcher & Channel::dispatcher()
{
	return pNetworkInterface_->mainDispatcher();
}

}
}
