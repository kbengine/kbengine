/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#include "updatables.h"	
#include "helper/profile.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Updatables::Updatables()
{
}

//-------------------------------------------------------------------------------------
Updatables::~Updatables()
{
	clear();
}

//-------------------------------------------------------------------------------------
void Updatables::clear()
{
	objects_.clear();
}

//-------------------------------------------------------------------------------------
bool Updatables::add(Updatable* updatable)
{
	// 由于没有大量优先级需求，因此这里固定优先级数组
	if (objects_.size() == 0)
	{
		objects_.push_back(std::map<uint32, Updatable*>());
		objects_.push_back(std::map<uint32, Updatable*>());
	}

	KBE_ASSERT(updatable->updatePriority() < objects_.size());

	static uint32 idx = 1;
	std::map<uint32, Updatable*>& pools = objects_[updatable->updatePriority()];

	// 防止重复
	while (pools.find(idx) != pools.end())
		++idx;

	pools[idx] = updatable;

	// 记录存储位置
	updatable->removeIdx = idx++;

	return true;
}

//-------------------------------------------------------------------------------------
bool Updatables::remove(Updatable* updatable)
{
	std::map<uint32, Updatable*>& pools = objects_[updatable->updatePriority()];
	pools.erase(updatable->removeIdx);
	updatable->removeIdx = -1;
	return true;
}

//-------------------------------------------------------------------------------------
void Updatables::update()
{
	AUTO_SCOPED_PROFILE("callUpdates");

	std::vector< std::map<uint32, Updatable*> >::iterator fpIter = objects_.begin();
	for (; fpIter != objects_.end(); ++fpIter)
	{
		std::map<uint32, Updatable*>& pools = (*fpIter);
		std::map<uint32, Updatable*>::iterator iter = pools.begin();
		for (; iter != pools.end();)
		{
			if (!iter->second->update())
			{
				pools.erase(iter++);
			}
			else
			{
				++iter;
			}
		}
	}
}

//-------------------------------------------------------------------------------------
}
