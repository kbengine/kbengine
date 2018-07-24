// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_REDIS_WATCHER_H
#define KBE_REDIS_WATCHER_H

#include "db_interface_redis.h"

namespace KBEngine{ 

class RedisWatcher
{
public:
	static void querystatistics(const char* strCommand, uint32 size);
	static void initializeWatcher();
};

}
#endif // KBE_REDIS_WATCHER_H
