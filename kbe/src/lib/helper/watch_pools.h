// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_WATCH_POOLS_HANDLER_H
#define KBE_WATCH_POOLS_HANDLER_H

#include "common/common.h"
#include "helper/debug_helper.h"

namespace KBEngine { 

class WatchPool
{
public:
	static bool initWatchPools();
	static bool finiWatchPools();
};

}

#endif
