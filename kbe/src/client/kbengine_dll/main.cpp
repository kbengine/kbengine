
#include "cstdkbe/cstdkbe.hpp"

#undef DEFINE_IN_INTERFACE
#include "client_lib/client_interface.hpp"
#define DEFINE_IN_INTERFACE
#define CLIENT
#include "client_lib/client_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "baseapp/baseapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "cellapp/cellapp_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "baseappmgr/baseappmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.hpp"

#include "machine/machine_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "machine/machine_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "cellappmgr/cellappmgr_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "tools/message_log/messagelog_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "tools/message_log/messagelog_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "tools/billing_system/billingsystem_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "tools/billing_system/billingsystem_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "tools/bots/bots_interface.hpp"

#undef DEFINE_IN_INTERFACE
#include "resourcemgr/resourcemgr_interface.hpp"
#define DEFINE_IN_INTERFACE
#include "resourcemgr/resourcemgr_interface.hpp"

BOOL APIENTRY DllMain( HANDLE hModule,
					  DWORD ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		printf("\nprocess attach of dll");
		break;
	case DLL_THREAD_ATTACH:
		printf("\nthread attach of dll");
		break;
	case DLL_THREAD_DETACH:
		printf("\nthread detach of dll");
		break;
	case DLL_PROCESS_DETACH:
		printf("\nprocess detach of dll");
		break;
	}

	return TRUE;
} 