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

#include "baseapp.h"
#include "archiver.h"
#include "base.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
Archiver::Archiver():
	archiveIndex_(INT_MAX),
	backupEntityIDs_()
{
}

//-------------------------------------------------------------------------------------
Archiver::~Archiver()
{
}

//-------------------------------------------------------------------------------------
void Archiver::tick()
{
	int32 periodInTicks = secondsToTicks(ServerConfig::getSingleton().getBaseApp().archivePeriod, 0);
	if (periodInTicks == 0)
		return;

	if (archiveIndex_ >= periodInTicks)
	{
		this->createArchiveTable();
	}

	// 算法如下:
	// base的数量 * idx / tick周期 = 每次在vector中移动的一个区段
	// 这个区段在每个gametick进行处理, 刚好平滑的在periodInTicks中处理完任务
	// 如果archiveIndex_ >= periodInTicks则重新产生一次随机序列
	int size = (int)backupEntityIDs_.size();
	int startIndex = size * archiveIndex_ / periodInTicks;

	++archiveIndex_;

	int endIndex   = size * archiveIndex_ / periodInTicks;

	for (int i = startIndex; i < endIndex; ++i)
	{
		Base * pBase = Baseapp::getSingleton().findEntity(backupEntityIDs_[i]);
		
		if(pBase && pBase->hasDB())
		{
			this->archive(*pBase);
		}
	}
}

//-------------------------------------------------------------------------------------
void Archiver::archive(Base& base)
{
	base.writeToDB(NULL, NULL, NULL);

	if(base.shouldAutoArchive() == KBE_NEXT_ONLY)
		base.shouldAutoArchive(0);
}

//-------------------------------------------------------------------------------------
void Archiver::createArchiveTable()
{
	archiveIndex_ = 0;
	backupEntityIDs_.clear();

	Entities<Base>::ENTITYS_MAP::const_iterator iter = Baseapp::getSingleton().pEntities()->getEntities().begin();

	for(; iter != Baseapp::getSingleton().pEntities()->getEntities().end(); ++iter)
	{
		Base* pBase = static_cast<Base*>(iter->second.get());

		if(pBase->hasDB() && pBase->shouldAutoArchive() > 0)
		{
			backupEntityIDs_.push_back(iter->first);
		}
	}

	// 随机一下序列
	std::random_shuffle(backupEntityIDs_.begin(), backupEntityIDs_.end());
}

}
