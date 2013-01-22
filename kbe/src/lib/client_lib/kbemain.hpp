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

#ifndef __KBEMAIN_CLIENT__
#define __KBEMAIN_CLIENT__
#include "clientapp.hpp"
#include "entity.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "server/serverinfos.hpp"
#include "server/serverconfig.hpp"
#include "resmgr/resmgr.hpp"

namespace KBEngine{

inline void START_MSG(const char * name, uint64 appuid)
{
}

inline bool installPyScript(KBEngine::script::Script& script, COMPONENT_TYPE componentType)
{
	if(Resmgr::getSingleton().respaths().size() <= 0)
	{
		ERROR_MSG("installPyScript: KBE_RES_PATH is error!\n");
		return false;
	}

	std::wstring root_path = L"";
	wchar_t* tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(Resmgr::getSingleton().respaths()[1].c_str()));
	if(tbuf != NULL)
	{
		root_path += tbuf;
		free(tbuf);
	}
	else
	{
		return false;
	}

	std::wstring pyPaths = root_path + L"res/scripts/common;";
	pyPaths += root_path + L"res/scripts/data;";
	pyPaths += root_path + L"res/scripts/user_type;";

	if(componentType == CLIENT_TYPE)
		pyPaths += root_path + L"res/scripts/client;";
	else
		pyPaths += root_path + L"res/scripts/bots;";

	std::string kbe_res_path = Resmgr::getSingleton().respaths()[0].c_str();
	kbe_res_path += "scripts/common";

	tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(kbe_res_path.c_str()));
	bool ret = script.install(tbuf, pyPaths, "KBEngine", componentType);
	// 此处经测试传入python之后被python释放了
	// free(tbuf);

	EntityDef::installScript(script.getModule());
	client::Entity::installScript(script.getModule());
	Entities<client::Entity>::installScript(NULL);
	return ret;
}

inline bool uninstallPyScript(KBEngine::script::Script& script)
{
	EntityDef::uninstallScript();

	client::Entity::uninstallScript();
	Entities<client::Entity>::uninstallScript();
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
	return true;
}

template <class CLIENT_APP>
int kbeMainT(int argc, char * argv[], COMPONENT_TYPE componentType, 
			 int32 extlisteningPort_min = -1, int32 extlisteningPort_max = -1, const char * extlisteningInterface = "",
			 int32 intlisteningPort = 0, const char * intlisteningInterface = "")
{
	g_componentID = genUUID64();
	g_componentType = componentType;

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
		uninstallPyScript(script);
		return -1;
	}
	
	INFO_MSG(boost::format("---- %1% is running ----\n") % COMPONENT_NAME_EX(componentType));
	int ret = pApp->run();
	pApp->finalise();
	uninstallPyScript(script);
	INFO_MSG(boost::format("%1% has shut down.\n") % COMPONENT_NAME_EX(componentType));
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

#endif // __KBEMAIN__
