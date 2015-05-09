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


#include "channel.h"
#ifndef CODE_INLINE
#include "channel.inl"
#endif

#include "network/websocket_protocol.h"
#include "network/websocket_packet_filter.h"
#include "network/websocket_packet_reader.h"
#include "network/bundle.h"
#include "network/packet_reader.h"
#include "network/network_interface.h"
#include "network/tcp_packet_receiver.h"
#include "network/tcp_packet_sender.h"
#include "network/udp_packet_receiver.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "network/network_stats.h"

namespace KBEngine { 
namespace Network
{

//-------------------------------------------------------------------------------------
static ObjectPool<Channel> _g_objPool("Channel");
ObjectPool<Channel>& Channel::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
void Channel::destroyObjPool()
{
	DEBUG_MSG(fmt::format("Channel::destroyObjPool(): size {}.\n", 
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
size_t Channel::getPoolObjectBytes()
{
	size_t bytes = sizeof(pNetworkInterface_) + sizeof(traits_) + 
		sizeof(id_) + sizeof(inactivityTimerHandle_) + sizeof(inactivityExceptionPeriod_) + 
		sizeof(lastReceivedTime_) + (bufferedReceives_.size() * sizeof(Packet*)) + sizeof(pPacketReader_)
		+ sizeof(flags_) + sizeof(numPacketsSent_) + sizeof(numPacketsReceived_) + sizeof(numBytesSent_) + sizeof(numBytesReceived_)
		+ sizeof(lastTickBytesReceived_) + sizeof(lastTickBytesSent_) + sizeof(pFilter_) + sizeof(pEndPoint_) + sizeof(pPacketReceiver_) + sizeof(pPacketSender_)
		+ sizeof(proxyID_) + strextra_.size() + sizeof(channelType_)
		+ sizeof(componentID_) + sizeof(pMsgHandlers_);

	return bytes;
}

//-------------------------------------------------------------------------------------
Channel::SmartPoolObjectPtr Channel::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<Channel>(ObjPool().createObject(), _g_objPool));
}

//-------------------------------------------------------------------------------------
void Channel::onReclaimObject()
{
	this->clearState();
}

//-------------------------------------------------------------------------------------
Channel::Channel(NetworkInterface & networkInterface,
		const EndPoint * pEndPoint, Traits traits, ProtocolType pt,
		PacketFilterPtr pFilter, ChannelID id):
	pNetworkInterface_(NULL),
	traits_(traits),
	protocoltype_(pt),
	id_(id),
	inactivityTimerHandle_(),
	inactivityExceptionPeriod_(0),
	lastReceivedTime_(0),
	bundles_(),
	pPacketReader_(0),
	numPacketsSent_(0),
	numPacketsReceived_(0),
	numBytesSent_(0),
	numBytesReceived_(0),
	lastTickBytesReceived_(0),
	lastTickBytesSent_(0),
	pFilter_(pFilter),
	pEndPoint_(NULL),
	pPacketReceiver_(NULL),
	pPacketSender_(NULL),
	proxyID_(0),
	strextra_(),
	channelType_(CHANNEL_NORMAL),
	componentID_(UNKNOWN_COMPONENT_TYPE),
	pMsgHandlers_(NULL),
	flags_(0)
{
	this->clearBundle();
	initialize(networkInterface, pEndPoint, traits, pt, pFilter, id);
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
	pPacketReader_(0),
	// Stats
	numPacketsSent_(0),
	numPacketsReceived_(0),
	numBytesSent_(0),
	numBytesReceived_(0),
	lastTickBytesReceived_(0),
	lastTickBytesSent_(0),
	pFilter_(NULL),
	pEndPoint_(NULL),
	pPacketReceiver_(NULL),
	pPacketSender_(NULL),
	proxyID_(0),
	strextra_(),
	channelType_(CHANNEL_NORMAL),
	componentID_(UNKNOWN_COMPONENT_TYPE),
	pMsgHandlers_(NULL),
	flags_(0)
{
	this->clearBundle();
}

//-------------------------------------------------------------------------------------
Channel::~Channel()
{
	// DEBUG_MSG(fmt::format("Channel::~Channel(): {}\n", this->c_str()));
	finalise();
}

//-------------------------------------------------------------------------------------
bool Channel::initialize(NetworkInterface & networkInterface, 
		const EndPoint * pEndPoint, 
		Traits traits, 
		ProtocolType pt, 
		PacketFilterPtr pFilter, 
		ChannelID id)
{
	id_ = id;
	protocoltype_ = pt;
	traits_ = traits;
	pFilter_ = pFilter;
	pNetworkInterface_ = &networkInterface;
	this->pEndPoint(pEndPoint);

	KBE_ASSERT(pNetworkInterface_ != NULL);
	KBE_ASSERT(pEndPoint_ != NULL);


	if(protocoltype_ == PROTOCOL_TCP)
	{
		if(pPacketReceiver_)
		{
			if(pPacketReceiver_->type() == PacketReceiver::UDP_PACKET_RECEIVER)
			{
				SAFE_RELEASE(pPacketReceiver_);
				pPacketReceiver_ = new TCPPacketReceiver(*pEndPoint_, *pNetworkInterface_);
			}
		}
		else
		{
			pPacketReceiver_ = new TCPPacketReceiver(*pEndPoint_, *pNetworkInterface_);
		}

		KBE_ASSERT(pPacketReceiver_->type() == PacketReceiver::TCP_PACKET_RECEIVER);

		// UDP不需要注册描述符
		pNetworkInterface_->dispatcher().registerReadFileDescriptor(*pEndPoint_, pPacketReceiver_);

		// 需要发送数据时再注册
		// pPacketSender_ = new TCPPacketSender(*pEndPoint_, *pNetworkInterface_);
		// pNetworkInterface_->dispatcher().registerWriteFileDescriptor(*pEndPoint_, pPacketSender_);
	}
	else
	{
		if(pPacketReceiver_)
		{
			if(pPacketReceiver_->type() == PacketReceiver::TCP_PACKET_RECEIVER)
			{
				SAFE_RELEASE(pPacketReceiver_);
				pPacketReceiver_ = new UDPPacketReceiver(*pEndPoint_, *pNetworkInterface_);
			}
		}
		else
		{
			pPacketReceiver_ = new UDPPacketReceiver(*pEndPoint_, *pNetworkInterface_);
		}

		KBE_ASSERT(pPacketReceiver_->type() == PacketReceiver::UDP_PACKET_RECEIVER);
	}

	pPacketReceiver_->pEndPoint(pEndPoint_);
	if(pPacketSender_)
		pPacketSender_->pEndPoint(pEndPoint_);

	startInactivityDetection((traits_ == INTERNAL) ? g_channelInternalTimeout : 
													g_channelExternalTimeout,
							(traits_ == INTERNAL) ? g_channelInternalTimeout  / 2.f: 
													g_channelExternalTimeout / 2.f);

	return true;
}

//-------------------------------------------------------------------------------------
bool Channel::finalise()
{
	this->clearState();
	
	SAFE_RELEASE(pPacketReceiver_);
	SAFE_RELEASE(pPacketReader_);
	SAFE_RELEASE(pPacketSender_);

	Network::EndPoint::ObjPool().reclaimObject(pEndPoint_);
	pEndPoint_ = NULL;

	return true;
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
	if(period > 0.001f)
	{
		checkPeriod = std::max(1.f, checkPeriod);
		inactivityExceptionPeriod_ = uint64( period * stampsPerSecond() ) - uint64( 0.05f * stampsPerSecond() );
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
void Channel::pEndPoint(const EndPoint* pEndPoint)
{
	if (pEndPoint_ != pEndPoint)
	{
		Network::EndPoint::ObjPool().reclaimObject(pEndPoint_);
		pEndPoint_ = const_cast<EndPoint*>(pEndPoint);
	}
	
	lastReceivedTime_ = timestamp();
}

//-------------------------------------------------------------------------------------
void Channel::destroy()
{
	if(isDestroyed())
	{
		CRITICAL_MSG("is channel has Destroyed!\n");
		return;
	}

	clearState();
	flags_ |= FLAG_DESTROYED;
}

//-------------------------------------------------------------------------------------
void Channel::clearState( bool warnOnDiscard /*=false*/ )
{
	// 清空未处理的接受包缓存
	if (bufferedReceives_.size() > 0)
	{
		BufferedReceives::iterator iter = bufferedReceives_.begin();
		int hasDiscard = 0;
			
		for(; iter != bufferedReceives_.end(); ++iter)
		{
			Packet* pPacket = (*iter);
			if(pPacket->length() > 0)
				hasDiscard++;

			RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);
		}

		if (hasDiscard > 0 && warnOnDiscard)
		{
			WARNING_MSG(fmt::format("Channel::clearState( {} ): "
				"Discarding {} buffered packet(s)\n",
				this->c_str(), hasDiscard));
		}

		bufferedReceives_.clear();
	}

	clearBundle();

	lastReceivedTime_ = timestamp();

	numPacketsSent_ = 0;
	numPacketsReceived_ = 0;
	numBytesSent_ = 0;
	numBytesReceived_ = 0;
	lastTickBytesReceived_ = 0;
	proxyID_ = 0;
	strextra_ = "";
	channelType_ = CHANNEL_NORMAL;

	if(pEndPoint_ && protocoltype_ == PROTOCOL_TCP && !this->isDestroyed())
	{
		this->stopSend();

		if(pNetworkInterface_)
		{
			if(!this->isDestroyed())
				pNetworkInterface_->dispatcher().deregisterReadFileDescriptor(*pEndPoint_);
		}
	}

	// 这里只清空状态，不释放
	//SAFE_RELEASE(pPacketReader_);
	//SAFE_RELEASE(pPacketSender_);

	flags_ = 0;
	pFilter_ = NULL;

	stopInactivityDetection();

	// 由于pEndPoint通常由外部给入，必须释放，频道重新激活时会重新赋值
	if(pEndPoint_)
	{
		pEndPoint_->close();
		this->pEndPoint(NULL);
	}
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
	for(; iter != bundles_.end(); ++iter)
	{
		len += (*iter)->packetsLength();
	}

	return len;
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
		this->isCondemn(), this->isDestroyed());

	return dodgyString;
}

//-------------------------------------------------------------------------------------
void Channel::clearBundle()
{
	Bundles::iterator iter = bundles_.begin();
	for(; iter != bundles_.end(); ++iter)
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
			if (timestamp() - lastReceivedTime_ >= inactivityExceptionPeriod_)
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
void Channel::send(Bundle * pBundle)
{
	if (isDestroyed())
	{
		ERROR_MSG(fmt::format("Channel::send({}): channel has destroyed.\n", 
			this->c_str()));
		
		this->clearBundle();

		if(pBundle)
			Network::Bundle::ObjPool().reclaimObject(pBundle);

		return;
	}

	if(isCondemn())
	{
		//WARNING_MSG(fmt::format("Channel::send: is error, reason={}, from {}.\n", reasonToString(REASON_CHANNEL_CONDEMN), 
		//	c_str()));

		this->clearBundle();

		if(pBundle)
			Network::Bundle::ObjPool().reclaimObject(pBundle);

		return;
	}

	if(pBundle)
	{
		pBundle->finiMessage(true);
		bundles_.push_back(pBundle);
	}
	
	uint32 bundleSize = (uint32)bundles_.size();
	if(bundleSize == 0)
		return;

	if(!sending())
	{
		if(pPacketSender_ == NULL)
			pPacketSender_ = new TCPPacketSender(*pEndPoint_, *pNetworkInterface_);

		pPacketSender_->processSend(this);

		// 如果不能立即发送到系统缓冲区，那么交给poller处理
		if(bundles_.size() > 0 && !isCondemn() && !isDestroyed())
		{
			flags_ |= FLAG_SENDING;
			pNetworkInterface_->dispatcher().registerWriteFileDescriptor(*pEndPoint_, pPacketSender_);
		}
	}

	if(Network::g_sendWindowMessagesOverflowCritical > 0 && bundleSize > Network::g_sendWindowMessagesOverflowCritical)
	{
		if(this->isExternal())
		{
			WARNING_MSG(fmt::format("Channel::send[{:p}]: external channel({}), send window has overflowed({} > {}).\n", 
				(void*)this, this->c_str(), bundleSize, Network::g_sendWindowMessagesOverflowCritical));

			if(Network::g_extSendWindowMessagesOverflow > 0 && 
				bundleSize >  Network::g_extSendWindowMessagesOverflow)
			{
				ERROR_MSG(fmt::format("Channel::send[{:p}]: external channel({}), send window has overflowed({} > {}), Try adjusting the kbengine_defs.xml->windowOverflow->send.\n", 
					(void*)this, this->c_str(), bundleSize, Network::g_extSendWindowMessagesOverflow));

				this->condemn();
			}
		}
		else
		{
			if(Network::g_intSendWindowMessagesOverflow > 0 && 
				bundleSize > Network::g_intSendWindowMessagesOverflow)
			{
				ERROR_MSG(fmt::format("Channel::send[{:p}]: internal channel({}), send window has overflowed({} > {}).\n", 
					(void*)this, this->c_str(), bundleSize, Network::g_intSendWindowMessagesOverflow));

				this->condemn();
			}
			else
			{
				WARNING_MSG(fmt::format("Channel::send[{:p}]: internal channel({}), send window has overflowed({} > {}).\n", 
					(void*)this, this->c_str(), bundleSize, Network::g_sendWindowMessagesOverflowCritical));
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Channel::stopSend()
{
	if(!sending())
		return;

	flags_ &= ~FLAG_SENDING;

	pNetworkInterface_->dispatcher().deregisterWriteFileDescriptor(*pEndPoint_);
}

//-------------------------------------------------------------------------------------
void Channel::onSendCompleted()
{
	KBE_ASSERT(bundles_.size() == 0 && sending());
	stopSend();
}

//-------------------------------------------------------------------------------------
void Channel::onPacketSent(int bytes, bool sentCompleted)
{
	if(sentCompleted)
	{
		++numPacketsSent_;
		++g_numPacketsSent;
	}

	numBytesSent_ += bytes;
	g_numBytesSent += bytes;
	lastTickBytesSent_ += bytes;

	if(this->isExternal())
	{
		if(g_extSendWindowBytesOverflow > 0 && 
			lastTickBytesSent_ >= g_extSendWindowBytesOverflow)
		{
			ERROR_MSG(fmt::format("Channel::onPacketSent[{:p}]: external channel({}), bufferedBytes has overflowed({} > {}), Try adjusting the kbengine_defs.xml->windowOverflow->receive.\n", 
				(void*)this, this->c_str(), lastTickBytesSent_, g_extSendWindowBytesOverflow));

			this->condemn();
		}
	}
	else
	{
		if(g_intSendWindowBytesOverflow > 0 && 
			lastTickBytesSent_ >= g_intSendWindowBytesOverflow)
		{
			WARNING_MSG(fmt::format("Channel::onPacketSent[{:p}]: internal channel({}), bufferedBytes has overflowed({} > {}).\n", 
				(void*)this, this->c_str(), lastTickBytesSent_, g_intSendWindowBytesOverflow));
		}
	}
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
			ERROR_MSG(fmt::format("Channel::onPacketReceived[{:p}]: external channel({}), bufferedBytes has overflowed({} > {}), Try adjusting the kbengine_defs.xml->windowOverflow->receive.\n", 
				(void*)this, this->c_str(), lastTickBytesReceived_, g_extReceiveWindowBytesOverflow));

			this->condemn();
		}
	}
	else
	{
		if(g_intReceiveWindowBytesOverflow > 0 && 
			lastTickBytesReceived_ >= g_intReceiveWindowBytesOverflow)
		{
			WARNING_MSG(fmt::format("Channel::onPacketReceived[{:p}]: internal channel({}), bufferedBytes has overflowed({} > {}).\n", 
				(void*)this, this->c_str(), lastTickBytesReceived_, g_intReceiveWindowBytesOverflow));
		}
	}
}

//-------------------------------------------------------------------------------------
void Channel::addReceiveWindow(Packet* pPacket)
{
	bufferedReceives_.push_back(pPacket);
	uint32 size = (uint32)bufferedReceives_.size();

	if(Network::g_receiveWindowMessagesOverflowCritical > 0 && size > Network::g_receiveWindowMessagesOverflowCritical)
	{
		if(this->isExternal())
		{
			if(Network::g_extReceiveWindowMessagesOverflow > 0 && 
				size > Network::g_extReceiveWindowMessagesOverflow)
			{
				ERROR_MSG(fmt::format("Channel::addReceiveWindow[{:p}]: external channel({}), receive window has overflowed({} > {}), Try adjusting the kbengine_defs.xml->windowOverflow->receive->messages->external.\n", 
					(void*)this, this->c_str(), size, Network::g_extReceiveWindowMessagesOverflow));

				this->condemn();
			}
			else
			{
				WARNING_MSG(fmt::format("Channel::addReceiveWindow[{:p}]: external channel({}), receive window has overflowed({} > {}).\n", 
					(void*)this, this->c_str(), size, Network::g_receiveWindowMessagesOverflowCritical));
			}
		}
		else
		{
			if(Network::g_intReceiveWindowMessagesOverflow > 0 && 
				size > Network::g_intReceiveWindowMessagesOverflow)
			{
				WARNING_MSG(fmt::format("Channel::addReceiveWindow[{:p}]: internal channel({}), receive window has overflowed({} > {}).\n", 
					(void*)this, this->c_str(), size, Network::g_intReceiveWindowMessagesOverflow));
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Channel::condemn()
{ 
	if(isCondemn())
		return;

	flags_ |= FLAG_CONDEMN; 
	//WARNING_MSG(fmt::format("Channel::condemn[{:p}]: channel({}).\n", (void*)this, this->c_str())); 
}

//-------------------------------------------------------------------------------------
void Channel::handshake()
{
	if(hasHandshake())
		return;

	if(bufferedReceives_.size() > 0)
	{
		BufferedReceives::iterator packetIter = bufferedReceives_.begin();
		Packet* pPacket = (*packetIter);
		
		flags_ |= FLAG_HANDSHAKE;

		// 此处判定是否为websocket或者其他协议的握手
		if(websocket::WebSocketProtocol::isWebSocketProtocol(pPacket))
		{
			channelType_ = CHANNEL_WEB;
			if(websocket::WebSocketProtocol::handshake(this, pPacket))
			{
				if(pPacket->length() == 0)
				{
					bufferedReceives_.erase(packetIter);
				}

				if(!pPacketReader_ || pPacketReader_->type() != PacketReader::PACKET_READER_TYPE_WEBSOCKET)
				{
					SAFE_RELEASE(pPacketReader_);
					pPacketReader_ = new WebSocketPacketReader(this);
				}

				pFilter_ = new WebSocketPacketFilter(this);
				DEBUG_MSG(fmt::format("Channel::handshake: websocket({}) successfully!\n", this->c_str()));
				return;
			}
			else
			{
				DEBUG_MSG(fmt::format("Channel::handshake: websocket({}) error!\n", this->c_str()));
			}
		}

		if(!pPacketReader_ || pPacketReader_->type() != PacketReader::PACKET_READER_TYPE_SOCKET)
		{
			SAFE_RELEASE(pPacketReader_);
			pPacketReader_ = new PacketReader(this);
		}

		pPacketReader_->reset();
	}
}

//-------------------------------------------------------------------------------------
void Channel::processPackets(KBEngine::Network::MessageHandlers* pMsgHandlers)
{
	lastTickBytesReceived_ = 0;
	lastTickBytesSent_ = 0;

	if(pMsgHandlers_ != NULL)
	{
		pMsgHandlers = pMsgHandlers_;
	}

	if (this->isDestroyed())
	{
		ERROR_MSG(fmt::format("Channel::processPackets({}): channel[{:p}] is destroyed.\n", 
			this->c_str(), (void*)this));

		return;
	}

	if(this->isCondemn())
	{
		ERROR_MSG(fmt::format("Channel::processPackets({}): channel[{:p}] is condemn.\n", 
			this->c_str(), (void*)this));

		//this->destroy();
		return;
	}
	
	if(!hasHandshake())
	{
		handshake();
	}

	try
	{
		BufferedReceives::iterator packetIter = bufferedReceives_.begin();
		for(; packetIter != bufferedReceives_.end(); ++packetIter)
		{
			Packet* pPacket = (*packetIter);
			pPacketReader_->processMessages(pMsgHandlers, pPacket);
			RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);
		}
	}catch(MemoryStreamException &)
	{
		Network::MessageHandler* pMsgHandler = pMsgHandlers->find(pPacketReader_->currMsgID());
		WARNING_MSG(fmt::format("Channel::processPackets({}): packet invalid. currMsg=(name={}, id={}, len={}), currMsgLen={}\n",
			this->c_str()
			, (pMsgHandler == NULL ? "unknown" : pMsgHandler->name) 
			, pPacketReader_->currMsgID() 
			, (pMsgHandler == NULL ? -1 : pMsgHandler->msgLen) 
			, pPacketReader_->currMsgLen()));

		pPacketReader_->currMsgID(0);
		pPacketReader_->currMsgLen(0);
		condemn();
	}

	bufferedReceives_.clear();
}

//-------------------------------------------------------------------------------------
bool Channel::waitSend()
{
	return pEndPoint()->waitSend();
}

//-------------------------------------------------------------------------------------
EventDispatcher & Channel::dispatcher()
{
	return pNetworkInterface_->dispatcher();
}

//-------------------------------------------------------------------------------------

}
}
