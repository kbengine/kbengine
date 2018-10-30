// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SYNC_ENTITY_STREAM_TEMPLATE_HANDLER_H
#define KBE_SYNC_ENTITY_STREAM_TEMPLATE_HANDLER_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"
// #define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine{

class SyncEntityStreamTemplateHandler : public Task
{
public:
	SyncEntityStreamTemplateHandler(Network::NetworkInterface & networkInterface);
	~SyncEntityStreamTemplateHandler();
	
	bool process();
private:
	Network::NetworkInterface & networkInterface_;

};


}

#endif // KBE_SYNC_ENTITY_STREAM_TEMPLATE_HANDLER_H
