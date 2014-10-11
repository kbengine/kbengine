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


#include "cstdkbe/cstdkbe.hpp"
#include "server/kbemain.hpp"
#include "messagelog.hpp"

#undef DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "machine/machine_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "machine/machine_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "tools/billing_system/billingsystem_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "tools/billing_system/billingsystem_interface.hpp"

using namespace KBEngine;

int KBENGINE_MAIN(int argc, char* argv[])
{

#if KBE_PLATFORM != PLATFORM_WIN32
	rlimit rlimitData = { RLIM_INFINITY, RLIM_INFINITY };
	setrlimit(RLIMIT_CORE, &rlimitData);
#endif
	
	ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getMessagelog();
	int ret = kbeMainT<Messagelog>(argc, argv, MESSAGELOG_TYPE, info.externalPorts_min, 
		info.externalPorts_max, info.externalInterface, 0, info.internalInterface);
	return ret; 
}
