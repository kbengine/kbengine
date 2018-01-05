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
