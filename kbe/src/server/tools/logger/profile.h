// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_APP_PROFILE_H
#define KBE_APP_PROFILE_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "helper/profile.h"
#include "helper/eventhistory_stats.h"

namespace KBEngine{

extern ProfileVal SCRIPTCALL_PROFILE;

extern EventHistoryStats g_privateClientEventHistoryStats;
extern EventHistoryStats g_publicClientEventHistoryStats;

}
#endif
