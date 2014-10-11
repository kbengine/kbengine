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

#ifndef KBEMAIN_CLIENT_HPP
#define KBEMAIN_CLIENT_HPP
#include "clientapp.hpp"
#include "entity.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/message_handler.hpp"
#include "server/machine_infos.hpp"
#include "server/serverconfig.hpp"
#include "resmgr/resmgr.hpp"
#include "client_lib/config.hpp"

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

inline bool installPyScript(KBEngine::script::Script& script, COMPONENT_TYPE componentType)
{
	if(Resmgr::getSingleton().respaths().size() <= 0 || 
		Resmgr::getSingleton().getPyUserResPath().size() == 0 || 
		Resmgr::getSingleton().getPySysResPath().size() == 0)
	{
		ERROR_MSG("EntityApp::installPyScript: KBE_RES_PATH is error!\n");
		return false;
	}

	std::wstring user_res_path = L"";
	wchar_t* tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(Resmgr::getSingleton().getPyUserResPath().c_str()));
	if(tbuf != NULL)
	{
		user_res_path += tbuf;
		free(tbuf);
	}
	else
	{
		ERROR_MSG("EntityApp::installPyScript: KBE_RES_PATH error[char2wchar]!\n");
		return false;
	}

	std::wstring pyPaths = user_res_path + L"scripts/common;";
	pyPaths += user_res_path + L"scripts/data;";
	pyPaths += user_res_path + L"scripts/user_type;";

	if(componentType == CLIENT_TYPE)
		pyPaths += user_res_path + L"scripts/client;";
	else
		pyPaths += user_res_path + L"scripts/bots;";

	std::string kbe_res_path = Resmgr::getSingleton().getPySysResPath();
	kbe_res_path += "scripts/common";

	tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(kbe_res_path.c_str()));
	bool ret = script.install(tbuf, pyPaths, "KBEngine", componentType);
	free(tbuf);

	EntityDef::installScript(script.getModule());
	client::Entity::installScript(script.getModule());
	Entities<client::Entity>::installScript(NULL);
	return ret;
}

inline bool uninstallPyScript(KBEngine::script::Script& script)
{
	client::Entity::uninstallScript();
	Entities<client::Entity>::uninstallScript();
	EntityDef::uninstallScript();
	return script.uninstall();
}

inline bool loadConfig()
{
	Resmgr::getSingleton().initialize();
	
	if(g_componentType == BOTS_TYPE)
	{
		// "../../res/server/kbengine_defs.xml"
		g_kbeSrvConfig.loadConfig("server/kbengine_defs.xml");

		// "../../../demo/res/server/kbengine.xml"
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

template <class CLIENT_APP>
int kbeMainT(int argc, char * argv[], COMPONENT_TYPE componentType, 
			 int32 extlisteningPort_min = -1, int32 extlisteningPort_max = -1, const char * extlisteningInterface = "",
			 int32 intlisteningPort = 0, const char * intlisteningInterface = "")
{
	g_componentID = genUUID64();
	g_componentType = componentType;
	parseMainCommandArgs(argc, argv);
	setEvns();

	if(!loadConfig())
	{
		ERROR_MSG("app::initialize is error!\n");
		return -1;
	}
	
	DebugHelper::initHelper(componentType);
	INFO_MSG( "-----------------------------------------------------------------------------------------\n\n\n");

	Mercury::EventDispatcher dispatcher;
	Mercury::NetworkInterface networkInterface(&dispatcher, 
		extlisteningPort_min, extlisteningPort_max, extlisteningInterface, 0, 0,
		(intlisteningPort != -1) ? htons(intlisteningPort) : -1, intlisteningInterface, 0, 0);
	
	KBEngine::script::Script script;
	if(!installPyScript(script, componentType))
	{
		ERROR_MSG("app::initialize is error!\n");
		return -1;
	}

	CLIENT_APP* pApp = new CLIENT_APP(dispatcher, networkInterface, componentType, g_componentID);
	pApp->setScript(&script);

	START_MSG(COMPONENT_NAME_EX(componentType), g_componentID);
	if(!pApp->initialize()){
		ERROR_MSG("app::initialize is error!\n");
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

#endif // KBEMAIN_CLIENT_HPP
