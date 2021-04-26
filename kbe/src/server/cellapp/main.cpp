// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "server/kbemain.h"
#include "cellapp.h"

#undef DEFINE_IN_INTERFACE
#include "client_lib/client_interface.h"
#define DEFINE_IN_INTERFACE
#include "client_lib/client_interface.h"

#undef DEFINE_IN_INTERFACE
#include "machine/machine_interface.h"
#define DEFINE_IN_INTERFACE
#include "machine/machine_interface.h"

#undef DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.h"
#define DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.h"

#undef DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.h"
#define DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.h"

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
#include "tools/bots/bots_interface.h"
#define DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.h"

#undef DEFINE_IN_INTERFACE
#include "tools/interfaces/interfaces_interface.h"
#define DEFINE_IN_INTERFACE
#include "tools/interfaces/interfaces_interface.h"

using namespace KBEngine;

int KBENGINE_MAIN(int argc, char* argv[])
{
	ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getCellApp();
	int ret = kbeMainT<Cellapp>(argc, argv, CELLAPP_TYPE, -1, -1, -1, -1, "", 0, 0, info.internalInterface);
	return ret; 
}
