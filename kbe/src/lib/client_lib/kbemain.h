// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBEMAIN_CLIENT_H
#define KBEMAIN_CLIENT_H
#include "clientapp.h"
#include "entity.h"
#include "common/common.h"
#include "helper/debug_helper.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/message_handler.h"
#include "server/machine_infos.h"
#include "server/serverconfig.h"
#include "pyscript/py_gc.h"
#include "resmgr/resmgr.h"
#include "client_lib/config.h"
#include "entitydef/entity_component.h"

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
		Network::MessageHandlers::getDigestStr(),
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

inline bool installPyScript(KBEngine::script::Script& script, COMPONENT_TYPE componentType)
{
	if(Resmgr::getSingleton().respaths().size() <= 0 || 
		Resmgr::getSingleton().getPyUserResPath().size() == 0 || 
		Resmgr::getSingleton().getPySysResPath().size() == 0 ||
		Resmgr::getSingleton().getPyUserScriptsPath().size() == 0)
	{
		ERROR_MSG("EntityApp::installPyScript: KBE_RES_PATH error!\n");
		return false;
	}

	std::wstring user_scripts_path = L"";
	wchar_t* tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(Resmgr::getSingleton().getPyUserScriptsPath().c_str()));
	if(tbuf != NULL)
	{
		user_scripts_path += tbuf;
		free(tbuf);
	}
	else
	{
		ERROR_MSG("EntityApp::installPyScript: KBE_RES_PATH error[char2wchar]!\n");
		return false;
	}

	std::wstring pyPaths = user_scripts_path + L"common;";
	pyPaths += user_scripts_path + L"data;";
	pyPaths += user_scripts_path + L"user_type;";

	if (componentType == CLIENT_TYPE)
	{
		pyPaths += user_scripts_path + L"client;";
		pyPaths += user_scripts_path + L"client/interfaces;";
		pyPaths += user_scripts_path + L"client/components;";
	}
	else
	{
		pyPaths += user_scripts_path + L"bots;";
		pyPaths += user_scripts_path + L"bots/interfaces;";
		pyPaths += user_scripts_path + L"bots/components;";
	}

	std::string kbe_res_path = Resmgr::getSingleton().getPySysResPath();
	kbe_res_path += "scripts/common";

	tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(kbe_res_path.c_str()));
	bool ret = script.install(tbuf, pyPaths, "KBEngine", componentType);
	free(tbuf);

	EntityDef::installScript(script.getModule());
	client::Entity::installScript(script.getModule());
	EntityComponent::installScript(script.getModule());
	Entities<client::Entity>::installScript(NULL);
	EntityGarbages<client::Entity>::installScript(NULL);
	return ret;
}

inline bool uninstallPyScript(KBEngine::script::Script& script)
{
	// script::PyGC::set_debug(script::PyGC::DEBUG_STATS|script::PyGC::DEBUG_LEAK);
	// script::PyGC::collect();
	client::Entity::uninstallScript();
	EntityComponent::uninstallScript();
	Entities<client::Entity>::uninstallScript();
	EntityGarbages<client::Entity>::uninstallScript();
	EntityDef::uninstallScript();

	script::PyGC::debugTracing();
	return script.uninstall();
}

inline bool loadConfig()
{
	Resmgr::getSingleton().initialize();
	
	if(g_componentType == BOTS_TYPE)
	{
		// "../../res/server/kbengine_defaults.xml"
		g_kbeSrvConfig.loadConfig("server/kbengine_defaults.xml");

		// "../../../assets/res/server/kbengine.xml"
		g_kbeSrvConfig.loadConfig("server/kbengine.xml");
	}
	else
	{
		Config::getSingleton().loadConfig("kbengine.xml");
	}

	return true;
}

inline void parseMainCommandArgs(int argc, char* argv[])
{
	if(argc < 2)
	{
		return;
	}

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
				}
				catch(...)
				{
					ERROR_MSG("parseCommandArgs: --cid=? invalid, no set!\n");
				}
			}

			continue;
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
	}
}

inline void setEvns()
{
	std::string scomponentGroupOrder = "0";
	std::string scomponentGlobalOrder = "0";
	std::string scomponentID = "0";

	if(g_componentGroupOrder > 0)
	{
		COMPONENT_ORDER icomponentGroupOrder = g_componentGroupOrder;
		scomponentGroupOrder = KBEngine::StringConv::val2str(icomponentGroupOrder);
	}
	
	if(g_componentGlobalOrder > 0)
	{
		COMPONENT_ORDER icomponentGlobalOrder = g_componentGlobalOrder;
		scomponentGlobalOrder = KBEngine::StringConv::val2str(icomponentGlobalOrder);
	}

	{
		COMPONENT_ID v = g_componentID;
		scomponentID = KBEngine::StringConv::val2str(v);
	}

	setenv("KBE_COMPONENTID", scomponentID.c_str(), 1);
	setenv("KBE_BOOTIDX_GLOBAL", scomponentGlobalOrder.c_str(), 1);
	setenv("KBE_BOOTIDX_GROUP", scomponentGroupOrder.c_str(), 1);
}

template <class CLIENT_APP>
int kbeMainT(int argc, char * argv[], COMPONENT_TYPE componentType, 
	int32 extlisteningTcpPort_min = -1, int32 extlisteningTcpPort_max = -1,
	int32 extlisteningUdpPort_min = -1, int32 extlisteningUdpPort_max = -1, const char * extlisteningInterface = "",
	int32 intlisteningPort_min = 0, int32 intlisteningPort_max = 0, const char * intlisteningInterface = "")
{
	g_componentID = genUUID64();
	g_componentType = componentType;
	parseMainCommandArgs(argc, argv);
	setEvns();

	if(!loadConfig())
	{
		ERROR_MSG("app::initialize error!\n");
		return -1;
	}
	
	DebugHelper::initialize(componentType);
	INFO_MSG( "-----------------------------------------------------------------------------------------\n\n\n");

	Network::EventDispatcher dispatcher;
	DebugHelper::getSingleton().pDispatcher(&dispatcher);

	Network::NetworkInterface networkInterface(&dispatcher, 
		extlisteningTcpPort_min, extlisteningTcpPort_max, extlisteningUdpPort_min, extlisteningUdpPort_max, extlisteningInterface, 0, 0,
		intlisteningPort_min, intlisteningPort_max, intlisteningInterface, 0, 0);
	
	KBEngine::script::Script script;
	if(!installPyScript(script, componentType))
	{
		ERROR_MSG("app::initialize error!\n");
		return -1;
	}

	CLIENT_APP* pApp = new CLIENT_APP(dispatcher, networkInterface, componentType, g_componentID);
	pApp->setScript(&script);

	START_MSG(COMPONENT_NAME_EX(componentType), g_componentID);
	if(!pApp->initialize()){
		ERROR_MSG("app::initialize error!\n");
		pApp->finalise();
		Py_DECREF(pApp);
		uninstallPyScript(script);
		return -1;
	}
	
	INFO_MSG(fmt::format("---- {} is running ----\n", COMPONENT_NAME_EX(componentType)));
	int ret = pApp->run();
	pApp->finalise();
	Py_DECREF(pApp);
	uninstallPyScript(script);
	INFO_MSG(fmt::format("{} has shut down.\n", COMPONENT_NAME_EX(componentType)));
	return ret;
}

#define KBENGINE_MAIN																									\
kbeMain(int argc, char* argv[]);																						\
int main(int argc, char* argv[])																						\
{																														\
	return kbeMain(argc, argv);																							\
}																														\
int kbeMain

}

#endif // KBEMAIN_CLIENT_H
