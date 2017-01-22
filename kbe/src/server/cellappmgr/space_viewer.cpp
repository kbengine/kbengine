/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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


#include "cellappmgr.h"
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
		reportLimitTimerHandle_ = Cellappmgr::getSingleton().networkInterface().dispatcher().addTimer(
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
void SpaceViewers::updateSpaceViewer(const Network::Address& addr, SPACE_ID spaceID, bool del)
{
	if (del)
	{
		spaceViews_.erase(addr);
		return;
	}

	SpaceViewer& viewer = spaceViews_[addr];
	viewer.updateViewer(addr, spaceID);

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
	for (; iter != spaceViews_.end(); )
	{
		// �����viewer��ַ�Ҳ������������
		Network::Channel* pChannel = Cellappmgr::getSingleton().networkInterface().findChannel(iter->second.addr());
		if (pChannel == NULL)
		{
			spaceViews_.erase(iter++);
		}
		else
		{
			iter->second.timeout();
			++iter;
		}
	}
}

//-------------------------------------------------------------------------------------
SpaceViewer::SpaceViewer():
addr_(),
spaceID_(0)
{
}

//-------------------------------------------------------------------------------------
SpaceViewer::~SpaceViewer()
{
}

//-------------------------------------------------------------------------------------
void SpaceViewer::updateViewer(const Network::Address& addr, SPACE_ID spaceID)
{
	addr_ = addr;
	spaceID_ = spaceID;
}

//-------------------------------------------------------------------------------------
void SpaceViewer::timeout()
{
	updateClient();
}

//-------------------------------------------------------------------------------------
void SpaceViewer::sendStream(MemoryStream* s)
{
	Network::Channel* pChannel = Cellappmgr::getSingleton().networkInterface().findChannel(addr_);
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
void SpaceViewer::updateClient()
{
	if (spaceID_ == 0)
		return;

	MemoryStream s;

	std::map< COMPONENT_ID, Cellapp >& cellapps = Cellappmgr::getSingleton().cellapps();
	std::map< COMPONENT_ID, Cellapp >::iterator iter1 = cellapps.begin();
	for (; iter1 != cellapps.end(); ++iter1)
	{
		Cellapp& cellappref = iter1->second;
		Spaces& spaces = cellappref.spaces();

		std::map<SPACE_ID, Space>& allSpaces = spaces.spaces();
		std::map<SPACE_ID, Space>::iterator iter2 = allSpaces.begin();
		for (; iter2 != allSpaces.end(); ++iter2)
		{
			Space& space = iter2->second;
			
			if (space.id() != spaceID_)
				continue;

			// cellappID, spaceID, geomapping, cells
			s << iter1->first;
			s << space.id();
			s << space.getGeomappingPath();
			s << space.getScriptModuleName();

			Cells& cells = space.cells();
			std::map<CELL_ID, Cell>& allCells = cells.cells();
			s << allCells.size();

			std::map<CELL_ID, Cell>::iterator iter3 = allCells.begin();
			for (; iter3 != allCells.end(); ++iter3)
			{
				s << iter3->first;

				// ������Ϣ���ָ��ʵ�ֺ����
				// ����cell��С��״����Ϣ
			}
		}
	}

	sendStream(&s);
}

//-------------------------------------------------------------------------------------

}
