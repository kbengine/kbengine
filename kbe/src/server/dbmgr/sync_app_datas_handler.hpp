/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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

#ifndef __SYNC_APP_DATAS_HANDLER__
#define __SYNC_APP_DATAS_HANDLER__

// common include
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
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
		int32 startGroupOrder;
		int32 startGlobalOrder;
	};

	SyncAppDatasHandler(Mercury::NetworkInterface & networkInterface);
	~SyncAppDatasHandler();
	
	bool process();

	void pushApp(COMPONENT_ID cid, int32 startGroupOrder, int32 startGlobalOrder);
private:
	Mercury::NetworkInterface &		networkInterface_;
	uint64							lastRegAppTime_;
	std::vector<ComponentInitInfo>	apps_;

};


}
#endif
