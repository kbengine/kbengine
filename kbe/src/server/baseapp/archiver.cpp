// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "baseapp.h"
#include "archiver.h"
#include "entity.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
Archiver::Archiver():
	archiveIndex_(INT_MAX),
	arEntityIDs_()
{
}

//-------------------------------------------------------------------------------------
Archiver::~Archiver()
{
}

//-------------------------------------------------------------------------------------
void Archiver::tick()
{
	int32 periodInTicks = (int32)secondsToTicks(ServerConfig::getSingleton().getBaseApp().archivePeriod, 0);
	if (periodInTicks == 0)
		return;

	if (archiveIndex_ >= periodInTicks)
	{
		this->createArchiveTable();
	}

	// 算法如下:
	// baseEntity的数量 * idx / tick周期 = 每次在vector中移动的一个区段
	// 这个区段在每个gametick进行处理, 刚好平滑的在periodInTicks中处理完任务
	// 如果archiveIndex_ >= periodInTicks则重新产生一次随机序列
	int size = (int)arEntityIDs_.size();
	int startIndex = size * archiveIndex_ / periodInTicks;

	++archiveIndex_;

	int endIndex   = size * archiveIndex_ / periodInTicks;

	for (int i = startIndex; i < endIndex; ++i)
	{
		Entity* pEntity = Baseapp::getSingleton().findEntity(arEntityIDs_[i]);
		
		if(pEntity && pEntity->hasDB())
		{
			this->archive(*pEntity);
		}
	}
}

//-------------------------------------------------------------------------------------
void Archiver::archive(Entity& entity)
{
	entity.writeToDB(NULL, NULL, NULL);

	if(entity.shouldAutoArchive() == KBE_NEXT_ONLY)
		entity.shouldAutoArchive(0);
}

//-------------------------------------------------------------------------------------
void Archiver::createArchiveTable()
{
	archiveIndex_ = 0;
	arEntityIDs_.clear();

	Entities<Entity>::ENTITYS_MAP::const_iterator iter = Baseapp::getSingleton().pEntities()->getEntities().begin();

	for(; iter != Baseapp::getSingleton().pEntities()->getEntities().end(); ++iter)
	{
		Entity* pEntity = static_cast<Entity*>(iter->second.get());

		if(pEntity->hasDB() && pEntity->shouldAutoArchive() > 0)
		{
			arEntityIDs_.push_back(iter->first);
		}
	}

	// 随机一下序列
	std::random_shuffle(arEntityIDs_.begin(), arEntityIDs_.end());
}

}
