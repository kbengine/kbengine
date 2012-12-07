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

#include "watcher.hpp"
#include "resmgr/resmgr.hpp"

namespace KBEngine{
KBE_SINGLETON_INIT(Watchers);

Watchers _g_watchers;

//-------------------------------------------------------------------------------------
Watchers::Watchers():
watcherObjs_()
{
}

//-------------------------------------------------------------------------------------
Watchers::~Watchers()
{
	watcherObjs_.clear();
}

//-------------------------------------------------------------------------------------
void Watchers::addToStream(MemoryStream* s)
{
	WATCHER_MAP::iterator iter = watcherObjs_.begin();
	for(; iter != watcherObjs_.end(); iter++)
	{
		iter->second->addToStream(s);
	}
}

//-------------------------------------------------------------------------------------
void Watchers::updateFromStream(MemoryStream* s)
{
	while(s->opsize() > 0)
	{
		std::string name;
		(*s) >> name;
		
		WATCHER_MAP::iterator iter = watcherObjs_.find(name);
		if(iter != watcherObjs_.end())
		{
			iter->second->updateFromStream(s);
		}
	}
}

//-------------------------------------------------------------------------------------
bool Watchers::addWatcher(WatcherObject* pwo)
{
	if(hasWatcher(pwo->name()))
		return false;

	static WATCHER_ID id = 1;
	pwo->id(id++);

	watcherObjs_[pwo->name()].reset(pwo);

	DEBUG_MSG(boost::format("Watchers::addWatcher: %1%, id=%2%\n") % pwo->name() % pwo->id());
	return true;
}

//-------------------------------------------------------------------------------------
bool Watchers::delWatcher(std::string name)
{
	if(hasWatcher(name))
		return false;

	watcherObjs_.erase(name);
	DEBUG_MSG(boost::format("Watchers::delWatcher: %1%\n") % name);
	return true;
}

//-------------------------------------------------------------------------------------
bool Watchers::hasWatcher(std::string name)
{
	WATCHER_MAP::iterator iter = watcherObjs_.find(name);
	return iter != watcherObjs_.end();
}

//-------------------------------------------------------------------------------------		
}
