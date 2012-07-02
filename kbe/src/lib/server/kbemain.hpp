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

#ifndef __KBEMAIN__
#define __KBEMAIN__
#include "serverapp.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "server/serverinfos.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "server/componentbridge.hpp"

namespace KBEngine{

inline void START_MSG(const char * name, uint64 appuid)
{
	ServerInfos serverInfo;
	
	INFO_MSG( "---- %-10s "
			"Version: %s. "
			"Config: %s. "
			"Built: %s %s. "
			"AppUID: %"PRAppID". "
			"UID: %d. "
			"PID: %d ----\n",
		name, KBEVersion::versionString().c_str(),
		KBE_CONFIG, __TIME__, __DATE__, 
		appuid, getUserUID(), getProcessPID() );
	
	INFO_MSG( "Server %s: %s with %s RAM\n",
		serverInfo.serverName().c_str(),
		serverInfo.cpuInfo().c_str(),
		serverInfo.memInfo().c_str() );
}

template <class SERVER_APP>
int kbeMainT(int argc, char * argv[], COMPONENT_TYPE componentType, 
			 uint16 listeningPort = 0, const char * listeningInterface = "")
{
	g_componentType = componentType;
	DebugHelper::initHelper(componentType);

	Mercury::EventDispatcher dispatcher;
	Mercury::NetworkInterface networkInterface(&dispatcher, 
		Mercury::NETWORK_INTERFACE_INTERNAL, htons(listeningPort), listeningInterface);
	
	uint64 appuid = genUUID64();
	Componentbridge* pComponentbridge = new Componentbridge(networkInterface, componentType, appuid);
	SERVER_APP app(dispatcher, networkInterface, componentType, appuid);
	START_MSG(COMPONENT_NAME[componentType], appuid);
	if(!app.initialize()){
		ERROR_MSG("app::initialize is error!\n");
		delete pComponentbridge;
		return -1;
	}
	
	INFO_MSG( "---- %s is running ----\n", COMPONENT_NAME[componentType]);
	int ret = app.run();
	delete pComponentbridge;
	app.finalise();
	INFO_MSG("%s has shut down.\n", COMPONENT_NAME[componentType]);
	return ret;
}

#define KBENGINE_MAIN										\
kbeMain(int argc, char* argv[]);							\
int main(int argc, char* argv[])							\
{															\
	return kbeMain(argc, argv);								\
}															\
int kbeMain

}

#endif // __KBEMAIN__