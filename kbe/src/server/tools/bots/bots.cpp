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
#include "entity.hpp"
#include "client.hpp"
#include "bots_interface.hpp"
#include "resmgr/resmgr.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/serverconfig.hpp"
#include "helper/console_helper.hpp"

#include "../../../server/baseapp/baseapp_interface.hpp"
#include "../../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{
ServerConfig g_serverConfig;
Componentbridge* g_pComponentbridge = NULL;

//-------------------------------------------------------------------------------------
Bots::Bots(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
ClientApp(dispatcher, ninterface, componentType, componentID),
script_(),
scriptBaseTypes_(),
gameTimer_(),
clients_(),
reqCreateAndLoginTotalCount_(g_serverConfig.getBots().defaultAddBots_totalCount),
reqCreateAndLoginTickCount_(g_serverConfig.getBots().defaultAddBots_tickCount),
reqCreateAndLoginTickTime_(g_serverConfig.getBots().defaultAddBots_tickTime),
pCreateAndLoginHandler_(NULL)
{
	KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &BotsInterface::messageHandlers;
	g_pComponentbridge = new Componentbridge(ninterface, componentType, componentID);

	FD_ZERO( &frds );
}

//-------------------------------------------------------------------------------------
Bots::~Bots()
{
	SAFE_RELEASE(g_pComponentbridge);
}

//-------------------------------------------------------------------------------------
bool Bots::initialize()
{
	// 广播自己的地址给网上上的所有kbemachine
	this->getMainDispatcher().addFrequentTask(&Componentbridge::getSingleton());

	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));

	if(!installPyScript())
		return false;

	if(!installPyModules())
		return false;
	
	return installEntityDef();
}

//-------------------------------------------------------------------------------------
void Bots::finalise()
{
	gameTimer_.cancel();
	
	reqCreateAndLoginTotalCount_ = 0;
	SAFE_RELEASE(pCreateAndLoginHandler_);
	ClientApp::finalise();
}

//-------------------------------------------------------------------------------------
bool Bots::installEntityDef()
{
	if(!EntityDef::installScript(getScript().getModule()))
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
	Entity::installScript(getScript().getModule());
	registerScript(Entity::getScriptType());

	onInstallPyModules();
	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::uninstallPyModules()
{
	Entity::uninstallScript();
	EntityDef::uninstallScript();
	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::run(void)
{
	pCreateAndLoginHandler_ = new CreateAndLoginHandler();
	return ClientApp::run();
}

//-------------------------------------------------------------------------------------
void Bots::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
		{
			handleGameTick();
		}
		default:
			break;
	}

	ClientApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
int Bots::maxFD()
{
	int maxfd = -1;

	CLIENTS::iterator iter = clients().begin();
	for(;iter != clients().end(); iter++)
	{
		Client* pClient = iter->second.get();
		if((int)(*pClient->pChannel()->endpoint()) > maxfd)
			maxfd = (int)(*pClient->pChannel()->endpoint());
	}

	return maxfd;
}

//-------------------------------------------------------------------------------------
void Bots::handleGameTick()
{
	// time_t t = ::time(NULL);
	// DEBUG_MSG("EntityApp::handleGameTick[%"PRTime"]:%u\n", t, time_);

	g_kbetime++;
	threadPool_.onMainThreadTick();
	handleTimers();

	CLIENTS::iterator iter = clients().begin();
	for(;iter != clients().end(); iter++)
	{
		iter->second.get()->gameTick();
	}

	getNetworkInterface().handleChannels(KBEngine::Mercury::MessageHandlers::pMainMessageHandlers);

	int maxfd = maxFD() + 1;
	while(true)
	{
		struct timeval tv = { 0, 100000 }; // 100ms
		
		int selgot = select(maxfd, &frds, NULL, NULL, &tv);
		if(selgot == 0)
		{
			break;
		}
		else if(selgot == -1)
		{
			break;
		}
		else
		{
			CLIENTS::iterator iter = clients().begin();
			for(;iter != clients().end(); iter++)
			{
				Client* pClient = iter->second.get();
				
				while(1)
				{
					if(FD_ISSET(*pClient->pChannel()->endpoint(), &frds))
					{
						Mercury::TCPPacket* packet = new Mercury::TCPPacket();
						packet->resize(65535);
						packet->recvFromEndPoint(*pClient->pChannel()->endpoint(), NULL);
						pClient->pChannel()->addReceiveWindow(packet);
					}
				}

				pClient->pChannel()->handleMessage(NULL);
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Bots::lookApp(Mercury::Channel* pChannel)
{
	DEBUG_MSG(boost::format("Bots::lookApp: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	(*pBundle) << g_componentType;
	(*pBundle) << componentID_;
	(*pBundle).send(getNetworkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Bots::reqCloseServer(Mercury::Channel* pChannel, MemoryStream& s)
{
	DEBUG_MSG(boost::format("Bots::reqCloseServer: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	bool success = true;
	(*pBundle) << success;
	(*pBundle).send(getNetworkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	this->getMainDispatcher().breakProcessing();
}

//-------------------------------------------------------------------------------------
void Bots::onExecScriptCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string cmd;
	s.readBlob(cmd);

	PyObject* pycmd = PyUnicode_DecodeUTF8(cmd.data(), cmd.size(), NULL);
	if(pycmd == NULL)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}

	DEBUG_MSG(boost::format("EntityApp::onExecScriptCommand: size(%1%), command=%2%.\n") % 
		cmd.size() % cmd);

	std::string retbuf = "";
	PyObject* pycmd1 = PyUnicode_AsEncodedString(pycmd, "utf-8", NULL);

	if(script_.run_simpleString(PyBytes_AsString(pycmd1), &retbuf) == 0)
	{
		// 将结果返回给客户端
		Mercury::Bundle bundle;
		ConsoleInterface::ConsoleExecCommandCBMessageHandler msgHandler;
		bundle.newMessage(msgHandler);
		ConsoleInterface::ConsoleExecCommandCBMessageHandlerArgs1::staticAddToBundle(bundle, retbuf);
		bundle.send(this->getNetworkInterface(), pChannel);
	}

	Py_DECREF(pycmd);
	Py_DECREF(pycmd1);
}

//-------------------------------------------------------------------------------------
bool Bots::addClient(Client* pClient)
{
	clients().insert(std::make_pair< int, KBEShared_ptr<Client> >(*pClient->pChannel()->endpoint(), 
		KBEShared_ptr<Client>(pClient)));

	return true;
}

//-------------------------------------------------------------------------------------
Client* Bots::findClient(int fd)
{
	CLIENTS::iterator iter = clients().find(fd);
	if(iter != clients().end())
	{
		return iter->second.get();
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
void Bots::onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE retcode;
	std::string retdatas = "";

	s >> retcode;
	s.readBlob(retdatas);

	//Client* pClient = findClient(pChannel);
	//if(pClient)
	//{
	//	pClient->onCreateAccountResult(retcode, retdatas);
	//}
}

//-------------------------------------------------------------------------------------

}
