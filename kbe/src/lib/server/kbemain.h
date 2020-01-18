// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_KBEMAIN_H
#define KBE_KBEMAIN_H

#include "helper/memory_helper.h"

#include "serverapp.h"
#include "Python.h"
#include "common/common.h"
#include "common/kbekey.h"
#include "common/stringconv.h"
#include "helper/debug_helper.h"
#include "network/event_dispatcher.h"
#include "network/message_handler.h"
#include "network/network_interface.h"
#include "server/components.h"
#include "server/machine_infos.h"
#include "server/id_component_querier.h"
#include "resmgr/resmgr.h"

#if KBE_PLATFORM == PLATFORM_WIN32
#include "helper/crashhandler.h"
#endif

namespace KBEngine{

inline void START_MSG(const char * name, uint64 appuid)
{
	MachineInfos machineInfo;
	
	std::string s = (fmt::format("---- {} "
			"Version: {}. "
			"ScriptVersion: {}. "
			"Pythoncore: {}. "
			"Protocol: {}. "
			"Config: {} {}. "
			"Built: {} {}. "
			"AppID: {}. "
			"UID: {}. "
			"PID: {} ----\n",
		name, KBEVersion::versionString(), KBEVersion::scriptVersionString(), PY_VERSION,
		Network::MessageHandlers::getDigestStr(),
		KBE_CONFIG, KBE_ARCH, __TIME__, __DATE__,
		appuid, getUserUID(), getProcessPID()));

	INFO_MSG(s);
	
#if KBE_PLATFORM == PLATFORM_WIN32
	printf("%s", s.c_str());
#endif

	s = (fmt::format("Server {}: {} with {} RAM\n",
		machineInfo.machineName(),
		machineInfo.cpuInfo(),
		machineInfo.memInfo()));

	INFO_MSG(s);

#if KBE_PLATFORM == PLATFORM_WIN32
	printf("%s\n", s.c_str());
#endif

}

inline void loadConfig()
{
	Resmgr::getSingleton().initialize();

	// "../../res/server/kbengine_defaults.xml"
	g_kbeSrvConfig.loadConfig("server/kbengine_defaults.xml");

	// "../../../assets/res/server/kbengine.xml"
	g_kbeSrvConfig.loadConfig("server/kbengine.xml");
}

inline void setEvns()
{
	std::string scomponentGroupOrder = "0";
	std::string scomponentGlobalOrder = "0";
	std::string scomponentID = "0";

	if(g_componentGroupOrder > 0)
	{
		int32 icomponentGroupOrder = g_componentGroupOrder;
		scomponentGroupOrder = KBEngine::StringConv::val2str(icomponentGroupOrder);
	}
	
	if(g_componentGlobalOrder > 0)
	{
		int32 icomponentGlobalOrder = g_componentGlobalOrder;
		scomponentGlobalOrder = KBEngine::StringConv::val2str(icomponentGlobalOrder);
	}

	{
		uint64 v = g_componentID;
		scomponentID = KBEngine::StringConv::val2str(v);
	}

	setenv("KBE_COMPONENTID", scomponentID.c_str(), 1);
	setenv("KBE_BOOTIDX_GLOBAL", scomponentGlobalOrder.c_str(), 1);
	setenv("KBE_BOOTIDX_GROUP", scomponentGroupOrder.c_str(), 1);
}

inline bool checkComponentID(COMPONENT_TYPE componentType)
{
	if (getUserUID() <= 0)
		autoFixUserDigestUID();

	int32 uid = getUserUID();
	if ((componentType == MACHINE_TYPE || componentType == LOGGER_TYPE) && g_componentID == (COMPONENT_ID)-1)
	{
		int macMD5 = getMacMD5();
		
		COMPONENT_ID cid1 = (COMPONENT_ID)uid * COMPONENT_ID_MULTIPLE;
		COMPONENT_ID cid2 = (COMPONENT_ID)macMD5 * 10000;
		COMPONENT_ID cid3 = (COMPONENT_ID)componentType * 100;
		g_componentID = cid1 + cid2 + cid3 + 1;
	}
	else
	{
		if (g_componentID == (COMPONENT_ID)-1)
		{
			IDComponentQuerier cidQuerier;
			if (cidQuerier.good())
			{
				g_componentID = cidQuerier.query(componentType, uid);
				if (g_componentID <= 0)
					return false;
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

template <class SERVER_APP>
int kbeMainT(int argc, char * argv[], COMPONENT_TYPE componentType, 
	int32 extlisteningTcpPort_min = -1, int32 extlisteningTcpPort_max = -1, 
	int32 extlisteningUdpPort_min = -1, int32 extlisteningUdpPort_max = -1, const char * extlisteningInterface = "",
	int32 intlisteningPort_min = 0, int32 intlisteningPort_max = 0, const char * intlisteningInterface = "")
{
	int getuid = getUserUID();

	bool success = checkComponentID(componentType);
	if (!success)
		return -1;

	setEvns();
	startLeakDetection(componentType, g_componentID);

	g_componentType = componentType;
	DebugHelper::initialize(componentType);

	INFO_MSG( "-----------------------------------------------------------------------------------------\n\n\n");

	std::string publicKeyPath = Resmgr::getSingleton().getPyUserResPath() + "key/" + "kbengine_public.key";
	std::string privateKeyPath = Resmgr::getSingleton().getPyUserResPath() + "key/" + "kbengine_private.key";

	bool isExsit = access(publicKeyPath.c_str(), 0) == 0 && access(privateKeyPath.c_str(), 0) == 0;
	if (!isExsit)
	{
		publicKeyPath = Resmgr::getSingleton().matchPath("key/") + "kbengine_public.key";
		privateKeyPath = Resmgr::getSingleton().matchPath("key/") + "kbengine_private.key";
	}
	
	KBEKey kbekey(publicKeyPath, privateKeyPath);

	Resmgr::getSingleton().print();

	Network::EventDispatcher dispatcher;
	DebugHelper::getSingleton().pDispatcher(&dispatcher);

	const ChannelCommon& channelCommon = g_kbeSrvConfig.channelCommon();

	Network::g_SOMAXCONN = g_kbeSrvConfig.tcp_SOMAXCONN(g_componentType);

	Network::NetworkInterface networkInterface(&dispatcher, 
		extlisteningTcpPort_min, extlisteningTcpPort_max, extlisteningUdpPort_min, extlisteningUdpPort_max, extlisteningInterface,
		channelCommon.extReadBufferSize, channelCommon.extWriteBufferSize,
		intlisteningPort_min, intlisteningPort_max, intlisteningInterface,
		channelCommon.intReadBufferSize, channelCommon.intWriteBufferSize);
	
	DebugHelper::getSingleton().pNetworkInterface(&networkInterface);

	g_kbeSrvConfig.updateInfos(true, componentType, g_componentID, 
			networkInterface.intTcpAddr(), networkInterface.extTcpAddr(), networkInterface.extUdpAddr());
	
	if (getuid <= 0)
	{
		WARNING_MSG(fmt::format("invalid UID({}) <= 0, please check UID for environment! automatically set to {}.\n", getuid, getUserUID()));
	}

	Components::getSingleton().initialize(&networkInterface, componentType, g_componentID);
	
	SERVER_APP app(dispatcher, networkInterface, componentType, g_componentID);
	Components::getSingleton().findLogger();
	START_MSG(COMPONENT_NAME_EX(componentType), g_componentID);

	if(!app.initialize())
	{
		ERROR_MSG("app::initialize(): initialization failed!\n");

		Components::getSingleton().finalise();
		app.finalise();

		// 如果还有日志未同步完成， 这里会继续同步完成才结束
		DebugHelper::getSingleton().finalise();

#if KBE_PLATFORM == PLATFORM_WIN32
		// 等待几秒，让用户能够在窗口上看到信息
		Beep(587, 500);
		KBEngine::sleep(5000);
#endif
		return -1;
	}
	
	INFO_MSG(fmt::format("---- {} is running ----\n", COMPONENT_NAME_EX(componentType)));

#if KBE_PLATFORM == PLATFORM_WIN32
	printf("[INFO]: %s", (fmt::format("---- {} is running ----\n", COMPONENT_NAME_EX(componentType))).c_str());
#endif
	int ret = app.run();
	
	Components::getSingleton().finalise();
	app.finalise();
	INFO_MSG(fmt::format("{}({}) has shut down.\n", COMPONENT_NAME_EX(componentType), g_componentID));

	// 如果还有日志未同步完成， 这里会继续同步完成才结束
	DebugHelper::getSingleton().finalise();
	return ret;
}

inline void parseMainCommandArgs(int argc, char* argv[])
{
	if(argc < 2)
	{
		return;
	}

	bool isSeted = false;
	for(int argIdx=1; argIdx<argc; ++argIdx)
	{
		std::string cmd = argv[argIdx];
		
		std::string findcmd = "--cid=";
		std::string::size_type fi1 = cmd.find(findcmd);
		if(fi1 != std::string::npos)
		{
			cmd.erase(fi1, findcmd.size());
			if(cmd.size() > 0)
			{
				COMPONENT_ID cid = 0;
				try
				{
					StringConv::str2value(cid, cmd.c_str());
					g_componentID = cid;
					isSeted = true;
				}
				catch(...)
				{
					ERROR_MSG("parseCommandArgs: --cid=? invalid, no set! type is uint64\n");
				}
			}

			continue;
		}
		else
		{
			if (!isSeted)
				g_componentID = (COMPONENT_ID)-1;
		}

		findcmd = "--gus=";
		fi1 = cmd.find(findcmd);
		if(fi1 != std::string::npos)
		{
			cmd.erase(fi1, findcmd.size());
			if(cmd.size() > 0)
			{
				int32 gus = 0;
				try
				{
					StringConv::str2value(gus, cmd.c_str());

					KBE_ASSERT(gus <= 65535);
					g_genuuid_sections = gus;
				}
				catch(...)
				{
					ERROR_MSG("parseCommandArgs: --gus=? invalid, no set! type is uint16\n");
				}
			}

			continue;
		}

		findcmd = "--hide=";
		fi1 = cmd.find(findcmd);
		if (fi1 != std::string::npos)
		{
			cmd.erase(fi1, findcmd.size());
			if (cmd.size() > 0)
			{
				int32 hide = 0;
				try
				{
					StringConv::str2value(hide, cmd.c_str());
				}
				catch (...)
				{
					ERROR_MSG("parseCommandArgs: --hide=? invalid, no set! type is int8\n");
				}

				if (hide > 0)
				{
#if KBE_PLATFORM == PLATFORM_WIN32
					ShowWindow(GetConsoleWindow(), SW_HIDE);
#else
#endif
				}
			}

			continue;
		}

		findcmd = "--KBE_ROOT=";
		fi1 = cmd.find(findcmd);
		if (fi1 != std::string::npos)
		{
			cmd.erase(fi1, findcmd.size());
			if (cmd.size() > 0)
			{
				setenv("KBE_ROOT", cmd.c_str(), 1);
			}

			continue;
		}

		findcmd = "--KBE_RES_PATH=";
		fi1 = cmd.find(findcmd);
		if (fi1 != std::string::npos)
		{
			cmd.erase(fi1, findcmd.size());
			if (cmd.size() > 0)
			{
				setenv("KBE_RES_PATH", cmd.c_str(), 1);
			}

			continue;
		}

		findcmd = "--KBE_BIN_PATH=";
		fi1 = cmd.find(findcmd);
		if (fi1 != std::string::npos)
		{
			cmd.erase(fi1, findcmd.size());
			if (cmd.size() > 0)
			{
				setenv("KBE_BIN_PATH", cmd.c_str(), 1);
			}

			continue;
		}
	}
}

#if KBE_PLATFORM == PLATFORM_WIN32
#define KBENGINE_MAIN																									\
kbeMain(int argc, char* argv[]);																						\
int main(int argc, char* argv[])																						\
{																														\
	loadConfig();																										\
	g_componentID = genUUID64();																						\
	parseMainCommandArgs(argc, argv);																					\
	char dumpname[MAX_BUF] = {0};																						\
	kbe_snprintf(dumpname, MAX_BUF, "%" PRAppID, g_componentID);														\
	KBEngine::exception::installCrashHandler(1, dumpname);																\
	int retcode = -1;																									\
	THREAD_TRY_EXECUTION;																								\
	retcode = kbeMain(argc, argv);																						\
	THREAD_HANDLE_CRASH;																								\
	return retcode;																										\
}																														\
int kbeMain
#else
#define KBENGINE_MAIN																									\
kbeMain(int argc, char* argv[]);																						\
int main(int argc, char* argv[])																						\
{																														\
	loadConfig();																										\
	g_componentID = genUUID64();																						\
	parseMainCommandArgs(argc, argv);																					\
	return kbeMain(argc, argv);																							\
}																														\
int kbeMain
#endif
}

#endif // KBE_KBEMAIN_H
