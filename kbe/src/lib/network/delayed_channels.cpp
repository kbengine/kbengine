#include "delayed_channels.hpp"
#include "network/channel.hpp"
#include "network/event_dispatcher.hpp"

namespace KBEngine{
namespace Mercury
{
//-------------------------------------------------------------------------------------
void DelayedChannels::init( EventDispatcher & dispatcher )
{
	dispatcher.addFrequentTask( this );
}

//-------------------------------------------------------------------------------------
void DelayedChannels::fini( EventDispatcher & dispatcher )
{
	dispatcher.cancelFrequentTask( this );
}

//-------------------------------------------------------------------------------------
void DelayedChannels::add( Channel & channel )
{
	channels_.insert( &channel );
}

//-------------------------------------------------------------------------------------
void DelayedChannels::sendIfDelayed( Channel & channel )
{
	if (channels_.erase( &channel ) > 0)
	{
		channel.send();
	}
}

//-------------------------------------------------------------------------------------
void DelayedChannels::process()
{
	Channels::iterator iter = channels_.begin();

	while (iter != channels_.end())
	{
		Channel * pChannel = iter->get();

		if (!pChannel->isDead())
		{
			pChannel->send();
		}

		++iter;
	}

	channels_.clear();
}

//-------------------------------------------------------------------------------------
} 
}
