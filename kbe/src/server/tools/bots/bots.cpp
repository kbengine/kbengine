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


#include "bots.hpp"
#include "bots_interface.hpp"
#include "resmgr/resmgr.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/serverconfig.hpp"
#include "../../../server/baseapp/baseapp_interface.hpp"
#include "../../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{
ServerConfig g_serverConfig;

//-------------------------------------------------------------------------------------
Bots::Bots(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
ClientApp(dispatcher, ninterface, componentType, componentID)
{
}

//-------------------------------------------------------------------------------------
Bots::~Bots()
{
}

//-------------------------------------------------------------------------------------
bool Bots::initialize()
{
	if(!Resmgr::getSingleton().initialize())
		return false;

	// "../../res/server/kbengine_defs.xml"
	if(!g_kbeSrvConfig.loadConfig("server/kbengine_defs.xml"))
		return false;

	// "../../../demo/res/server/kbengine.xml"
	if(!g_kbeSrvConfig.loadConfig("server/kbengine.xml"))
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::installEntityDef()
{
	if(!EntityDef::installScript(NULL))
		return false;

	// 初始化数据类别
	// demo/res/scripts/entity_defs/alias.xml
	if(!DataTypes::initialize("scripts/entity_defs/alias.xml"))
		return false;

	// 初始化所有扩展模块
	// demo/res/scripts/
	if(!EntityDef::initialize(Resmgr::getSingleton().respaths()[1] + "res/scripts/", scriptBaseTypes_, BOTS_TYPE)){
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
int Bots::registerPyObjectToScript(const char* attrName, PyObject* pyObj)
{ 
	return script_.registerToModule(attrName, pyObj); 
}

//-------------------------------------------------------------------------------------
int Bots::unregisterPyObjectToScript(const char* attrName)
{ 
	return script_.unregisterToModule(attrName); 
}

//-------------------------------------------------------------------------------------
bool Bots::installPyScript()
{
	if(Resmgr::getSingleton().respaths().size() <= 0)
	{
		ERROR_MSG("EntityApp::installPyScript: KBE_RES_PATH is error!\n");
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
	pyPaths += root_path + L"res/scripts/bots;";
	
	std::string kbe_res_path = Resmgr::getSingleton().respaths()[0].c_str();
	kbe_res_path += "scripts/common";

	tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(kbe_res_path.c_str()));
	bool ret = getScript().install(tbuf, pyPaths, "KBEngine", componentType_);
	// 此处经测试传入python之后被python释放了
	// free(tbuf);
	return ret;
}

//-------------------------------------------------------------------------------------
void Bots::registerScript(PyTypeObject* pto)
{
	scriptBaseTypes_.push_back(pto);
}

//-------------------------------------------------------------------------------------
bool Bots::uninstallPyScript()
{
	return uninstallPyModules() && getScript().uninstall();
}

//-------------------------------------------------------------------------------------
bool Bots::installPyModules()
{
	onInstallPyModules();
	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::uninstallPyModules()
{
	EntityDef::uninstallScript();
	return true;
}

//-------------------------------------------------------------------------------------

}
