// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "watcher.h"
#include "resmgr/resmgr.h"

namespace KBEngine{
WatcherPaths* pWatcherPaths = NULL;
Watchers* pWatchers = NULL;

//-------------------------------------------------------------------------------------
WatcherObject::WatcherObject(std::string path):
  path_(path),
  name_(),
  strval_(),
  id_(0),
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
	clear();
}

//-------------------------------------------------------------------------------------
void Watchers::clear()
{
	watcherObjs_.clear();
}

//-------------------------------------------------------------------------------------
Watchers& Watchers::rootWatchers()
{
	if(pWatchers == NULL)
		pWatchers = new Watchers;
	return *pWatchers;
}

//-------------------------------------------------------------------------------------
void Watchers::addToStream(MemoryStream* s)
{
	WATCHER_MAP::iterator iter = watcherObjs_.begin();
	for(; iter != watcherObjs_.end(); ++iter)
	{
		iter->second->addToStream(s);
	}
}

//-------------------------------------------------------------------------------------
void Watchers::updateStream(MemoryStream* s)
{
}

//-------------------------------------------------------------------------------------
bool Watchers::addWatcher(const std::string& path, WatcherObject* pwo)
{
	if(hasWatcher(pwo->name()))
	{
		return false;
	}

	watcherObjs_[pwo->name()].reset(pwo);

	//DEBUG_MSG(fmt::format("Watchers::addWatcher: path={}, name={}, id={}\n", 
	//	pwo->path(), pwo->name(), pwo->id()));

	return true;
}

//-------------------------------------------------------------------------------------
bool Watchers::delWatcher(const std::string& name)
{
	if(!hasWatcher(name))
		return false;

	watcherObjs_.erase(name);
	DEBUG_MSG(fmt::format("Watchers::delWatcher: {}\n", name));
	return true;
}

//-------------------------------------------------------------------------------------
bool Watchers::hasWatcher(const std::string& name)
{
	WATCHER_MAP::iterator iter = watcherObjs_.find(name);
	return iter != watcherObjs_.end();
}

//-------------------------------------------------------------------------------------
void Watchers::readWatchers(MemoryStream* s)
{
	WATCHER_MAP::iterator iter = watcherObjs_.begin();
	for(; iter != watcherObjs_.end(); ++iter)
	{
		iter->second->addToInitStream(s);
	}
}

//-------------------------------------------------------------------------------------
KBEShared_ptr< WatcherObject > Watchers::getWatcher(const std::string& name)
{
	WATCHER_MAP::iterator iter = watcherObjs_.find(name);
	if(iter != watcherObjs_.end())
		return iter->second;

	return KBEShared_ptr< WatcherObject > ((WatcherObject*)NULL);
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
	clear();
}

//-------------------------------------------------------------------------------------
void WatcherPaths::clear()
{
	watcherPaths_.clear();
}

//-------------------------------------------------------------------------------------
bool WatcherPaths::finalise()
{
	SAFE_RELEASE(pWatcherPaths);
	SAFE_RELEASE(pWatchers);
	return true;
}

//-------------------------------------------------------------------------------------
WatcherPaths& WatcherPaths::root()
{
	if(pWatcherPaths == NULL)
	{
		pWatcherPaths = new WatcherPaths();
	}

	return *pWatcherPaths;
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
			return iter->second->_addWatcher(path, pwo);
		}
		else
		{
			WatcherPaths* watcherPaths = new WatcherPaths();
			watcherPaths_[vec[0]].reset(watcherPaths);
			return watcherPaths->_addWatcher(path, pwo);
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
WatcherObject* WatcherPaths::addWatcherFromStream(std::string path, std::string name, 
										WATCHER_ID wid, WATCHER_VALUE_TYPE wtype, MemoryStream* s)
{
	WatcherObject* pWobj = NULL;

	std::string fullpath = "";

	if(path.size() == 0)
		fullpath = name;
	else
		fullpath = path + "/" + name;

	pWobj = new WatcherObject(fullpath);

	switch(wtype)
	{
	case WATCHER_VALUE_TYPE_UINT8:
		pWobj->updateStream<uint8>(s);
		break;
	case WATCHER_VALUE_TYPE_UINT16:
		pWobj->updateStream<uint16>(s);
		break;
	case WATCHER_VALUE_TYPE_UINT32:
		pWobj->updateStream<uint32>(s);
		break;
	case WATCHER_VALUE_TYPE_UINT64:
		pWobj->updateStream<uint64>(s);
		break;
	case WATCHER_VALUE_TYPE_INT8:
		pWobj->updateStream<int8>(s);
		break;
	case WATCHER_VALUE_TYPE_INT16:
		pWobj->updateStream<int16>(s);
		break;
	case WATCHER_VALUE_TYPE_INT32:
		pWobj->updateStream<int32>(s);
		break;
	case WATCHER_VALUE_TYPE_INT64:
		pWobj->updateStream<int64>(s);
		break;
	case WATCHER_VALUE_TYPE_FLOAT:
		pWobj->updateStream<float>(s);
		break;
	case WATCHER_VALUE_TYPE_DOUBLE:
		pWobj->updateStream<double>(s);
		break;
	case WATCHER_VALUE_TYPE_CHAR:
		pWobj->updateStream<char*>(s);
		break;
	case WATCHER_VALUE_TYPE_STRING:
		pWobj->updateStream<std::string>(s);
		break;
	case WATCHER_VALUE_TYPE_BOOL:
		pWobj->updateStream<bool>(s);
		break;
	case WATCHER_VALUE_TYPE_COMPONENT_TYPE:
		pWobj->updateStream<COMPONENT_TYPE>(s);
		break;
	default:
		KBE_ASSERT(false && "no support!\n");
	};

	pWobj->id(wid);
	bool ret = _addWatcher(path, pWobj);
	KBE_ASSERT(ret);

	return pWobj;
}

//-------------------------------------------------------------------------------------
bool WatcherPaths::delWatcher(const std::string& fullpath)
{
	if(hasWatcher(fullpath) == false)
	{
		DEBUG_MSG(fmt::format("WatcherPaths::delWatcher: not found {}\n", fullpath));
		return false;
	}

	std::vector<std::string> vec;
	KBEngine::strutil::kbe_split(fullpath, '/', vec);
	
	if(vec.size() == 1)
		return false;
	
	std::string name = (*(vec.end() - 1));
	vec.erase(vec.end() - 1);

	WatcherPaths* pCurrWatcherPaths = this;
	for(std::vector<std::string>::iterator iter = vec.begin(); iter != vec.end(); ++iter)
	{
		WATCHER_PATHS& paths = pCurrWatcherPaths->watcherPaths();
		KBEUnordered_map<std::string, KBEShared_ptr<WatcherPaths> >::iterator fiter = paths.find((*iter));
		if(fiter == paths.end())
			return false;

		pCurrWatcherPaths = fiter->second.get();
		if(pCurrWatcherPaths == NULL)
			return false;
	}
	
	return pCurrWatcherPaths->watchers().delWatcher(name);
}

//-------------------------------------------------------------------------------------
bool WatcherPaths::hasWatcher(const std::string& fullpath)
{
	std::vector<std::string> vec;
	KBEngine::strutil::kbe_split(fullpath, '/', vec);
	
	if(vec.size() == 1)
		return false;

	vec.erase(vec.end() - 1);

	WatcherPaths* pCurrWatcherPaths = this;
	for(std::vector<std::string>::iterator iter = vec.begin(); iter != vec.end(); ++iter)
	{
		WATCHER_PATHS& paths = pCurrWatcherPaths->watcherPaths();
		KBEUnordered_map<std::string, KBEShared_ptr<WatcherPaths> >::iterator fiter = paths.find((*iter));
		if(fiter == paths.end())
			return false;

		pCurrWatcherPaths = fiter->second.get();
		if(pCurrWatcherPaths == NULL)
			return false;
	}

	return pCurrWatcherPaths != NULL;
}

//-------------------------------------------------------------------------------------
KBEShared_ptr< WatcherObject > WatcherPaths::getWatcher(const std::string& fullpath)
{
	if(hasWatcher(fullpath) == false)
	{
		DEBUG_MSG(fmt::format("WatcherPaths::delWatcher: not found {}\n", fullpath));
		return KBEShared_ptr< WatcherObject > ((WatcherObject*)NULL);
	}

	std::vector<std::string> vec;
	KBEngine::strutil::kbe_split(fullpath, '/', vec);
	
	if(vec.size() == 1)
		return KBEShared_ptr< WatcherObject > ((WatcherObject*)NULL);
	
	std::string name = (*(vec.end() - 1));
	vec.erase(vec.end() - 1);

	WatcherPaths* pCurrWatcherPaths = this;
	for(std::vector<std::string>::iterator iter = vec.begin(); iter != vec.end(); ++iter)
	{
		WATCHER_PATHS& paths = pCurrWatcherPaths->watcherPaths();
		KBEUnordered_map<std::string, KBEShared_ptr<WatcherPaths> >::iterator fiter = paths.find((*iter));
		if(fiter == paths.end())
			return KBEShared_ptr< WatcherObject > ((WatcherObject*)NULL);

		pCurrWatcherPaths = fiter->second.get();
		if(pCurrWatcherPaths == NULL)
			return KBEShared_ptr< WatcherObject > ((WatcherObject*)NULL);
	}

	return pCurrWatcherPaths->watchers().getWatcher(name);
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
		
		if (vec.size() <= 0)
			return;

		path.erase(0, vec[0].size() + 1);

		WATCHER_PATHS::iterator iter = watcherPaths_.begin();
		for(; iter != watcherPaths_.end(); ++iter)
		{
			if(iter->first == vec[0])
			{
				iter->second->readWatchers(path, s);
				break;
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void WatcherPaths::dirPath(std::string path, std::vector<std::string>& vec)
{
	if(path.size() == 0)
	{
		WATCHER_PATHS::iterator iter = watcherPaths_.begin();
		for(; iter != watcherPaths_.end(); ++iter)
		{
			vec.push_back(iter->first);
		}

		Watchers::WATCHER_MAP& map = watchers_.watcherObjs();
		Watchers::WATCHER_MAP::iterator mapiter = map.begin();
		for(; mapiter != map.end(); ++mapiter)
		{
			vec.push_back(mapiter->first);
		}
	}
	else
	{
		std::vector<std::string> tvec;
		KBEngine::strutil::kbe_split(path, '/', tvec);
		
		path.erase(0, tvec[0].size() + 1);

		WATCHER_PATHS::iterator iter = watcherPaths_.begin();
		for(; iter != watcherPaths_.end(); ++iter)
		{
			if(iter->first == tvec[0])
			{
				iter->second->dirPath(path, vec);
				break;
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void WatcherPaths::readChildPaths(std::string srcPath, std::string path, MemoryStream* s)
{
	if(path.size() == 0)
	{
		if(srcPath.size() == 0)
			srcPath = "/";

		(*s) << srcPath;

		WATCHER_PATHS::iterator iter = watcherPaths_.begin();
		for(; iter != watcherPaths_.end(); ++iter)
		{
			(*s) << iter->first;
		}
	}
	else
	{
		std::vector<std::string> vec;
		KBEngine::strutil::kbe_split(path, '/', vec);
		
		if (vec.size() <= 0)
			return;

		path.erase(0, vec[0].size() + 1);

		WATCHER_PATHS::iterator iter = watcherPaths_.begin();
		for(; iter != watcherPaths_.end(); ++iter)
		{
			if(iter->first == vec[0])
			{
				iter->second->readChildPaths(srcPath, path, s);
				break;
			}
		}
	}
}

//-------------------------------------------------------------------------------------		
}
