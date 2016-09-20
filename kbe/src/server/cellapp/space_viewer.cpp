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
#include "space.h"	
#include "entity.h"
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
reportLimitTimerHandle_(),
spaceViews_()
{
}

//-------------------------------------------------------------------------------------
SpaceViewers::~SpaceViewers()
{
	finalise();
}

//-------------------------------------------------------------------------------------
bool SpaceViewers::addTimer()
{
	if (!reportLimitTimerHandle_.isSet())
	{
		reportLimitTimerHandle_ = Cellapp::getSingleton().networkInterface().dispatcher().addTimer(
			1000000 / 10, this);

		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
void SpaceViewers::finalise()
{
	clear();
	reportLimitTimerHandle_.cancel();
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

	addTimer();
}

//-------------------------------------------------------------------------------------
void SpaceViewers::handleTimeout(TimerHandle handle, void * arg)
{
	if (spaceViews_.size() == 0)
	{
		reportLimitTimerHandle_.cancel();
		return;
	}

	std::map< Network::Address, SpaceViewer>::iterator iter = spaceViews_.begin();
	for (; iter != spaceViews_.end(); ++iter)
		iter->second.timeout();
}

//-------------------------------------------------------------------------------------
SpaceViewer::SpaceViewer():
addr_(),
spaceID_(0),
cellID_(0),
viewedEntities()
{
}

//-------------------------------------------------------------------------------------
SpaceViewer::~SpaceViewer()
{
}

//-------------------------------------------------------------------------------------
void SpaceViewer::resetViewer()
{
	viewedEntities.clear();
}

//-------------------------------------------------------------------------------------
void SpaceViewer::updateViewer(const Network::Address& addr, SPACE_ID spaceID, CELL_ID cellID)
{
	addr_ = addr;

	bool chagnedSpace = spaceID_ != spaceID;

	if (chagnedSpace)
	{
		onChangedSpaceOrCell();
		spaceID_ = spaceID;
	}

	if (cellID_ != cellID)
	{
		if (!chagnedSpace)
			onChangedSpaceOrCell();

		cellID_ = cellID;
	}
}

//-------------------------------------------------------------------------------------
void SpaceViewer::onChangedSpaceOrCell()
{
	resetViewer();
}

//-------------------------------------------------------------------------------------
void SpaceViewer::timeout()
{
	update();
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
	(*pBundle) << g_componentID;
	(*pBundle).append(s);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void SpaceViewer::update()
{
	if (spaceID_ == 0)
		return;

	Space* space = Spaces::findSpace(spaceID_);
	if (space == NULL || !space->isGood())
	{
		return;
	}

	// 获取本次与上次结果的差值，将差值放入stream中更新到客户端
	// 差值包括新增的实体，以及已经有的实体的位置变化
	MemoryStream s;

	sendStream(&s);
}

//-------------------------------------------------------------------------------------

}
