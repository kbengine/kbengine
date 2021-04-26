// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "common/common.h"
#include "server/kbemain.h"
#include "machine.h"

#undef DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.h"
#define DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.h"

#undef DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.h"
#define DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.h"

#undef DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.h"
#define DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.h"

#undef DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.h"
#define DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.h"

#undef DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.h"
#define DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.h"

#undef DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.h"
#define DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.h"

#undef DEFINE_IN_INTERFACE
#include "tools/logger/logger_interface.h"
#define DEFINE_IN_INTERFACE
#include "tools/logger/logger_interface.h"

#undef DEFINE_IN_INTERFACE
#include "tools/interfaces/interfaces_interface.h"
#define DEFINE_IN_INTERFACE
#include "tools/interfaces/interfaces_interface.h"

#undef DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.h"
#define DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.h"

using namespace KBEngine;

int KBENGINE_MAIN(int argc, char* argv[])
{
#if KBE_PLATFORM != PLATFORM_WIN32
	rlimit rlimitData = { RLIM_INFINITY, RLIM_INFINITY };
	setrlimit(RLIMIT_CORE, &rlimitData);
#endif
	
	ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getKBMachine();
	int ret = kbeMainT<Machine>(argc, argv, MACHINE_TYPE, info.externalTcpPorts_min, 
		info.externalTcpPorts_max, -1, -1, "", 0, 0, info.internalInterface);
	return ret; 
}
