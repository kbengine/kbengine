// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SYNC_APP_DATAS_HANDLER_H
#define KBE_SYNC_APP_DATAS_HANDLER_H

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

class SyncAppDatasHandler : public Task
{
public:
	struct ComponentInitInfo
	{
		COMPONENT_ID cid;
		COMPONENT_ORDER startGroupOrder;
		COMPONENT_ORDER startGlobalOrder;
	};

	SyncAppDatasHandler(Network::NetworkInterface & networkInterface);
	~SyncAppDatasHandler();
	
	bool process();

	void pushApp(COMPONENT_ID cid, COMPONENT_ORDER startGroupOrder, COMPONENT_ORDER startGlobalOrder);

private:
	Network::NetworkInterface &		networkInterface_;
	uint64							lastRegAppTime_;
	std::vector<ComponentInitInfo>	apps_;

};


}

#endif // KBE_SYNC_APP_DATAS_HANDLER_H
