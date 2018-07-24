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
bool WatchObjectPool::initWatchPools()
{
	WATCH_OBJECT("objectPools/Witness/size", &watchWitnessPool_size);
	WATCH_OBJECT("objectPools/Witness/max", &watchWitnessPool_max);
	WATCH_OBJECT("objectPools/Witness/isDestroyed", &watchWitnessPool_isDestroyed);
	WATCH_OBJECT("objectPools/Witness/memory", &watchWitnessPool_bytes);
	WATCH_OBJECT("objectPools/Witness/totalAllocs", &watchWitnessPool_totalAllocs);
	return true;
}

//-------------------------------------------------------------------------------------
bool WatchObjectPool::finiWatchPools()
{
	return true;
}

//-------------------------------------------------------------------------------------

}
