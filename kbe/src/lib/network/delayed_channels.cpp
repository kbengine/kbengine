// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "delayed_channels.h"
#include "network/channel.h"
#include "network/address.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"

namespace KBEngine{
namespace Network
{

//-------------------------------------------------------------------------------------
void DelayedChannels::init(EventDispatcher & dispatcher, NetworkInterface* pNetworkInterface)
{
	pNetworkInterface_ = pNetworkInterface;
	dispatcher.addTask( this );
}

//-------------------------------------------------------------------------------------
void DelayedChannels::fini(EventDispatcher & dispatcher)
{
	dispatcher.cancelTask( this );
}

//-------------------------------------------------------------------------------------
void DelayedChannels::add(Channel & channel)
{
	channeladdrs_.insert(channel.addr());
}

//-------------------------------------------------------------------------------------
void DelayedChannels::sendIfDelayed(Channel & channel)
{
	if (channeladdrs_.erase(channel.addr()) > 0)
	{
		channel.send();
	}
}

//-------------------------------------------------------------------------------------
bool DelayedChannels::process()
{
	if (channeladdrs_.size() > 0)
	{
		ChannelAddrs::iterator iter = channeladdrs_.begin();

		while (iter != channeladdrs_.end())
		{
			Channel * pChannel = pNetworkInterface_->findChannel((*iter));

			if (pChannel && (pChannel->condemn() != Channel::FLAG_CONDEMN_AND_DESTROY && !pChannel->isDestroyed()))
			{
				pChannel->send();
			}

			++iter;
		}

		channeladdrs_.clear();
	}

	return true;
}

//-------------------------------------------------------------------------------------
} 
}
