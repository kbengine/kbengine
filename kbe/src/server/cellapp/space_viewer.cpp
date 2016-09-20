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


#include "cellapp.h"
#include "space_viewer.h"
#include "network/network_interface.h"
#include "network/event_dispatcher.h"
#include "network/address.h"
#include "network/network_stats.h"
#include "network/bundle.h"
#include "network/message_handler.h"
#include "common/memorystream.h"
#include "helper/console_helper.h"
#include "helper/profile.h"
#include "server/serverconfig.h"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
SpaceViewers::SpaceViewers():
spaceViews_()
{
}

//-------------------------------------------------------------------------------------
SpaceViewers::~SpaceViewers()
{
	clear();
}

//-------------------------------------------------------------------------------------
void SpaceViewers::updateSpaceViewer(const Network::Address& addr, SPACE_ID spaceID, CELL_ID cellID, bool del)
{
	if (del)
	{
		spaceViews_.erase(addr);
		return;
	}

	SpaceViewer& viewer = spaceViews_[addr];
	viewer.updateViewer(addr, spaceID, cellID);
}

//-------------------------------------------------------------------------------------
void SpaceViewers::handleTimeout(TimerHandle handle, void * arg)
{
	std::map< Network::Address, SpaceViewer>::iterator iter = spaceViews_.begin();
	for (; iter != spaceViews_.end(); ++iter)
		iter->second.timeout();
}

//-------------------------------------------------------------------------------------
SpaceViewer::SpaceViewer():
addr_()
{
}

//-------------------------------------------------------------------------------------
SpaceViewer::~SpaceViewer()
{
}

//-------------------------------------------------------------------------------------
void SpaceViewer::updateViewer(const Network::Address& addr, SPACE_ID spaceID, CELL_ID cellID)
{
	addr_ = addr;
}

//-------------------------------------------------------------------------------------
void SpaceViewer::timeout()
{
	update();

	MemoryStream s;
	sendStream(&s);
}

//-------------------------------------------------------------------------------------
void SpaceViewer::sendStream(MemoryStream* s)
{
	Network::Channel* pChannel = Cellapp::getSingleton().networkInterface().findChannel(addr_);
	if(pChannel == NULL)
	{
		WARNING_MSG(fmt::format("SpaceViewer::sendStream: not found addr({})\n",
			addr_.c_str()));

		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();

	ConsoleInterface::ConsoleQuerySpacesHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	(*pBundle) << g_componentType;
	(*pBundle).append(s);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void SpaceViewer::update()
{

}

//-------------------------------------------------------------------------------------

}
