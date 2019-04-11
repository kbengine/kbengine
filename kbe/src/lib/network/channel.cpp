// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
#include "network/kcp_packet_sender.h"
#include "network/kcp_packet_receiver.h"
#include "network/kcp_packet_reader.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "network/network_stats.h"
#include "helper/profile.h"
#include "common/ssl.h"

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
Channel* Channel::createPoolObject(const std::string& logPoint)
{
	return _g_objPool.createObject(logPoint);
}

//-------------------------------------------------------------------------------------
void Channel::reclaimPoolObject(Channel* obj)
{
	_g_objPool.reclaimObject(obj);
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
	size_t bytes = sizeof(pNetworkInterface_) + sizeof(traits_) + sizeof(protocoltype_) + sizeof(protocolSubtype_) + 
		sizeof(id_) + sizeof(inactivityTimerHandle_) + sizeof(inactivityExceptionPeriod_) + 
		sizeof(lastReceivedTime_) + sizeof(lastTickBufferedReceives_) + sizeof(pPacketReader_) + (bundles_.size() * sizeof(Bundle*)) +
		+ sizeof(flags_) + sizeof(numPacketsSent_) + sizeof(numPacketsReceived_) + sizeof(numBytesSent_) + sizeof(numBytesReceived_)
		+ sizeof(lastTickBytesReceived_) + sizeof(lastTickBytesSent_) + sizeof(pFilter_) + sizeof(pEndPoint_) + sizeof(pPacketReceiver_) + sizeof(pPacketSender_)
		+ sizeof(proxyID_) + strextra_.size() + sizeof(channelType_)
		+ sizeof(componentID_) + sizeof(pMsgHandlers_) + condemnReason_.size() + sizeof(kcpUpdateTimerHandle_) + sizeof(pKCP_) + sizeof(hasSetNextKcpUpdate_);

	return bytes;
}

//-------------------------------------------------------------------------------------
Channel::SmartPoolObjectPtr Channel::createSmartPoolObj(const std::string& logPoint)
{
	return SmartPoolObjectPtr(new SmartPoolObject<Channel>(ObjPool().createObject(logPoint), _g_objPool));
}

//-------------------------------------------------------------------------------------
void Channel::onReclaimObject()
{
	this->clearState();
}

//-------------------------------------------------------------------------------------
void Channel::onEabledPoolObject()
{

}

//-------------------------------------------------------------------------------------
Channel::Channel(NetworkInterface & networkInterface,
		const EndPoint * pEndPoint, Traits traits, ProtocolType pt, ProtocolSubType spt,
		PacketFilterPtr pFilter, ChannelID id):
	pNetworkInterface_(NULL),
	traits_(traits),
	protocoltype_(pt),
	protocolSubtype_(spt),
	id_(id),
	inactivityTimerHandle_(),
	inactivityExceptionPeriod_(0),
	lastReceivedTime_(0),
	bundles_(),
	lastTickBufferedReceives_(0),
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
	flags_(0),
	pKCP_(NULL),
	kcpUpdateTimerHandle_(),
	hasSetNextKcpUpdate_(false),
	condemnReason_()
{
	this->clearBundle();
	initialize(networkInterface, pEndPoint, traits, pt, spt, pFilter, id);
}

//-------------------------------------------------------------------------------------
Channel::Channel():
	pNetworkInterface_(NULL),
	traits_(EXTERNAL),
	protocoltype_(PROTOCOL_TCP),
	protocolSubtype_(SUB_PROTOCOL_DEFAULT),
	id_(0),
	inactivityTimerHandle_(),
	inactivityExceptionPeriod_(0),
	lastReceivedTime_(0),
	bundles_(),
	lastTickBufferedReceives_(0),
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
	flags_(0),
	pKCP_(NULL),
	kcpUpdateTimerHandle_(),
	hasSetNextKcpUpdate_(false),
	condemnReason_()
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
		ProtocolSubType spt,
		PacketFilterPtr pFilter, 
		ChannelID id)
{
	id_ = id;
	protocoltype_ = pt;
	protocolSubtype_ = spt;
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

		if (pPacketSender_ && pPacketSender_->type() != PacketSender::TCP_PACKET_SENDER)
		{
			KCPPacketSender::reclaimPoolObject((KCPPacketSender*)pPacketSender_);
			pPacketSender_ = NULL;
		}
	}
	else
	{
		if (protocolSubtype_ == SUB_PROTOCOL_KCP)
		{
			if (pPacketReceiver_)
			{
				if (pPacketReceiver_->type() == PacketReceiver::TCP_PACKET_RECEIVER)
				{
					SAFE_RELEASE(pPacketReceiver_);
					pPacketReceiver_ = new KCPPacketReceiver(*pEndPoint_, *pNetworkInterface_);
				}
			}
			else
			{
				pPacketReceiver_ = new KCPPacketReceiver(*pEndPoint_, *pNetworkInterface_);
			}

			if (!init_kcp())
			{
				KBE_ASSERT(false);
				return false;
			}
		}
		else
		{
			if (pPacketReceiver_)
			{
				if (pPacketReceiver_->type() == PacketReceiver::TCP_PACKET_RECEIVER)
				{
					SAFE_RELEASE(pPacketReceiver_);
					pPacketReceiver_ = new UDPPacketReceiver(*pEndPoint_, *pNetworkInterface_);
				}
			}
			else
			{
				pPacketReceiver_ = new UDPPacketReceiver(*pEndPoint_, *pNetworkInterface_);
			}
		}

		KBE_ASSERT(pPacketReceiver_->type() == PacketReceiver::UDP_PACKET_RECEIVER);

		if (pPacketSender_ && pPacketSender_->type() != PacketSender::UDP_PACKET_SENDER)
		{
			TCPPacketSender::reclaimPoolObject((TCPPacketSender*)pPacketSender_);
			pPacketSender_ = NULL;
		}
	}

	pPacketReceiver_->pEndPoint(pEndPoint_);
	
	if(pPacketSender_)
		pPacketSender_->pEndPoint(pEndPoint_);

	startInactivityDetection((traits_ == INTERNAL) ? g_channelInternalTimeout : 
													g_channelExternalTimeout,
							(traits_ == INTERNAL) ? g_channelInternalTimeout / 2.f: 
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

	Network::EndPoint::reclaimPoolObject(pEndPoint_);
	pEndPoint_ = NULL;

	return true;
}

//-------------------------------------------------------------------------------------
uint32 Channel::getRTT()
{
	if (protocolSubtype_ == SUB_PROTOCOL_KCP && pKCP_)
		return (uint32)(pKCP_->rx_srtt/* BSD标准，毫秒 */) * 1000;

	if (!pEndPoint())
		return 0;

	return pEndPoint()->getRTT();
}

//-------------------------------------------------------------------------------------
IUINT32 createNewKCPID(IUINT32 arrayIndex)
{
	IUINT32 sessionID = 0;

	IUINT32 index = arrayIndex;
	index <<= 16;

	sessionID |= index;

	IUINT32 rnd = 0;

	srand(getSystemTime());
	rnd = (IUINT32)(rand() << 16);

	sessionID |= rnd;
	return sessionID;
}

bool Channel::init_kcp()
{
	static IUINT32 convID = 1;

	// 防止溢出，理论上正常使用不会用完
	KBE_ASSERT(convID != 0);

	if(id_ == 0)
		id_ = convID++;

	pKCP_ = ikcp_create((IUINT32)id_, (void*)this);
	pKCP_->output = &Channel::kcp_output;

	// 配置窗口大小：平均延迟200ms，每20ms发送一个包，
	// 而考虑到丢包重发，设置最大收发窗口为128
	int sndwnd = this->isExternal() ? Network::g_rudp_extWritePacketsQueueSize : Network::g_rudp_intWritePacketsQueueSize;
	int rcvwnd = this->isExternal() ? Network::g_rudp_extReadPacketsQueueSize : Network::g_rudp_intReadPacketsQueueSize;

	// nodelay-启用以后若干常规加速将启动
	// interval为内部处理时钟，默认设置为 10ms
	// resend为快速重传指标，设置为2
	// nc为是否禁用常规流控，这里禁止
	int nodelay = Network::g_rudp_nodelay ? 1 : 0;
	int interval = Network::g_rudp_tickInterval;
	int resend = Network::g_rudp_missAcksResend;
	int disableNC = (!Network::g_rudp_congestionControl) ? 1 : 0;
	int minrto = Network::g_rudp_minRTO;

	if (this->isExternal() && Network::g_rudp_mtu > 0 && Network::g_rudp_mtu < (PACKET_MAX_SIZE_UDP * 4))
	{
		ikcp_setmtu(pKCP_, Network::g_rudp_mtu);
	}
	else
	{
		uint32 mtu = PACKET_MAX_SIZE_UDP - 72;
		if(pKCP_->mtu != mtu)
			ikcp_setmtu(pKCP_, mtu);
	}
		
	ikcp_wndsize(pKCP_, sndwnd, rcvwnd);
	ikcp_nodelay(pKCP_, nodelay, interval, resend, disableNC);
	pKCP_->rx_minrto = minrto;

	pKCP_->writelog = &Channel::kcp_writeLog;
	
	/*
	pKCP_->logmask |= (IKCP_LOG_OUTPUT | IKCP_LOG_INPUT | IKCP_LOG_SEND | IKCP_LOG_RECV | IKCP_LOG_IN_DATA | IKCP_LOG_IN_ACK | 
		IKCP_LOG_IN_PROBE | IKCP_LOG_IN_WINS | IKCP_LOG_OUT_DATA | IKCP_LOG_OUT_ACK | IKCP_LOG_OUT_PROBE | IKCP_LOG_OUT_WINS);
	*/

	hasSetNextKcpUpdate_ = false;
	addKcpUpdate();
	return true;
}

//-------------------------------------------------------------------------------------
bool Channel::fina_kcp()
{
	if (!pKCP_)
		return true;

	ikcp_release(pKCP_);
	pKCP_ = NULL;

	if (kcpUpdateTimerHandle_.isSet())
		kcpUpdateTimerHandle_.cancel();

	hasSetNextKcpUpdate_ = false;
	return true;
}

//-------------------------------------------------------------------------------------
void Channel::kcp_writeLog(const char *log, struct IKCPCB *kcp, void *user)
{
	Channel* pChannel = (Channel*)user;
	DEBUG_MSG(fmt::format("Channel::kcp_writeLog: {}, addr={}\n", log, pChannel->c_str()));
}

//-------------------------------------------------------------------------------------
int Channel::kcp_output(const char *buf, int len, ikcpcb *kcp, void *user)
{
	Channel* pChannel = (Channel*)user;

	if (pChannel->condemn() == Channel::FLAG_CONDEMN_AND_DESTROY)
		return -1;

	return ((KCPPacketSender*)pChannel->pPacketSender())->kcp_output(buf, len, kcp, pChannel);
}

//-------------------------------------------------------------------------------------
void Channel::addKcpUpdate(int64 microseconds)
{
	//AUTO_SCOPED_PROFILE("addKcpUpdate");

	if (microseconds <= 1)
	{
		// 避免send等操作导致多次添加和取消timer
		if (!hasSetNextKcpUpdate_)
			hasSetNextKcpUpdate_ = true;
		else
			return;
	}
	else
	{
		hasSetNextKcpUpdate_ = false;
	}

	if (kcpUpdateTimerHandle_.isSet())
	{
		kcpUpdateTimerHandle_.cancel();
	}

	kcpUpdateTimerHandle_ = this->dispatcher().addTimer(microseconds, this, (void *)KCP_UPDATE);
}

//-------------------------------------------------------------------------------------
void Channel::kcpUpdate()
{
	//AUTO_SCOPED_PROFILE("kcpUpdate");

	uint32 current = kbe_clock();
	ikcp_update(pKCP_, current);

	uint32 nextUpdateKcpTime = ikcp_check(pKCP_, current) - current;

	if (nextUpdateKcpTime > 0)
	{
		addKcpUpdate(nextUpdateKcpTime * 1000);
	}
	else
	{
		kcpUpdateTimerHandle_.cancel();
		hasSetNextKcpUpdate_ = false;
	}
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
	if (period > 0.1f)
	{
		checkPeriod = std::max(1.f, checkPeriod);

		int64 icheckPeriod = int64(checkPeriod * 1000000);
		if (icheckPeriod <= 0)
		{
			ERROR_MSG(fmt::format("Channel::startInactivityDetection: checkPeriod overflowed, close checker! period={}, checkPeriod={}\n", period, checkPeriod));
			return;
		}

		inactivityExceptionPeriod_ = uint64(period * stampsPerSecond()) - uint64(0.05f * stampsPerSecond());
		lastReceivedTime_ = timestamp();

		inactivityTimerHandle_ =
			this->dispatcher().addTimer(icheckPeriod,
				this, (void *)TIMEOUT_INACTIVITY_CHECK);
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
		Network::EndPoint::reclaimPoolObject(pEndPoint_);
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
	clearBundle();

	lastReceivedTime_ = timestamp();

	numPacketsSent_ = 0;
	numPacketsReceived_ = 0;
	numBytesSent_ = 0;
	numBytesReceived_ = 0;
	lastTickBytesReceived_ = 0;
	lastTickBytesSent_ = 0;
	lastTickBufferedReceives_ = 0;
	proxyID_ = 0;
	strextra_ = "";
	channelType_ = CHANNEL_NORMAL;
	condemnReason_ = "";

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

	if (pPacketReader_)
		pPacketReader_->reset();

	if (!fina_kcp())
	{
		KBE_ASSERT(false);
	}

	flags_ = 0;
	pFilter_ = NULL;

	stopInactivityDetection();

	// 由于pEndPoint通常由外部给入，必须释放，频道重新激活时会重新赋值
	if(pEndPoint_)
	{
		pEndPoint_->destroySSL();
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
	static char dodgyString[MAX_BUF * 2] = { "None" };
	char tdodgyString[MAX_BUF] = { 0 };

	if (pEndPoint_ && !pEndPoint_->addr().isNone())
		pEndPoint_->addr().writeToString(tdodgyString, MAX_BUF);

	kbe_snprintf(dodgyString, MAX_BUF * 2, "%s/%d/%d/%d", tdodgyString, id_,
		this->condemn(), this->isDestroyed());

	return dodgyString;
}

//-------------------------------------------------------------------------------------
void Channel::clearBundle()
{
	Bundles::iterator iter = bundles_.begin();
	for(; iter != bundles_.end(); ++iter)
	{
		Bundle::reclaimPoolObject((*iter));
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
		case KCP_UPDATE:
		{
			kcpUpdate();
			break;
		}
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------
void Channel::sendto(bool reliable, Bundle* pBundle)
{
	KBE_ASSERT(protocoltype_ == PROTOCOL_UDP);

	if (isDestroyed())
	{
		ERROR_MSG(fmt::format("Channel::sendto({}): channel has destroyed.\n",
			this->c_str()));

		this->clearBundle();

		if (pBundle)
			Network::Bundle::reclaimPoolObject(pBundle);

		return;
	}

	if (condemn() > 0)
	{
		//WARNING_MSG(fmt::format("Channel::sendto: error, reason={}, from {}.\n", reasonToString(REASON_CHANNEL_CONDEMN), 
		//	c_str()));

		// this->clearBundle();

		if (pBundle)
			Network::Bundle::reclaimPoolObject(pBundle);

		return;
	}

	if (pBundle)
	{
		pBundle->pChannel(this);
		pBundle->finiMessage(true);
		bundles_.push_back(pBundle);
	}

	uint32 bundleSize = (uint32)bundles_.size();
	if (bundleSize == 0)
		return;

	if (pPacketSender_ == NULL)
	{
		pPacketSender_ = KCPPacketSender::createPoolObject(OBJECTPOOL_POINT);
		pPacketSender_->pEndPoint(pEndPoint_);
		pPacketSender_->pNetworkInterface(pNetworkInterface_);
	}
	else
	{
		if (pPacketSender_->type() != PacketSender::UDP_PACKET_SENDER)
		{
			TCPPacketSender::reclaimPoolObject((TCPPacketSender*)pPacketSender_);
			pPacketSender_ = KCPPacketSender::createPoolObject(OBJECTPOOL_POINT);
			pPacketSender_->pEndPoint(pEndPoint_);
			pPacketSender_->pNetworkInterface(pNetworkInterface_);
		}
	}

	pPacketSender_->processSend(this, reliable ? 1 : 0);
	sendCheck(bundleSize);
}

//-------------------------------------------------------------------------------------
void Channel::send(Bundle* pBundle)
{
	if (protocoltype_ == PROTOCOL_UDP)
	{
		sendto(true, pBundle);
		return;
	}

	if (isDestroyed())
	{
		ERROR_MSG(fmt::format("Channel::send({}): channel has destroyed.\n",
			this->c_str()));

		this->clearBundle();

		if (pBundle)
			Network::Bundle::reclaimPoolObject(pBundle);

		return;
	}

	if (condemn() > 0)
	{
		//WARNING_MSG(fmt::format("Channel::send: error, reason={}, from {}.\n", reasonToString(REASON_CHANNEL_CONDEMN), 
		//	c_str()));

		// this->clearBundle();

		if (pBundle)
			Network::Bundle::reclaimPoolObject(pBundle);

		return;
	}

	if (pBundle)
	{
		pBundle->pChannel(this);
		pBundle->finiMessage(true);
		bundles_.push_back(pBundle);
	}

	uint32 bundleSize = (uint32)bundles_.size();
	if (bundleSize == 0)
		return;

	if (!sending())
	{
		if (pPacketSender_ == NULL)
		{
			pPacketSender_ = TCPPacketSender::createPoolObject(OBJECTPOOL_POINT);
			pPacketSender_->pEndPoint(pEndPoint_);
			pPacketSender_->pNetworkInterface(pNetworkInterface_);
		}
		else
		{
			if (pPacketSender_->type() != PacketSender::TCP_PACKET_SENDER)
			{
				KCPPacketSender::reclaimPoolObject((KCPPacketSender*)pPacketSender_);
				pPacketSender_ = TCPPacketSender::createPoolObject(OBJECTPOOL_POINT);
				pPacketSender_->pEndPoint(pEndPoint_);
				pPacketSender_->pNetworkInterface(pNetworkInterface_);
			}
		}

		pPacketSender_->processSend(this, 0);

		// 如果不能立即发送到系统缓冲区，那么交给poller处理
		if (bundles_.size() > 0 && condemn() == 0 && !isDestroyed())
		{
			flags_ |= FLAG_SENDING;
			pNetworkInterface_->dispatcher().registerWriteFileDescriptor(*pEndPoint_, pPacketSender_);
		}
	}

	sendCheck(bundleSize);
}

//-------------------------------------------------------------------------------------
void Channel::sendCheck(uint32 bundleSize)
{
	if(this->isExternal())
	{
		if (Network::g_sendWindowMessagesOverflowCritical > 0 && bundleSize > Network::g_sendWindowMessagesOverflowCritical)
		{
			WARNING_MSG(fmt::format("Channel::sendCheck[{:p}]: external channel({}), send-window bufferedMessages has overflowed({} > {}).\n",
				(void*)this, this->c_str(), bundleSize, Network::g_sendWindowMessagesOverflowCritical));

			if (Network::g_extSendWindowMessagesOverflow > 0 &&
				bundleSize >  Network::g_extSendWindowMessagesOverflow)
			{
				ERROR_MSG(fmt::format("Channel::sendCheck[{:p}]: external channel({}), send-window bufferedMessages has overflowed({} > {}), Try adjusting the kbengine[_defs].xml->windowOverflow->send->messages.\n",
					(void*)this, this->c_str(), bundleSize, Network::g_extSendWindowMessagesOverflow));

				this->condemn("Channel::sendCheck: send-window bufferedMessages has overflowed!");
			}
		}

		if (g_extSendWindowBytesOverflow > 0)
		{
			uint32 bundleBytes = bundlesLength();
			if(bundleBytes >= g_extSendWindowBytesOverflow)
			{
				ERROR_MSG(fmt::format("Channel::sendCheck[{:p}]: external channel({}), bufferedBytes has overflowed({} > {}), Try adjusting the kbengine[_defs].xml->windowOverflow->send->bytes.\n",
					(void*)this, this->c_str(), bundleBytes, g_extSendWindowBytesOverflow));

				this->condemn("Channel::sendCheck: send-window bufferedBytes has overflowed!");
			}
		}
	}
	else
	{
		if (Network::g_sendWindowMessagesOverflowCritical > 0 && bundleSize > Network::g_sendWindowMessagesOverflowCritical)
		{
			if (Network::g_intSendWindowMessagesOverflow > 0 &&
				bundleSize > Network::g_intSendWindowMessagesOverflow)
			{
				ERROR_MSG(fmt::format("Channel::sendCheck[{:p}]: internal channel({}), send-window bufferedMessages has overflowed({} > {}).\n",
					(void*)this, this->c_str(), bundleSize, Network::g_intSendWindowMessagesOverflow));

				this->condemn("Channel::sendCheck: send-window bufferedMessages has overflowed!");
			}
			else
			{
				WARNING_MSG(fmt::format("Channel::sendCheck[{:p}]: internal channel({}), send-window bufferedMessages has overflowed({} > {}).\n",
					(void*)this, this->c_str(), bundleSize, Network::g_sendWindowMessagesOverflowCritical));
			}
		}

		if (g_intSendWindowBytesOverflow > 0)
		{
			uint32 bundleBytes = bundlesLength();
			if (bundleBytes >= g_intSendWindowBytesOverflow)
			{
				WARNING_MSG(fmt::format("Channel::sendCheck[{:p}]: internal channel({}), bufferedBytes has overflowed({} > {}).\n",
					(void*)this, this->c_str(), bundleBytes, g_intSendWindowBytesOverflow));
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
bool Channel::sending() const 
{
	if (pKCP())
	{
		return ikcp_waitsnd(pKCP()) > 0;
	}

	return (flags_ & FLAG_SENDING) > 0; 
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

	if (bytes > 0)
	{
		numBytesSent_ += bytes;
		g_numBytesSent += bytes;
		lastTickBytesSent_ += bytes;
	}

	if(this->isExternal())
	{
		if(g_extSentWindowBytesOverflow > 0 && 
			lastTickBytesSent_ >= g_extSentWindowBytesOverflow)
		{
			ERROR_MSG(fmt::format("Channel::onPacketSent[{:p}]: external channel({}), sentBytes has overflowed({} > {}), Try adjusting the kbengine[_defs].xml->windowOverflow->send->tickSentBytes.\n", 
				(void*)this, this->c_str(), lastTickBytesSent_, g_extSentWindowBytesOverflow));

			this->condemn("Channel::onPacketSent: sentBytes has overflowed!");
		}
	}
	else
	{
		if(g_intSentWindowBytesOverflow > 0 && 
			lastTickBytesSent_ >= g_intSentWindowBytesOverflow)
		{
			WARNING_MSG(fmt::format("Channel::onPacketSent[{:p}]: internal channel({}), sentBytes has overflowed({} > {}).\n", 
				(void*)this, this->c_str(), lastTickBytesSent_, g_intSentWindowBytesOverflow));
		}
	}
}

//-------------------------------------------------------------------------------------
void Channel::onPacketReceived(int bytes)
{
	lastReceivedTime_ = timestamp();
	++numPacketsReceived_;
	++g_numPacketsReceived;

	if (bytes > 0)
	{
		numBytesReceived_ += bytes;
		lastTickBytesReceived_ += bytes;
		g_numBytesReceived += bytes;
	}

	if(this->isExternal())
	{
		if(g_extReceiveWindowBytesOverflow > 0 && 
			lastTickBytesReceived_ >= g_extReceiveWindowBytesOverflow)
		{
			ERROR_MSG(fmt::format("Channel::onPacketReceived[{:p}]: external channel({}), bufferedBytes has overflowed({} > {}), Try adjusting the kbengine[_defs].xml->windowOverflow->receive.\n", 
				(void*)this, this->c_str(), lastTickBytesReceived_, g_extReceiveWindowBytesOverflow));

			this->condemn("Channel::onPacketReceived: bufferedBytes has overflowed!");
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
	++lastTickBufferedReceives_; 

	if(Network::g_receiveWindowMessagesOverflowCritical > 0 && lastTickBufferedReceives_ > Network::g_receiveWindowMessagesOverflowCritical)
	{
		if(this->isExternal())
		{
			if(Network::g_extReceiveWindowMessagesOverflow > 0 && 
				lastTickBufferedReceives_ > Network::g_extReceiveWindowMessagesOverflow)
			{
				ERROR_MSG(fmt::format("Channel::addReceiveWindow[{:p}]: external channel({}), receive window has overflowed({} > {}), Try adjusting the kbengine[_defs].xml->windowOverflow->receive->messages->external.\n", 
					(void*)this, this->c_str(), lastTickBufferedReceives_, Network::g_extReceiveWindowMessagesOverflow));

				this->condemn("Channel::addReceiveWindow: receive window has overflowed!");
			}
			else
			{
				WARNING_MSG(fmt::format("Channel::addReceiveWindow[{:p}]: external channel({}), receive window has overflowed({} > {}).\n", 
					(void*)this, this->c_str(), lastTickBufferedReceives_, Network::g_receiveWindowMessagesOverflowCritical));
			}
		}
		else
		{
			if(Network::g_intReceiveWindowMessagesOverflow > 0 && 
				lastTickBufferedReceives_ > Network::g_intReceiveWindowMessagesOverflow)
			{
				WARNING_MSG(fmt::format("Channel::addReceiveWindow[{:p}]: internal channel({}), receive window has overflowed({} > {}).\n", 
					(void*)this, this->c_str(), lastTickBufferedReceives_, Network::g_intReceiveWindowMessagesOverflow));
			}
		}
	}

	KBE_ASSERT(KBEngine::Network::MessageHandlers::pMainMessageHandlers);

	{
		AUTO_SCOPED_PROFILE("processRecvMessages");
		processPackets(KBEngine::Network::MessageHandlers::pMainMessageHandlers, pPacket);
	}
}

//-------------------------------------------------------------------------------------
void Channel::condemn(const std::string& reason, bool waitSendCompletedDestroy)
{
	if (condemnReason_.size() == 0)
		condemnReason_ = reason;

	flags_ |= (waitSendCompletedDestroy ? FLAG_CONDEMN_AND_WAIT_DESTROY : FLAG_CONDEMN);
}

//-------------------------------------------------------------------------------------
bool Channel::handshake(Packet* pPacket)
{
	if(hasHandshake())
		return false;

	if (protocoltype_ == PROTOCOL_TCP)
	{
		// https/wss
		if (!pEndPoint_->isSSL())
		{
			int sslVersion = KB_SSL::isSSLProtocal(pPacket);
			if (sslVersion != -1)
			{
				// 无论成功和失败都返回true，让外部回收数据包并继续等待握手
				pEndPoint_->setupSSL(sslVersion, pPacket);

				if (pPacket->length() == 0)
					return true;
			}
		}
		else
		{
			// 如果开启了ssl通讯，因目前只支持wss，所以必须等待websocket握手成功才算通过
			if (!websocket::WebSocketProtocol::isWebSocketProtocol(pPacket))
				return true;
		}

		flags_ |= FLAG_HANDSHAKE;

		// 此处判定是否为websocket或者其他协议的握手
		if (websocket::WebSocketProtocol::isWebSocketProtocol(pPacket))
		{
			channelType_ = CHANNEL_WEB;
			if (websocket::WebSocketProtocol::handshake(this, pPacket))
			{
				if (!pPacketReader_ || pPacketReader_->type() != PacketReader::PACKET_READER_TYPE_WEBSOCKET)
				{
					SAFE_RELEASE(pPacketReader_);
					pPacketReader_ = new WebSocketPacketReader(this);
				}

				pFilter_ = new WebSocketPacketFilter(this);
				DEBUG_MSG(fmt::format("Channel::handshake: websocket({}) successfully!\n", this->c_str()));

				// 无论如何都返回true，直到握手成功
				return true;
			}
			else
			{
				DEBUG_MSG(fmt::format("Channel::handshake: websocket({}) error!\n", this->c_str()));
			}
		}
	}
	else
	{
		if (protocolSubtype_ == SUB_PROTOCOL_KCP)
		{
			std::string hello;
			(*pPacket) >> hello;
			pPacket->clear(false);

			if (hello != UDP_HELLO)
			{
				// 这里不做处理，防止客户端在断线感应期间可能会发送一些包， 导致新的连接被握手失败从而再也无法通讯
				//DEBUG_MSG(fmt::format("Channel::handshake: kcp({}) error!\n", this->c_str()));
				//this->condemn();
			}
			else
			{
				UDPPacket* pHelloAckUDPPacket = UDPPacket::createPoolObject(OBJECTPOOL_POINT);
				(*pHelloAckUDPPacket) << Network::UDP_HELLO_ACK << KBEVersion::versionString() << (uint32)id();
				pEndPoint()->sendto(pHelloAckUDPPacket->data(), pHelloAckUDPPacket->length());
				UDPPacket::reclaimPoolObject(pHelloAckUDPPacket);

				if (!pPacketReader_ || pPacketReader_->type() != PacketReader::PACKET_READER_TYPE_KCP)
				{
					SAFE_RELEASE(pPacketReader_);
					pPacketReader_ = new KCPPacketReader(this);
				}

				DEBUG_MSG(fmt::format("Channel::handshake: kcp({}) successfully!\n", this->c_str()));
				flags_ |= FLAG_HANDSHAKE;
			}

			// 无论如何都返回true，直到握手成功
			return true;
		}
	}

	if(!pPacketReader_ || pPacketReader_->type() != PacketReader::PACKET_READER_TYPE_SOCKET)
	{
		SAFE_RELEASE(pPacketReader_);
		pPacketReader_ = new PacketReader(this);
	}

	return false;
}

//-------------------------------------------------------------------------------------
void Channel::updateTick(KBEngine::Network::MessageHandlers* pMsgHandlers)
{
	lastTickBytesReceived_ = 0;
	lastTickBytesSent_ = 0;
	lastTickBufferedReceives_ = 0;
}

//-------------------------------------------------------------------------------------
void Channel::processPackets(KBEngine::Network::MessageHandlers* pMsgHandlers, Packet* pPacket)
{
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

	if(this->condemn() > 0)
	{
		ERROR_MSG(fmt::format("Channel::processPackets({}): channel[{:p}] is condemn.\n", 
			this->c_str(), (void*)this));

		//this->destroy();
		return;
	}
	
	if(!hasHandshake())
	{
		if (handshake(pPacket))
		{
			RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);
			return;
		}
	}

	try
	{
		pPacketReader_->processMessages(pMsgHandlers, pPacket);
	}
	catch(MemoryStreamException &)
	{
		Network::MessageHandler* pMsgHandler = pMsgHandlers->find(pPacketReader_->currMsgID());
		WARNING_MSG(fmt::format("Channel::processPackets({}): packet invalid. currMsg=({}, id={}, len={}), currMsgLen={}\n",
			this->c_str()
			, (pMsgHandler == NULL ? "unknown" : pMsgHandler->name) 
			, pPacketReader_->currMsgID() 
			, (pMsgHandler == NULL ? -1 : pMsgHandler->msgLen) 
			, pPacketReader_->currMsgLen()));

		pPacketReader_->currMsgID(0);
		pPacketReader_->currMsgLen(0);
		condemn("Channel::processPackets: packet invalid!");
	}

	RECLAIM_PACKET(pPacket->isTCPPacket(), pPacket);
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
Bundle* Channel::createSendBundle()
{
	if(bundles_.size() > 0)
	{
		Bundle* pBundle = bundles_.back();
		Bundle::Packets& packets = pBundle->packets();

		// pBundle和packets[0]都必须是没有被对象池回收的对象
		// 必须是未经过加密的包，如果已经加密了就不要再重复拿出来用了，否则外部容易向其中添加未加密数据 
		if (pBundle->packetHaveSpace() &&
			!packets[0]->encrypted())
		{
			// 先从队列删除
			bundles_.pop_back();
			pBundle->pChannel(this);
			pBundle->pCurrMsgHandler(NULL);
			pBundle->currMsgPacketCount(0);
			pBundle->currMsgLength(0);
			pBundle->currMsgLengthPos(0);
			if (!pBundle->pCurrPacket())
			{
				Packet* pPacket = pBundle->packets().back();
				pBundle->packets().pop_back();
				pBundle->pCurrPacket(pPacket);
			}

			return pBundle;
		}
	}
	
	Bundle* pBundle = Bundle::createPoolObject(OBJECTPOOL_POINT);
	pBundle->pChannel(this);
	return pBundle;
}

//-------------------------------------------------------------------------------------

}
}
