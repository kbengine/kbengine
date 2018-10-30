// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "baseapp.h"
#include "backuper.h"
#include "server/serverconfig.h"

namespace KBEngine{	
float backupPeriod = 0.0;

//-------------------------------------------------------------------------------------
Backuper::Backuper():
backupEntityIDs_(),
backupRemainder_(0.f)
{
}

//-------------------------------------------------------------------------------------
Backuper::~Backuper()
{
	backupEntityIDs_.clear();
}

//-------------------------------------------------------------------------------------
void Backuper::tick()
{
	int32 periodInTicks = (int32)secondsToTicks(ServerConfig::getSingleton().getBaseApp().backupPeriod, 0);
	if (periodInTicks == 0)
		return;

	// 这里对备份的entity做一下分批操作
	// 大概算法是配置上填写的备份周期换算成tick数量， 每个tick备份一部分entity
	float numToBackUpFloat = float(Baseapp::getSingleton().pEntities()->size()) / periodInTicks + backupRemainder_;

	// 本次备份的数量
	int numToBackUp = int(numToBackUpFloat);

	// 计算出精度导致的损失数量
	backupRemainder_ = numToBackUpFloat - numToBackUp;

	// 如果备份表中没有内容了则重新产生一份新的
	if (backupEntityIDs_.empty())
	{
		this->createBackupTable();
	}

	MemoryStream* s = MemoryStream::createPoolObject(OBJECTPOOL_POINT);
	
	while((numToBackUp > 0) && !backupEntityIDs_.empty())
	{
		Entity * pEntity = Baseapp::getSingleton().findEntity(backupEntityIDs_.back());
		backupEntityIDs_.pop_back();
		
		if (pEntity && backup(*pEntity, *s))
		{
			--numToBackUp;
		}
		
		s->clear(false);
	}
	
	MemoryStream::reclaimPoolObject(s);
}

//-------------------------------------------------------------------------------------
bool Backuper::backup(Entity& entity, MemoryStream& s)
{
	// 这里开始将需要备份的数据写入流
	entity.writeBackupData(&s);

	if(entity.shouldAutoBackup() == KBE_NEXT_ONLY)
		entity.shouldAutoBackup(0);

	return true;
}

//-------------------------------------------------------------------------------------
void Backuper::createBackupTable()
{
	backupEntityIDs_.clear();

	Entities<Entity>::ENTITYS_MAP::const_iterator iter = Baseapp::getSingleton().pEntities()->getEntities().begin();

	for(; iter != Baseapp::getSingleton().pEntities()->getEntities().end(); ++iter)
	{
		Entity* pEntity = static_cast<Entity*>(iter->second.get());

		if(pEntity->shouldAutoBackup() > 0)
			backupEntityIDs_.push_back(iter->first);
	}

	// 随机一下序列
	std::random_shuffle(backupEntityIDs_.begin(), backupEntityIDs_.end());
}

}
