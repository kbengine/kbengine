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

#include "baseapp.h"
#include "archiver.h"
#include "base.h"

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

	// �㷨����:
	// base������ * idx / tick���� = ÿ����vector���ƶ���һ������
	// ���������ÿ��gametick���д���, �պ�ƽ������periodInTicks�д���������
	// ���archiveIndex_ >= periodInTicks�����²���һ���������
	int size = (int)arEntityIDs_.size();
	int startIndex = size * archiveIndex_ / periodInTicks;

	++archiveIndex_;

	int endIndex   = size * archiveIndex_ / periodInTicks;

	for (int i = startIndex; i < endIndex; ++i)
	{
		Base * pBase = Baseapp::getSingleton().findEntity(arEntityIDs_[i]);
		
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
	arEntityIDs_.clear();

	Entities<Base>::ENTITYS_MAP::const_iterator iter = Baseapp::getSingleton().pEntities()->getEntities().begin();

	for(; iter != Baseapp::getSingleton().pEntities()->getEntities().end(); ++iter)
	{
		Base* pBase = static_cast<Base*>(iter->second.get());

		if(pBase->hasDB() && pBase->shouldAutoArchive() > 0)
		{
			arEntityIDs_.push_back(iter->first);
		}
	}

	// ���һ������
	std::random_shuffle(arEntityIDs_.begin(), arEntityIDs_.end());
}

}
