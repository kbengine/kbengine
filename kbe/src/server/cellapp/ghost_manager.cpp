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

#include "cellapp.hpp"
#include "ghost_manager.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
GhostManager::GhostManager(Mercury::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface),
realEntities_(),
ghost_route_(),
messages_(),
inTasks_(true)
{
	networkInterface.mainDispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
GhostManager::~GhostManager()
{
	if(inTasks_)
		networkInterface_.mainDispatcher().cancelFrequentTask(this);

	std::map<COMPONENT_ID, std::vector< Mercury::Bundle* > >::iterator iter = messages_.begin();
	for(; iter != messages_.end(); iter++)
	{
		std::vector< Mercury::Bundle* >::iterator iter1 = iter->second.begin();
		for(; iter1 != iter->second.end(); iter1++)
			Mercury::Bundle::ObjPool().reclaimObject((*iter1));
	}

	DEBUG_MSG("GhostManager::~GhostManager()\n");
	inTasks_ = false;
}

//-------------------------------------------------------------------------------------
void GhostManager::pushMessage(COMPONENT_ID componentID, Mercury::Bundle* pBundle)
{
	messages_[componentID].push_back(pBundle);

	if(!inTasks_)
	{
		inTasks_ = true;
		networkInterface_.mainDispatcher().addFrequentTask(this);
	}
}

//-------------------------------------------------------------------------------------
void GhostManager::pushRouteMessage(ENTITY_ID entityID, COMPONENT_ID componentID, Mercury::Bundle* pBundle)
{
	pushMessage(componentID, pBundle);
	addRoute(entityID, componentID);
}

//-------------------------------------------------------------------------------------
void GhostManager::addRoute(ENTITY_ID entityID, COMPONENT_ID componentID)
{
	ROUTE_INFO& info = ghost_route_[entityID];
	
	info.componentID = componentID;
	info.lastTime = timestamp();

	if(!inTasks_)
	{
		inTasks_ = true;
		networkInterface_.mainDispatcher().addFrequentTask(this);
	}
}

//-------------------------------------------------------------------------------------
COMPONENT_ID GhostManager::getRoute(ENTITY_ID entityID)
{
	std::map<ENTITY_ID, ROUTE_INFO>::iterator iter = ghost_route_.find(entityID);
	if(iter == ghost_route_.end())
		return 0;

	return iter->second.componentID;
}

//-------------------------------------------------------------------------------------
void GhostManager::checkRoute()
{
	std::map<ENTITY_ID, ROUTE_INFO>::iterator iter = ghost_route_.begin();
	for(; iter != ghost_route_.end(); iter++)
	{
		if(timestamp() - iter->second.lastTime > uint64( 5 * stampsPerSecond() ))
		{
			ghost_route_.erase(iter++);
		}
		else
		{
			++iter;
		}
	}
}

//-------------------------------------------------------------------------------------
void GhostManager::syncMessages()
{
	std::map<COMPONENT_ID, std::vector< Mercury::Bundle* > >::iterator iter = messages_.begin();
	for(; iter != messages_.end(); iter++)
	{
		std::vector< Mercury::Bundle* >::iterator iter1 = iter->second.begin();
		for(; iter1 != iter->second.end(); iter1++)
		{
			// 将消息同步到ghost
			Mercury::Bundle::ObjPool().reclaimObject((*iter1));
		}
			
		iter->second.clear();
	}

	messages_.clear();
}

//-------------------------------------------------------------------------------------
void GhostManager::syncGhosts()
{
	std::map<ENTITY_ID, Entity*>::iterator iter = realEntities_.begin();
	for(; iter != realEntities_.end(); )
	{
		COMPONENT_ID ghostCell = iter->second->ghostCell();
		if(ghostCell > 0)
		{
			// 将位置等信息同步到ghost
			++iter;
		}
		else
		{
			realEntities_.erase(iter++);
		}
	}
}

//-------------------------------------------------------------------------------------
bool GhostManager::process()
{
	if(messages_.size() == 0 && 
		ghost_route_.size() == 0 && 
		realEntities_.size() == 0)
	{
		inTasks_ = false;
		return inTasks_;
	}

	syncMessages();
	syncGhosts();
	checkRoute();

	return inTasks_;
}

//-------------------------------------------------------------------------------------

}
