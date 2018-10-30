// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "helper/watcher.h"
#include "watch_obj_pools.h"
#include "witness.h"

namespace KBEngine { 

int32 watchWitnessPool_size()
{
	return (int)Witness::ObjPool().objects().size();
}

int32 watchWitnessPool_max()
{
	return (int)Witness::ObjPool().max();
}

int32 watchWitnessPool_totalAllocs()
{
	return (int)Witness::ObjPool().totalAllocs();
}

bool watchWitnessPool_isDestroyed()
{
	return Witness::ObjPool().isDestroyed();
}

uint32 watchWitnessPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Witness>::OBJECTS::const_iterator iter = Witness::ObjPool().objects().begin();
	for(; iter != Witness::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchEntityRefPool_size()
{
	return (int)EntityRef::ObjPool().objects().size();
}

int32 watchEntityRefPool_max()
{
	return (int)EntityRef::ObjPool().max();
}

int32 watchEntityRefPool_totalAllocs()
{
	return (int)EntityRef::ObjPool().totalAllocs();
}

bool watchEntityRefPool_isDestroyed()
{
	return EntityRef::ObjPool().isDestroyed();
}

uint32 watchEntityRefPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<EntityRef>::OBJECTS::const_iterator iter = EntityRef::ObjPool().objects().begin();
	for (; iter != EntityRef::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
std::string watch_celltracelogs()
{
	WatcherPaths::WATCHER_PATHS paths = WatcherPaths::root().watcherPaths()["root"]->watcherPaths();
	WatcherPaths::WATCHER_PATHS::iterator iter = paths.find("objectPools");

	if (iter == paths.end())
		return "NotFound";

	paths = iter->second->watcherPaths();
	iter = paths.begin();

	for (; iter != paths.end(); ++iter)
	{
		const std::string& pathName = iter->first;
		std::map<std::string, ObjectPoolLogPoint>* pLogPoints = NULL;

		if (pathName == "Witness")
		{
			pLogPoints = &Witness::ObjPool().logPoints();
		}
		else if (pathName == "EntityRef")
		{
			pLogPoints = &EntityRef::ObjPool().logPoints();
		}

		if (!pLogPoints)
			continue;

		std::map<std::string, ObjectPoolLogPoint>::const_iterator oiter = pLogPoints->begin();
		for (; oiter != pLogPoints->end(); ++oiter)
		{
			const std::string& pointName = oiter->first;

			Watchers::WATCHER_MAP& watchers = iter->second->watchers().watcherObjs();
			Watchers::WATCHER_MAP::iterator fiter = watchers.find(pointName);
			if (fiter != watchers.end())
				continue;

			WATCH_OBJECT(fmt::format("objectPools/{}/{}", pathName, pointName).c_str(), oiter->second.count);
		}
	}

	return "Collecting...";
}

//-------------------------------------------------------------------------------------
bool WatchObjectPool::initWatchPools()
{
	WATCH_OBJECT("objectPools/Witness/size", &watchWitnessPool_size);
	WATCH_OBJECT("objectPools/Witness/max", &watchWitnessPool_max);
	WATCH_OBJECT("objectPools/Witness/isDestroyed", &watchWitnessPool_isDestroyed);
	WATCH_OBJECT("objectPools/Witness/memory", &watchWitnessPool_bytes);
	WATCH_OBJECT("objectPools/Witness/totalAllocs", &watchWitnessPool_totalAllocs);

	WATCH_OBJECT("objectPools/EntityRef/size", &watchEntityRefPool_size);
	WATCH_OBJECT("objectPools/EntityRef/max", &watchEntityRefPool_max);
	WATCH_OBJECT("objectPools/EntityRef/isDestroyed", &watchEntityRefPool_isDestroyed);
	WATCH_OBJECT("objectPools/EntityRef/memory", &watchEntityRefPool_bytes);
	WATCH_OBJECT("objectPools/EntityRef/totalAllocs", &watchEntityRefPool_totalAllocs);

	WATCH_OBJECT("objectPools/CellTraceLogs", &watch_celltracelogs);
	return true;
}

//-------------------------------------------------------------------------------------
bool WatchObjectPool::finiWatchPools()
{
	return true;
}

//-------------------------------------------------------------------------------------

}
