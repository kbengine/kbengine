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


#include "delayed_channels.hpp"
#include "network/channel.hpp"
#include "network/address.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"

namespace KBEngine{
namespace Mercury
{

//-------------------------------------------------------------------------------------
void DelayedChannels::init(EventDispatcher & dispatcher, NetworkInterface* pNetworkInterface)
{
	pNetworkInterface_ = pNetworkInterface;
	dispatcher.addFrequentTask( this );
}

//-------------------------------------------------------------------------------------
void DelayedChannels::fini(EventDispatcher & dispatcher)
{
	dispatcher.cancelFrequentTask( this );
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
	ChannelAddrs::iterator iter = channeladdrs_.begin();

	while (iter != channeladdrs_.end())
	{
		Channel * pChannel = pNetworkInterface_->findChannel((*iter));

		if (pChannel && (pChannel->isCondemn() || !pChannel->isDestroyed()))
		{
			pChannel->send();
		}

		++iter;
	}

	channeladdrs_.clear();
	return true;
}

//-------------------------------------------------------------------------------------
} 
}
