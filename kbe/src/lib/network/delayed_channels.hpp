/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef __DELAYED_CHANNELS_HPP__
#define __DELAYED_CHANNELS_HPP__

#include "cstdkbe/tasks.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/singleton.hpp"

namespace KBEngine{
namespace Mercury
{
class Channel;
class EventDispatcher;

typedef SmartPointer<Channel> ChannelPtr;
class DelayedChannels : public Task
{
public:
	void init(EventDispatcher & dispatcher);
	void fini(EventDispatcher & dispatcher);

	void add(Channel & channel);

	void sendIfDelayed(Channel & channel);

private:
	virtual void process();

	typedef std::set<ChannelPtr> Channels;
	Channels channels_;
};

}
}
#endif // __DELAYED_CHANNELS_HPP__
