// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
