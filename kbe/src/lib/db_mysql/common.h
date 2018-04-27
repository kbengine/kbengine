// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_DB_MYSQL_COMMON_H
#define KBE_DB_MYSQL_COMMON_H

#include "db_context.h"
#include "common/common.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"

namespace KBEngine{ 

// 记录KBE所设置过的所有mysql标记，提供sync_item_to_db时检查设置项
extern uint32 ALL_MYSQL_SET_FLAGS;

}
#endif // KBE_DB_MYSQL_COMMON_H
