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
