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

#ifndef KBE_KBEMAIN_HPP
#define KBE_KBEMAIN_HPP

#include "helper/memory_helper.hpp"

#include "serverapp.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/kbekey.hpp"
#include "cstdkbe/stringconv.hpp"
#include "helper/debug_helper.hpp"
#include "network/event_dispatcher.hpp"
#include "network/message_handler.hpp"
#include "network/network_interface.hpp"
#include "server/componentbridge.hpp"
#include "server/machine_infos.hpp"
#include "resmgr/resmgr.hpp"

#if KBE_PLATFORM == PLATFORM_WIN32
#include "helper/crashhandler.hpp"
#endif

namespace KBEngine{

inline void START_MSG(const char * name, uint64 appuid)
{
	MachineInfos machineInfo;
	
	std::string s = (fmt::format("---- {} "
			"Version: {}. "
			"ScriptVersion: {}. "
			"Protocol: {}. "
			"Config: {}. "
			"Built: {} {}. "
			"AppID: {}. "
			"UID: {}. "
			"PID: {} ----\n",
		name, KBEVersion::versionString(), KBEVersion::scriptVersionString(),
		Mercury::MessageHandlers::getDigestStr(),
		KBE_CONFIG, __TIME__, __DATE__,
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

	// "../../res/server/kbengine_defs.xml"
	g_kbeSrvConfig.loadConfig("server/kbengine_defs.xml");

	// "../../../demo/res/server/kbengine.xml"
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

#if KBE_PLATFORM == PLATFORM_WIN32
		_putenv((std::string("KBE_COMPONENTID=") + scomponentID).c_str());
		_putenv((std::string("KBE_GLOBALID=") + scomponentGlobalOrder).c_str());
		_putenv((std::string("KBE_GROUPID=") + scomponentGroupOrder).c_str());
#else
		setenv("KBE_COMPONENTID", scomponentID.c_str(), 1);
		setenv("KBE_GLOBALID", scomponentGlobalOrder.c_str(), 1);
		setenv("KBE_GROUPID", scomponentGroupOrder.c_str(), 1);
#endif
}

template <class SERVER_APP>
int kbeMainT(int argc, char * argv[], COMPONENT_TYPE componentType, 
			 int32 extlisteningPort_min = -1, int32 extlisteningPort_max = -1, const char * extlisteningInterface = "",
			 int32 intlisteningPort = 0, const char * intlisteningInterface = "")
{
	setEvns();
	startLeakDetection(componentType, g_componentID);

	g_componentType = componentType;
	DebugHelper::initHelper(componentType);
	INFO_MSG( "-----------------------------------------------------------------------------------------\n\n\n");

	KBEKey kbekey(Resmgr::getSingleton().matchPath("key/") + "kbengine_public.key", 
		Resmgr::getSingleton().matchPath("key/") + "kbengine_private.key");

	Resmgr::getSingleton().print();

	Mercury::EventDispatcher dispatcher;
	DebugHelper::getSingleton().pDispatcher(&dispatcher);

	const ChannelCommon& channelCommon = g_kbeSrvConfig.channelCommon();

	Mercury::g_SOMAXCONN = g_kbeSrvConfig.tcp_SOMAXCONN(g_componentType);

	Mercury::NetworkInterface networkInterface(&dispatcher, 
		extlisteningPort_min, extlisteningPort_max, extlisteningInterface, 
		channelCommon.extReadBufferSize, channelCommon.extWriteBufferSize,
		(intlisteningPort != -1) ? htons(intlisteningPort) : -1, intlisteningInterface,
		channelCommon.intReadBufferSize, channelCommon.intWriteBufferSize);
	
	DebugHelper::getSingleton().pNetworkInterface(&networkInterface);

	g_kbeSrvConfig.updateInfos(true, componentType, g_componentID, 
			networkInterface.intaddr(), networkInterface.extaddr());
	
	if(getUserUID() <= 0)
	{
		WARNING_MSG(fmt::format("invalid UID({}) <= 0, please check UID for environment!\n", getUserUID()));
	}

	Componentbridge* pComponentbridge = new Componentbridge(networkInterface, componentType, g_componentID);
	SERVER_APP app(dispatcher, networkInterface, componentType, g_componentID);
	START_MSG(COMPONENT_NAME_EX(componentType), g_componentID);

	if(!app.initialize())
	{
		ERROR_MSG("app::initialize() is error!\n");
		SAFE_RELEASE(pComponentbridge);
		app.finalise();
		return -1;
	}
	
	INFO_MSG(fmt::format("---- {} is running ----\n", COMPONENT_NAME_EX(componentType)));
#if KBE_PLATFORM == PLATFORM_WIN32
	printf("[INFO]: %s", (fmt::format("---- {} is running ----\n", COMPONENT_NAME_EX(componentType))).c_str());

	wchar_t exe_path[MAX_PATH];
	memset(exe_path, 0, MAX_PATH * sizeof(wchar_t));
	GetCurrentDirectory(MAX_PATH, exe_path);
	
	char* ccattr = strutil::wchar2char(exe_path);
	printf("Writing to: %s/logs/%s.*.log\n\n", ccattr, COMPONENT_NAME_EX(componentType));
	free(ccattr);
#endif

	int ret = app.run();

	SAFE_RELEASE(pComponentbridge);
	app.finalise();
	INFO_MSG(fmt::format("{}({}) has shut down.\n", COMPONENT_NAME_EX(componentType), g_componentID));
	return ret;
}

inline void parseMainCommandArgs(int argc, char* argv[])
{
	if(argc < 2)
	{
		return;
	}

	for(int argIdx=1; argIdx<argc; argIdx++)
	{
		std::string cmd = argv[argIdx];
		
		std::string findcmd = "--cid=";
		std::string::size_type fi1 = cmd.find(findcmd);
		if(fi1 != std::string::npos)
		{
			cmd.erase(fi1, findcmd.size());
			if(cmd.size() > 0)
			{
				uint64 cid = 0;
				try
				{
					StringConv::str2value(cid, cmd.c_str());
					g_componentID = cid;
				}
				catch(...)
				{
					ERROR_MSG("parseCommandArgs: --cid=? invalid, no set!\n");
				}
			}

			continue;
		}

		findcmd = "--grouporder=";
		fi1 = cmd.find(findcmd);
		if(fi1 != std::string::npos)
		{
			cmd.erase(fi1, findcmd.size());
			if(cmd.size() > 0)
			{
				int8 orderid = 0;
				try
				{
					StringConv::str2value(orderid, cmd.c_str());
					g_componentGroupOrder = orderid;
				}
				catch(...)
				{
					ERROR_MSG("parseCommandArgs: --grouporder=? invalid, no set!\n");
				}
			}

			continue;
		}

		findcmd = "--globalorder=";
		fi1 = cmd.find(findcmd);
		if(fi1 != std::string::npos)
		{
			cmd.erase(fi1, findcmd.size());
			if(cmd.size() > 0)
			{
				int8 orderid = 0;
				try
				{
					StringConv::str2value(orderid, cmd.c_str());
					g_componentGlobalOrder = orderid;
				}
				catch(...)
				{
					ERROR_MSG("parseCommandArgs: --globalorder=? invalid, no set!\n");
				}
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
	kbe_snprintf(dumpname, MAX_BUF, "%"PRAppID, g_componentID);															\
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

#endif // KBE_KBEMAIN_HPP
