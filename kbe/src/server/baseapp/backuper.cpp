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

	// ����Ա��ݵ�entity��һ�·�������
	// ����㷨����������д�ı������ڻ����tick������ ÿ��tick����һ����entity
	float numToBackUpFloat = float(Baseapp::getSingleton().pEntities()->size()) / periodInTicks + backupRemainder_;

	// ���α��ݵ�����
	int numToBackUp = int(numToBackUpFloat);

	// ��������ȵ��µ���ʧ����
	backupRemainder_ = numToBackUpFloat - numToBackUp;

	// ������ݱ���û�������������²���һ���µ�
	if (backupEntityIDs_.empty())
	{
		this->createBackupTable();
	}

	MemoryStream* s = MemoryStream::createPoolObject();
	
	while((numToBackUp > 0) && !backupEntityIDs_.empty())
	{
		Base * pBase = Baseapp::getSingleton().findEntity(backupEntityIDs_.back());
		backupEntityIDs_.pop_back();
		
		if (pBase && backup(*pBase, *s))
		{
			--numToBackUp;
		}
		
		s->clear(false);
	}
	
	MemoryStream::reclaimPoolObject(s);
}

//-------------------------------------------------------------------------------------
bool Backuper::backup(Base& base, MemoryStream& s)
{
	// ���￪ʼ����Ҫ���ݵ�����д����
	base.writeBackupData(&s);

	if(base.shouldAutoBackup() == KBE_NEXT_ONLY)
		base.shouldAutoBackup(0);

	return true;
}

//-------------------------------------------------------------------------------------
void Backuper::createBackupTable()
{
	backupEntityIDs_.clear();

	Entities<Base>::ENTITYS_MAP::const_iterator iter = Baseapp::getSingleton().pEntities()->getEntities().begin();

	for(; iter != Baseapp::getSingleton().pEntities()->getEntities().end(); ++iter)
	{
		Base* pBase = static_cast<Base*>(iter->second.get());

		if(pBase->shouldAutoBackup() > 0)
			backupEntityIDs_.push_back(iter->first);
	}

	// ���һ������
	std::random_shuffle(backupEntityIDs_.begin(), backupEntityIDs_.end());
}

}
