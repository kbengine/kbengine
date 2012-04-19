#include "channel.hpp"
#ifndef CODE_INLINE
#include "channel.ipp"
#endif
#include "network/bundle.hpp"
#include "network/network_interface.hpp"
#include "network/tcp_packet_receiver.hpp"

namespace KBEngine { 
namespace Mercury
{
const int EXTERNAL_CHANNEL_SIZE = 256;
const int INTERNAL_CHANNEL_SIZE = 4096;
const int INDEXED_CHANNEL_SIZE = 512;


Channel::Channel(NetworkInterface & networkInterface,
		const EndPoint * endpoint, Traits traits,
		PacketFilterPtr pFilter, ChannelID id):
	pNetworkInterface_(&networkInterface),
	traits_(traits),
	id_(id),
	inactivityTimerHandle_(),
	inactivityExceptionPeriod_(0),
	lastReceivedTime_(0),
	pBundle_(NULL),
	windowSize_(	(traits != INTERNAL)    ? EXTERNAL_CHANNEL_SIZE :
					(id == CHANNEL_ID_NULL) ? INTERNAL_CHANNEL_SIZE :
											  INDEXED_CHANNEL_SIZE),

	bufferedReceives_(windowSize_),
	numBufferedReceives_(0),
	isDestroyed_(false),
	shouldDropNextSend_(false),
	// Stats
	numPacketsSent_(0),
	numPacketsReceived_(0),
	numBytesSent_(0),
	numBytesReceived_(0),
	pFilter_(pFilter),
	pEndPoint_(NULL),
	pPacketReceiver_(NULL)
{
	// This corresponds to the decRef in Channel::destroy.
	this->incRef();

	if (pFilter_ && id_ != CHANNEL_ID_NULL)
	{
		CRITICAL_MSG("Channel::Channel: "
			"PacketFilters are not supported on indexed channels (id:%d)\n",
			id_);
	}

	this->clearBundle();
	this->endpoint(endpoint);
	
	pPacketReceiver_ = new TCPPacketReceiver(*pEndPoint_, networkInterface);
	pNetworkInterface_->dispatcher().registerFileDescriptor(*pEndPoint_, pPacketReceiver_);
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
void Channel::endpoint(const EndPoint* endpoint)
{
	if (endpoint && pEndPoint_ != endpoint)
	{
		lastReceivedTime_ = timestamp();
		pEndPoint_ = const_cast<EndPoint*>(endpoint);
	}
}

//-------------------------------------------------------------------------------------
Channel::~Channel()
{
	pNetworkInterface_->onChannelGone(this);
	pNetworkInterface_->dispatcher().deregisterFileDescriptor(*pEndPoint_);
	pEndPoint_->close();
	pEndPoint_->detach();
	
	SAFE_RELEASE(pPacketReceiver_);
	SAFE_RELEASE(pBundle_);
	SAFE_RELEASE(pEndPoint_);
}

//-------------------------------------------------------------------------------------
void Channel::destroy()
{
	if(!isDestroyed_)
	{
		CRITICAL_MSG("is channel has Destroyed!");
		return;
	}

	isDestroyed_ = true;
	this->decRef();
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
	numBytesSent_ += pBundle->size();


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
	if (!pBundle_)
	{
		pBundle_ = new Bundle(pFilter_ ? pFilter_->maxSpareSize() : 0, this);
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
	}
}

//-------------------------------------------------------------------------------------
void Channel::reset(const EndPoint* endpoint, bool warnOnDiscard)
{
	// Don't do anything if the address hasn't changed.
	if (endpoint == pEndPoint_)
	{
		return;
	}

	// This handles registering this channel (deregistering done in
	// clearState above).
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
EventDispatcher & Channel::dispatcher()
{
	return pNetworkInterface_->mainDispatcher();
}

}
}