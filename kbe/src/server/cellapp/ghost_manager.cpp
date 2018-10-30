// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "cellapp.h"
#include "ghost_manager.h"
#include "entitydef/scriptdef_module.h"
#include "network/bundle.h"
#include "network/channel.h"

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
	std::map<COMPONENT_ID, std::vector< Network::Bundle* > >::iterator iter = messages_.begin();
	for(; iter != messages_.end(); ++iter)
	{
		std::vector< Network::Bundle* >::iterator iter1 = iter->second.begin();
		for(; iter1 != iter->second.end(); ++iter1)
			Network::Bundle::reclaimPoolObject((*iter1));
	}

	cancel();
}

//-------------------------------------------------------------------------------------
Network::Bundle* GhostManager::createSendBundle(COMPONENT_ID componentID)
{
	std::map<COMPONENT_ID, std::vector< Network::Bundle* > >::iterator iter = messages_.find(componentID);

	if (iter != messages_.end())
	{
		if (iter->second.size() > 0)
		{
			Network::Bundle* pBundle = iter->second.back();
			if (pBundle->packetHaveSpace())
			{
				// 先从队列删除
				iter->second.pop_back();
				pBundle->pChannel(NULL);
				pBundle->pCurrMsgHandler(NULL);
				pBundle->currMsgPacketCount(0);
				pBundle->currMsgLength(0);
				pBundle->currMsgLengthPos(0);
				return pBundle;
			}
		}
	}

	return Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
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
		(*pTimerHandle_) = Cellapp::getSingleton().dispatcher().addTimer(1000000 / g_kbeSrvConfig.getCellApp().ghostUpdateHertz, this,
								NULL);
	}
}

//-------------------------------------------------------------------------------------
void GhostManager::pushMessage(COMPONENT_ID componentID, Network::Bundle* pBundle)
{
	messages_[componentID].push_back(pBundle);
	start();
}

//-------------------------------------------------------------------------------------
void GhostManager::pushRouteMessage(ENTITY_ID entityID, COMPONENT_ID componentID, Network::Bundle* pBundle)
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
	std::map<COMPONENT_ID, std::vector< Network::Bundle* > >::iterator iter = messages_.begin();
	for(; iter != messages_.end(); ++iter)
	{
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(iter->first);
		std::vector< Network::Bundle* >::iterator iter1 = iter->second.begin();

		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ERROR_MSG(fmt::format("GhostManager::syncMessages: not found cellapp({})!\n", iter->first));
			
			for(; iter1 != iter->second.end(); ++iter1)
				Network::Bundle::reclaimPoolObject((*iter1));

			iter->second.clear();
			continue;
		}

		for(; iter1 != iter->second.end(); ++iter1)
		{
			// 将消息同步到ghost
			cinfos->pChannel->send((*iter1));
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
				ERROR_MSG(fmt::format("GhostManager::syncGhosts: not found cellapp({})!\n", iter->first));
				++iter;
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
