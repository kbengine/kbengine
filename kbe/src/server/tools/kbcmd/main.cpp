// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "server/kbemain.h"
#include "kbcmd.h"
#include "client_sdk.h"
#include "entitydef/entitydef.h"

#undef DEFINE_IN_INTERFACE
#include "machine/machine_interface.h"
#define DEFINE_IN_INTERFACE
#include "machine/machine_interface.h"

#undef DEFINE_IN_INTERFACE
#include "client_lib/client_interface.h"
#define DEFINE_IN_INTERFACE
#include "client_lib/client_interface.h"

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
#include "loginapp/loginapp_interface.h"
#define DEFINE_IN_INTERFACE
#include "loginapp/loginapp_interface.h"

#undef DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.h"
#define DEFINE_IN_INTERFACE
#include "dbmgr/dbmgr_interface.h"

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

#define PARSE_COMMAND_ARG_BEGIN()	\
	for (int argIdx = 1; argIdx < argc; ++argIdx)	\
	{	\
		std::string cmd = argv[argIdx];	\
		std::string findcmd;	\
		std::string::size_type fi1;	\

#define PARSE_COMMAND_ARG_DO_FUNC(NAME, EXEC)	\
	cmd = argv[argIdx];	\
	findcmd = NAME;	\
	fi1 = cmd.find(findcmd); \
	if (fi1 != std::string::npos)	\
	{	\
		cmd.erase(fi1, findcmd.size());	\
		try \
		{ \
			if (EXEC == -1) \
				return -1; \
		} \
		catch (...) \
		{ \
			ERROR_MSG("parseCommandArgs: "#NAME"? invalid, no set! type is uint64\n"); \
		} \
		\
		continue; \
	} \

#define PARSE_COMMAND_ARG_DO_FUNC_RETURN(NAME, EXEC)	\
	cmd = argv[argIdx];	\
	findcmd = NAME;	\
	fi1 = cmd.find(findcmd); \
	if (fi1 != std::string::npos)	\
	{	\
		cmd.erase(fi1, findcmd.size());	\
		try \
		{ \
			return EXEC; \
		} \
		catch (...) \
		{ \
			ERROR_MSG("parseCommandArgs: "#NAME"? invalid, no set! type is uint64\n"); \
		} \
		\
		continue; \
	} \

#define PARSE_COMMAND_ARG_GET_VALUE(NAME, VAL)	\
	cmd = argv[argIdx];	\
	findcmd = NAME;	\
	fi1 = cmd.find(findcmd);	\
	if (fi1 != std::string::npos)	\
	{	\
		cmd.erase(fi1, findcmd.size());	\
		if (cmd.size() > 0)	\
		{	\
			try \
			{ \
				VAL = cmd;	\
			} \
			catch (...) \
			{ \
				ERROR_MSG("parseCommandArgs: "#NAME"? invalid, no set! type is uint64\n"); \
			} \
		} \
		 \
		continue; \
	} \

#define PARSE_COMMAND_ARG_END()	}

int process_make_client_sdk(int argc, char* argv[], const std::string clientType)
{
	Resmgr::getSingleton().initialize();
	setEvns();
	loadConfig();

	DebugHelper::initialize(g_componentType);

	INFO_MSG("-----------------------------------------------------------------------------------------\n\n\n");

	Resmgr::getSingleton().print();

	Network::EventDispatcher dispatcher;
	DebugHelper::getSingleton().pDispatcher(&dispatcher);

	Network::g_SOMAXCONN = g_kbeSrvConfig.tcp_SOMAXCONN(g_componentType);

	Network::NetworkInterface networkInterface(&dispatcher);

	DebugHelper::getSingleton().pNetworkInterface(&networkInterface);

	KBCMD app(dispatcher, networkInterface, g_componentType, g_componentID);

	START_MSG(COMPONENT_NAME_EX(g_componentType), g_componentID);

	if (!app.initialize())
	{
		ERROR_MSG("app::initialize(): initialization failed!\n");

		app.finalise();

		// 如果还有日志未同步完成， 这里会继续同步完成才结束
		DebugHelper::getSingleton().finalise();
		return -1;
	}

	std::vector<PyTypeObject*>	scriptBaseTypes;
	if (!EntityDef::initialize(scriptBaseTypes, g_componentType)) 
	{
		ERROR_MSG("app::initialize(): EntityDef initialization failed!\n");

		app.finalise();

		// 如果还有日志未同步完成， 这里会继续同步完成才结束
		DebugHelper::getSingleton().finalise();
		return -1;
	}

	std::string path = "";

	PARSE_COMMAND_ARG_BEGIN();
	PARSE_COMMAND_ARG_GET_VALUE("--outpath=", path);
	PARSE_COMMAND_ARG_END();

	ClientSDK* pClientSDK = ClientSDK::createClientSDK(clientType);
	
	int ret = 0;

	if (pClientSDK)
	{
		try
		{
			if (!pClientSDK->create(path))
			{
				ret = -1;
			}
		}
		catch (std::exception &err)
		{
			ERROR_MSG(fmt::format("app::initialize(): create clientsdk error({})!\n", err.what()));
		}
	}
	else
	{
		ERROR_MSG(fmt::format("app::initialize(): create clientsdk error! nonsupport type={}\n", clientType));
		ret = -1;
	}

	app.finalise();
	INFO_MSG(fmt::format("{}({}) has shut down. ClientSDK={}\n", COMPONENT_NAME_EX(g_componentType), g_componentID, pClientSDK->good()));

	// 如果还有日志未同步完成， 这里会继续同步完成才结束
	DebugHelper::getSingleton().finalise();

	if(pClientSDK)
		delete pClientSDK;

	return ret;
}

int process_getuid(int argc, char* argv[])
{
	if (getUserUID() == 0)
	{
		autoFixUserDigestUID();
	}

	setenv("UID", fmt::format("{}", getUserUID()).c_str(), 1);
	printf("%s", fmt::format("{}", getUserUID()).c_str());
	return getUserUID();
}

int process_help(int argc, char* argv[])
{
	printf("Usage:\n");
	printf("--clientsdk:\n");
	printf("\tAutomatically generate client code based on entity_defs file. Environment variables based on KBE.\n");
	printf("\tkbcmd.exe --clientsdk=unity --outpath=c:/unity_kbesdk\n");
	printf("\tkbcmd.exe --clientsdk=ue4 --outpath=c:/unity_kbesdk\n");
	printf("\tkbcmd.exe --clientsdk=ue4 --outpath=c:/unity_kbesdk --KBE_ROOT=\"*\"  --KBE_RES_PATH=\"*\"  --KBE_BIN_PATH=\"*\"\n");

	printf("\tkbcmd.exe --getuid\n");

	printf("\n--help:\n");
	printf("\tDisplay help information.\n");
	return 0;
}

int main(int argc, char* argv[])
{
	g_componentType = TOOL_TYPE;
	g_componentID = 0;

	if (argc == 1)
	{
		return process_help(argc, argv);
	}

	parseMainCommandArgs(argc, argv);

	PARSE_COMMAND_ARG_BEGIN();
	PARSE_COMMAND_ARG_DO_FUNC("--clientsdk=", process_make_client_sdk(argc, argv, cmd));
	PARSE_COMMAND_ARG_DO_FUNC_RETURN("--getuid", process_getuid(argc, argv));
	PARSE_COMMAND_ARG_DO_FUNC("--help", process_help(argc, argv));
	PARSE_COMMAND_ARG_END();

	return 0;
}

