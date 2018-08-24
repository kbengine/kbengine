// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "cellapp.h"
#include "spacememory.h"	
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
	for (; iter != spaceViews_.end(); )
	{
		// 如果该viewer地址找不到了则将其擦除
		Network::Channel* pChannel = Cellapp::getSingleton().networkInterface().findChannel(iter->second.addr());
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
spaceID_(0),
cellID_(0),
viewedEntities(),
updateType_(0),
lastUpdateVersion_(0)
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
	lastUpdateVersion_ = 0;
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
	switch (updateType_)
	{
	case 0: // 初始化
		initClient();
		break;
	default: // 更新实体
		updateClient();
	};
}

//-------------------------------------------------------------------------------------
void SpaceViewer::sendStream(MemoryStream* s, int type)
{
	Network::Channel* pChannel = Cellapp::getSingleton().networkInterface().findChannel(addr_);
	if(pChannel == NULL)
	{
		WARNING_MSG(fmt::format("SpaceViewer::sendStream: not found addr({})\n",
			addr_.c_str()));

		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	ConsoleInterface::ConsoleQuerySpacesHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	(*pBundle) << g_componentType;
	(*pBundle) << g_componentID;
	(*pBundle) << type;
	(*pBundle).append(s);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void SpaceViewer::initClient()
{
	MemoryStream s;

	// 先下发脚本ID对应脚本模块的名称，便于降低后面实体同步量，实体只同步id过去
	const EntityDef::SCRIPT_MODULES& scriptModules = EntityDef::getScriptModules();
	s << (uint32)scriptModules.size();

	EntityDef::SCRIPT_MODULES::const_iterator moduleIter = scriptModules.begin();
	for (; moduleIter != scriptModules.end(); ++moduleIter)
	{
		s << moduleIter->get()->getUType();
		s << moduleIter->get()->getName();
	}

	sendStream(&s, updateType_);

	// 改变为更新实体
	updateType_ = 1;

	lastUpdateVersion_ = 0;
}

//-------------------------------------------------------------------------------------
void SpaceViewer::updateClient()
{
	if (spaceID_ == 0)
		return;

	SpaceMemory* space = SpaceMemorys::findSpace(spaceID_);
	if (space == NULL || !space->isGood())
	{
		return;
	}

	// 最多每次更新500个实体
	const int MAX_UPDATE_COUNT = 100;
	int updateCount = 0;

	// 获取本次与上次结果的差值，将差值放入stream中更新到客户端
	// 差值包括新增的实体，以及已经有的实体的位置变化
	MemoryStream s;

	Entities<Entity>* pEntities = Cellapp::getSingleton().pEntities();
	Entities<Entity>::ENTITYS_MAP& entitiesMap = pEntities->getEntities();

	// 先检查已经监视的实体，对于版本号较低的优先更新
	if (updateCount < MAX_UPDATE_COUNT)
	{
		std::map< ENTITY_ID, ViewEntity >::iterator viewerIter = viewedEntities.begin();
		for (; viewerIter != viewedEntities.end(); )
		{
			if (updateCount >= MAX_UPDATE_COUNT)
				break;

			ViewEntity& viewEntity = viewerIter->second;
			if (viewEntity.updateVersion > lastUpdateVersion_)
			{
				++viewerIter;
				continue;
			}

			Entities<Entity>::ENTITYS_MAP::iterator iter = entitiesMap.find(viewerIter->first);

			// 找不到实体， 说明已经销毁或者跑到其他进程了
			// 如果在其他进程， 其他进程会将其更新到客户端
			if (iter == entitiesMap.end())
			{
				s << viewerIter->first;
				s << false; // true为更新， false为销毁

				// 将其从viewedEntities删除
				viewedEntities.erase(viewerIter++);
			}
			else
			{
				Entity* pEntity = static_cast<Entity*>(iter->second.get());
				if (pEntity->spaceID() != spaceID_)
				{
					// 将其从viewedEntities删除
					viewedEntities.erase(viewerIter++);
					continue;
				}

				/*
				if (pEntity->cellID() != cellID_)
				{
					// 将其从viewedEntities删除
					viewedEntities.erase(viewerIter++);
					continue;
				}
				*/

				// 有新增的实体或者已经观察到的实体，检查位置变化
				// 如果没有变化则pass
				if ((viewEntity.position - pEntity->position()).length() <= 0.0004f &&
					(viewEntity.direction.dir - pEntity->direction().dir).length() <= 0.0004f)
				{
					++viewerIter;
					continue;
				}

				viewEntity.entityID = pEntity->id();
				viewEntity.position = pEntity->position();
				viewEntity.direction = pEntity->direction();
				++viewEntity.updateVersion;

				s << viewEntity.entityID;
				s << true; // true为更新， false为销毁
				s << pEntity->pScriptModule()->getUType();
				s << viewEntity.position.x << viewEntity.position.y << viewEntity.position.z;
				s << viewEntity.direction.roll() << viewEntity.direction.pitch() << viewEntity.direction.yaw();

				++updateCount;
				++viewerIter;
			}
		}
	}

	// 再检查是否有新增的实体
	if (updateCount < MAX_UPDATE_COUNT)
	{
		Entities<Entity>::ENTITYS_MAP::iterator iter = entitiesMap.begin();

		for (; iter != entitiesMap.end(); ++iter)
		{
			if (updateCount >= MAX_UPDATE_COUNT)
				break;

			Entity* pEntity = static_cast<Entity*>(iter->second.get());
			if (pEntity->spaceID() != spaceID_)
				continue;

			/*
			if (pEntity->cellID() != cellID_)
				continue;
			*/

			std::map< ENTITY_ID, ViewEntity >::iterator findIter = viewedEntities.find(pEntity->id());
			ViewEntity& viewEntity = viewedEntities[pEntity->id()];

			if (findIter != viewedEntities.end())
				continue;

			viewEntity.entityID = pEntity->id();
			viewEntity.position = pEntity->position();
			viewEntity.direction = pEntity->direction();
			viewEntity.updateVersion = lastUpdateVersion_ + 1;

			++updateCount;

			s << viewEntity.entityID;
			s << true; // true为更新， false为销毁
			s << pEntity->pScriptModule()->getUType();
			s << viewEntity.position.x << viewEntity.position.y << viewEntity.position.z;
			s << viewEntity.direction.roll() << viewEntity.direction.pitch() << viewEntity.direction.yaw();
		}
	}

	sendStream(&s, updateType_);

	// 如果全部更新完毕，更换版本号
	if (updateCount < MAX_UPDATE_COUNT)
		++lastUpdateVersion_;
}

//-------------------------------------------------------------------------------------

}
