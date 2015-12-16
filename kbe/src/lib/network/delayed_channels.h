/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#ifndef DELAYED_CHANNELS_H
#define DELAYED_CHANNELS_H

#include "common/tasks.h"
#include "common/smartpointer.h"
#include "common/singleton.h"

namespace KBEngine{
namespace Network
{
class Channel;
class Address;
class EventDispatcher;
class NetworkInterface;

class DelayedChannels : public Task
{
public:
	void init(EventDispatcher & dispatcher, NetworkInterface* pNetworkInterface);
	void fini(EventDispatcher & dispatcher);

	void add(Channel & channel);

	void sendIfDelayed(Channel & channel);

private:
	virtual bool process();

	typedef std::set<Address> ChannelAddrs;
	ChannelAddrs channeladdrs_;

	NetworkInterface* pNetworkInterface_;
};

}
}
#endif // DELAYED_CHANNELS_H
