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

//-------------------------------------------------------------------------------------
WatcherObject::WatcherObject(std::string path):
  path_(path),
  name_(),
  id_(0),
  s_(),
  numWitness_(0)
{
	std::string::size_type fi = path.find_first_of('/');
	if(fi == std::string::npos)
	{
		name_ = path;
		path_ = "";
	}
	else
	{
		std::vector<std::string> vec;
		KBEngine::strutil::kbe_split(path, '/', vec);

		std::vector<std::string>::size_type size = vec.size();
		name_ = vec[size - 1];
		path_ = path.erase(path.size() - name_.size() - 1, path.size());
	}
}

//-------------------------------------------------------------------------------------
WatcherObject::~WatcherObject(){
}
	
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
Watchers& Watchers::rootWatchers()
{
	static Watchers watchers;
	return watchers;
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
void Watchers::updateStream(MemoryStream* s)
{
}

//-------------------------------------------------------------------------------------
bool Watchers::addWatcher(std::string path, WatcherObject* pwo)
{
	if(hasWatcher(pwo->name()))
	{
		return false;
	}

	watcherObjs_[pwo->name()].reset(pwo);
	DEBUG_MSG(boost::format("Watchers::addWatcher: path=%1%, name=%2%, id=%3%\n") % 
		pwo->path() % pwo->name() % pwo->id());
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
void Watchers::readWatchers(MemoryStream* s)
{
	WATCHER_MAP::iterator iter = watcherObjs_.begin();
	for(; iter != watcherObjs_.end(); iter++)
	{
		iter->second->addToInitStream(s);
	}
}

//-------------------------------------------------------------------------------------
WatcherPaths::WatcherPaths():
watcherPaths_(),
watchers_()
{
}

//-------------------------------------------------------------------------------------
WatcherPaths::~WatcherPaths()
{
	watcherPaths_.clear();
}

//-------------------------------------------------------------------------------------
WatcherPaths& WatcherPaths::root()
{
	static WatcherPaths watcherPaths;
	return watcherPaths;
}

//-------------------------------------------------------------------------------------
void WatcherPaths::addToStream(MemoryStream* s)
{
}

//-------------------------------------------------------------------------------------
void WatcherPaths::updateStream(MemoryStream* s)
{
}

//-------------------------------------------------------------------------------------
bool WatcherPaths::addWatcher(std::string path, WatcherObject* pwo)
{
	std::string szpath, name;
	std::string::size_type fi = path.find_first_of('/');
	if(fi == std::string::npos)
	{
		name = path;
		szpath = "";
	}
	else
	{
		std::vector<std::string> vec;
		KBEngine::strutil::kbe_split(path, '/', vec);

		std::vector<std::string>::size_type size = vec.size();
		name = vec[size - 1];
		szpath = path.erase(path.size() - name.size() - 1, path.size());
	}

	static WATCHER_ID id = 1;
	pwo->id(id++);

	return _addWatcher(szpath, pwo);
}

//-------------------------------------------------------------------------------------
bool WatcherPaths::_addWatcher(std::string path, WatcherObject* pwo)
{
	if(path.size() > 0)
	{
		std::vector<std::string> vec;
		KBEngine::strutil::kbe_split(path, '/', vec);
		
		path.erase(0, vec[0].size() + 1);

		WATCHER_PATHS::iterator iter = watcherPaths_.find(vec[0]);
		if(iter != watcherPaths_.end())
		{
			return iter->second._addWatcher(path, pwo);
		}
		else
		{
			WatcherPaths& watcherPaths  = watcherPaths_[vec[0]];
			return watcherPaths._addWatcher(path, pwo);
		}
	}

	if(!watchers_.addWatcher(path, pwo))
	{
		KBE_ASSERT(false && "watcher is exist!\n");
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool WatcherPaths::addWatcherFromStream(std::string path, std::string name, 
										WATCHER_ID wid, WATCHERTYPE wtype, MemoryStream* s)
{
	WatcherObject* pWobj = NULL;

	std::string fullpath = "";

	if(path.size() == 0)
		fullpath = name;
	else
		fullpath = path + "/" + name;

	switch(wtype)
	{
	case WATCHER_TYPE_UINT8:
		pWobj = new WatcherValue<uint8>(fullpath);
		pWobj->updateStream<uint8>(s);
		break;
	case WATCHER_TYPE_UINT16:
		pWobj = new WatcherValue<uint16>(fullpath);
		pWobj->updateStream<uint16>(s);
		break;
	case WATCHER_TYPE_UINT32:
		pWobj = new WatcherValue<uint32>(fullpath);
		pWobj->updateStream<uint32>(s);
		break;
	case WATCHER_TYPE_UINT64:
		pWobj = new WatcherValue<uint64>(fullpath);
		pWobj->updateStream<uint64>(s);
		break;
	case WATCHER_TYPE_INT8:
		pWobj = new WatcherValue<int8>(fullpath);
		pWobj->updateStream<int8>(s);
		break;
	case WATCHER_TYPE_INT16:
		pWobj = new WatcherValue<int16>(fullpath);
		pWobj->updateStream<int16>(s);
		break;
	case WATCHER_TYPE_INT32:
		pWobj = new WatcherValue<int32>(fullpath);
		pWobj->updateStream<int32>(s);
		break;
	case WATCHER_TYPE_INT64:
		pWobj = new WatcherValue<int64>(fullpath);
		pWobj->updateStream<int64>(s);
		break;
	case WATCHER_TYPE_FLOAT:
		pWobj = new WatcherValue<float>(fullpath);
		pWobj->updateStream<float>(s);
		break;
	case WATCHER_TYPE_DOUBLE:
		pWobj = new WatcherValue<double>(fullpath);
		pWobj->updateStream<double>(s);
		break;
	case WATCHER_TYPE_CHAR:
		pWobj = new WatcherValue<char*>(fullpath);
		pWobj->updateStream<char*>(s);
		break;
	case WATCHER_TYPE_STRING:
		pWobj = new WatcherValue<std::string>(fullpath);
		pWobj->updateStream<std::string>(s);
		break;
	case WATCHER_TYPE_BOOL:
		pWobj = new WatcherValue<bool>(fullpath);
		pWobj->updateStream<bool>(s);
		break;
	case WATCHER_TYPE_COMPONENT_TYPE:
		pWobj = new WatcherValue<COMPONENT_TYPE>(fullpath);
		pWobj->updateStream<COMPONENT_TYPE>(s);
		break;
	default:
		KBE_ASSERT(false && "no support!\n");
	};

	pWobj->id(wid);
	return pWobj != NULL && _addWatcher(path, pWobj);
}

//-------------------------------------------------------------------------------------
bool WatcherPaths::delWatcher(std::string fullpath)
{
	if(hasWatcher(fullpath))
		return false;

	watcherPaths_.erase(fullpath);
	DEBUG_MSG(boost::format("WatcherPaths::delWatcher: %1%\n") % fullpath);
	return true;
}

//-------------------------------------------------------------------------------------
bool WatcherPaths::hasWatcher(std::string fullpath)
{
	WATCHER_PATHS::iterator iter = watcherPaths_.find(fullpath);
	return iter != watcherPaths_.end();
}

//-------------------------------------------------------------------------------------
void WatcherPaths::readWatchers(std::string path, MemoryStream* s)
{
	if(path.size() == 0)
	{
		watchers_.readWatchers(s);
	}
	else
	{
		std::vector<std::string> vec;
		KBEngine::strutil::kbe_split(path, '/', vec);
		
		path.erase(0, vec[0].size() + 1);
		readWatchers(path, s);
	}
}

//-------------------------------------------------------------------------------------		
}
