/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __KBEMAIN__
#define __KBEMAIN__
#include "serverapp.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "server/serverinfos.hpp"
#include "network/event_dispatcher.hpp"
namespace KBEngine{

inline void START_MSG(const char * name)
{
	ServerInfos serverInfo;
	
	INFO_MSG( "---- %-10s "
			"Version: %s. "
			"Config: %s. "
			"Built: %s %s. "
			"UID: %d. "
			"PID: %d ----\n",
		name, KBEVersion::versionString().c_str(),
		KBE_CONFIG, __TIME__, __DATE__, 
		getUserUID(), getProcessPID() );
	
	INFO_MSG( "Server %s: %s with %s RAM\n",
		serverInfo.serverName().c_str(),
		serverInfo.cpuInfo().c_str(),
		serverInfo.memInfo().c_str() );
}

template <class SERVER_APP>
int kbeMainT(int argc, char * argv[], COMPONENT_TYPE componentType)
{
	Mercury::EventDispatcher dispatcher;
	SERVER_APP app(dispatcher, componentType);
	START_MSG(COMPONENT_NAME[componentType]);
	if(!app.initialize()){
		ERROR_MSG("app::initialize is error!");
		return -1;
	}
	
	int ret = app.run();
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