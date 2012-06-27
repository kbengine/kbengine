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
bool DelayedChannels::process()
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
	return true;
}

//-------------------------------------------------------------------------------------
} 
}
