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
GhostManager::GhostManager():
realEntities_(),
ghost_route_(),
messages_(),
pTimerHandle_(NULL),
checkTime_(0)
{
}

//-------------------------------------------------------------------------------------
GhostManager::~GhostManager()
{
	std::map<COMPONENT_ID, std::vector< Mercury::Bundle* > >::iterator iter = messages_.begin();
	for(; iter != messages_.end(); iter++)
	{
		std::vector< Mercury::Bundle* >::iterator iter1 = iter->second.begin();
		for(; iter1 != iter->second.end(); iter1++)
			Mercury::Bundle::ObjPool().reclaimObject((*iter1));
	}

	cancel();
}

//-------------------------------------------------------------------------------------
void GhostManager::cancel()
{
	if(pTimerHandle_)
	{
		pTimerHandle_->cancel();
		delete pTimerHandle_;
		pTimerHandle_ = NULL;

		DEBUG_MSG("GhostManager::cancel()\n");
	}
}

//-------------------------------------------------------------------------------------
void GhostManager::start()
{
	if(pTimerHandle_ == NULL)
	{
		pTimerHandle_ = new TimerHandle();
		(*pTimerHandle_) = Cellapp::getSingleton().mainDispatcher().addTimer(1000000 / g_kbeSrvConfig.getCellApp().ghostUpdateHertz, this,
								NULL);
	}
}

//-------------------------------------------------------------------------------------
void GhostManager::pushMessage(COMPONENT_ID componentID, Mercury::Bundle* pBundle)
{
	messages_[componentID].push_back(pBundle);
	start();
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

	start();
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
	for(; iter != ghost_route_.end(); )
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
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(iter->first);
		std::vector< Mercury::Bundle* >::iterator iter1 = iter->second.begin();

		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ERROR_MSG(boost::format("GhostManager::syncMessages: not found cellapp(%1%)!\n") % iter->first);
			
			for(; iter1 != iter->second.end(); iter1++)
				Mercury::Bundle::ObjPool().reclaimObject((*iter1));

			iter->second.clear();
			continue;
		}

		for(; iter1 != iter->second.end(); iter1++)
		{
			(*iter1)->send(Cellapp::getSingleton().networkInterface(), cinfos->pChannel);

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
			Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(ghostCell);
			if(cinfos == NULL || cinfos->pChannel == NULL)
			{
				ERROR_MSG(boost::format("GhostManager::syncGhosts: not found cellapp(%1%)!\n") % iter->first);
				continue;
			}

			++iter;
		}
		else
		{
			realEntities_.erase(iter++);
		}
	}
}

//-------------------------------------------------------------------------------------
void GhostManager::handleTimeout(TimerHandle, void * arg)
{
	if(timestamp() - checkTime_ > uint64( stampsPerSecond() * 0.1 ))
	{
		if(messages_.size() == 0 && 
			ghost_route_.size() == 0 && 
			realEntities_.size() == 0)
		{
			cancel();
			return;
		}

		checkRoute();
		checkTime_ = timestamp();
	}

	syncMessages();
	syncGhosts();
}

//-------------------------------------------------------------------------------------

}
