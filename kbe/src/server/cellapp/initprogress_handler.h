// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CELLAPP_INIT_PROGRESS_HANDLER_H
#define KBE_CELLAPP_INIT_PROGRESS_HANDLER_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{

class InitProgressHandler : public Task
{
public:
	InitProgressHandler(Network::NetworkInterface & networkInterface);
	~InitProgressHandler();
	
	bool process();

	void start();

	struct PendingConnectEntityApp
	{
		COMPONENT_TYPE componentType;
		int32 uid;
		COMPONENT_ID componentID;
		int count;
	};

	void addPendingConnectEntityApps(const PendingConnectEntityApp& app) {
		pendingConnectEntityApps_.push_back(app);
	}

	void updateInfos(COMPONENT_ID componentID, COMPONENT_ORDER	startGlobalOrder, COMPONENT_ORDER startGroupOrder) {
		startGlobalOrder_ = startGlobalOrder;
		startGroupOrder_ = startGroupOrder;
		componentID_ = componentID;
	}

	bool sendRegisterNewApps();

private:
	Network::NetworkInterface & networkInterface_;
	int delayTicks_;
	bool cellappReady_;

	std::vector< PendingConnectEntityApp > pendingConnectEntityApps_;

	COMPONENT_ORDER	startGlobalOrder_;
	COMPONENT_ORDER	startGroupOrder_;
	COMPONENT_ID componentID_;

};


}

#endif // KBE_CELLAPP_INIT_PROGRESS_HANDLER_H
