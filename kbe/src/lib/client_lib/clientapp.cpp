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


#include "clientapp.hpp"
#include "event.hpp"
#include "entity.hpp"
#include "config.hpp"
#include "network/channel.hpp"
#include "network/tcp_packet_receiver.hpp"
#include "thread/threadpool.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "entitydef/entitydef.hpp"
#include "server/componentbridge.hpp"
#include "server/serverconfig.hpp"
#include "helper/profile.hpp"
#include "client_lib/client_interface.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{

COMPONENT_TYPE g_componentType = UNKNOWN_COMPONENT_TYPE;
COMPONENT_ID g_componentID = 0;
COMPONENT_ORDER g_componentOrder = 1;
GAME_TIME g_kbetime = 0;
Mercury::Address lastAddr = Mercury::Address::NONE;

KBE_SINGLETON_INIT(ClientApp);
//-------------------------------------------------------------------------------------
ClientApp::ClientApp(Mercury::EventDispatcher& dispatcher, 
					 Mercury::NetworkInterface& ninterface, 
					 COMPONENT_TYPE componentType,
					 COMPONENT_ID componentID):
ClientObjectBase(ninterface),
TimerHandler(),
Mercury::ChannelTimeOutHandler(),
scriptBaseTypes_(),
gameTimer_(),
componentType_(componentType),
componentID_(componentID),
mainDispatcher_(dispatcher),
networkInterface_(ninterface),
pTCPPacketReceiver_(NULL),
time_(),
timers_(),
threadPool_(),
entryScript_()
{
	networkInterface_.pExtensionData(this);
	networkInterface_.pChannelTimeOutHandler(this);
	networkInterface_.pChannelDeregisterHandler(this);

	// 初始化mailbox模块获取channel函数地址
	EntityMailbox::setFindChannelFunc(std::tr1::bind(&ClientApp::findChannelByMailbox, this, 
		std::tr1::placeholders::_1));

	KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &ClientInterface::messageHandlers;

	Components::getSingleton().pNetworkInterface(&ninterface);
}

//-------------------------------------------------------------------------------------
ClientApp::~ClientApp()
{
}

//-------------------------------------------------------------------------------------
int ClientApp::registerPyObjectToScript(const char* attrName, PyObject* pyObj)
{ 
	return getScript().registerToModule(attrName, pyObj); 
}

//-------------------------------------------------------------------------------------
int ClientApp::unregisterPyObjectToScript(const char* attrName)
{ 
	return getScript().unregisterToModule(attrName); 
}

//-------------------------------------------------------------------------------------	
bool ClientApp::initializeBegin()
{
	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));

	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeConfig.gameUpdateHertz());
	return true;
}

//-------------------------------------------------------------------------------------	
bool ClientApp::initializeEnd()
{
	// 所有脚本都加载完毕
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onInit"), 
										const_cast<char*>("i"), 
										0);

	if(pyResult != NULL)
	{
		Py_DECREF(pyResult);
	}
	else
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------		
bool ClientApp::initialize()
{
	if(!threadPool_.isInitialize())
		threadPool_.createThreadPool(1, 1, 4);
	
	if(!initializeBegin())
		return false;

	if(!installPyModules())
		return false;
	
	if(!installEntityDef())
		return false;

	if(!inInitialize())
		return false;

	return initializeEnd();
}


//-------------------------------------------------------------------------------------
bool ClientApp::installEntityDef()
{
	if(!EntityDef::installScript(getScript().getModule()))
		return false;

	// 初始化数据类别
	// demo/res/scripts/entity_defs/alias.xml
	if(!DataTypes::initialize("scripts/entity_defs/alias.xml"))
		return false;

	// 初始化所有扩展模块
	// demo/res/scripts/
	if(!EntityDef::initialize(Resmgr::getSingleton().respaths()[1] + "res/scripts/", scriptBaseTypes_, g_componentType)){
		return false;
	}

	// 注册创建entity的方法到py
	// 向脚本注册app发布状态
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	publish,	__py_getAppPublish,		METH_VARARGS,	0)
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	fireEvent,	__py_fireEvent,			METH_VARARGS,	0)
	return true;
}

//-------------------------------------------------------------------------------------
void ClientApp::registerScript(PyTypeObject* pto)
{
	scriptBaseTypes_.push_back(pto);
}

//-------------------------------------------------------------------------------------
bool ClientApp::uninstallPyScript()
{
	return uninstallPyModules();
}

//-------------------------------------------------------------------------------------
bool ClientApp::installPyModules()
{
	registerScript(client::Entity::getScriptType());
	onInstallPyModules();

	// 安装入口模块
	PyObject *entryScriptFileName = PyUnicode_FromString(g_kbeConfig.entryScriptFile());
	if(entryScriptFileName != NULL)
	{
		entryScript_ = PyImport_Import(entryScriptFileName);
		SCRIPT_ERROR_CHECK();
		S_RELEASE(entryScriptFileName);

		if(entryScript_.get() == NULL)
		{
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientApp::uninstallPyModules()
{
	return true;
}

//-------------------------------------------------------------------------------------		
void ClientApp::finalise(void)
{
	if(pServerChannel_ && pServerChannel_->endpoint())
		getNetworkInterface().deregisterChannel(pServerChannel_);

	SAFE_RELEASE(pTCPPacketReceiver_);

	gameTimer_.cancel();
	threadPool_.finalise();
	ClientObjectBase::finalise();

	uninstallPyModules();
	uninstallPyScript();
}

//-------------------------------------------------------------------------------------		
double ClientApp::gameTimeInSeconds() const
{
	return double(g_kbetime) / 10;
}

//-------------------------------------------------------------------------------------
void ClientApp::handleTimeout(TimerHandle, void * arg)
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
}

//-------------------------------------------------------------------------------------
void ClientApp::handleGameTick()
{
	g_kbetime++;
	threadPool_.onMainThreadTick();
	handleTimers();
	
	if(lastAddr.ip != 0)
	{
		getNetworkInterface().deregisterChannel(lastAddr);
		getNetworkInterface().registerChannel(pServerChannel_);
		lastAddr.ip = 0;
	}

	getNetworkInterface().handleChannels(KBEngine::Mercury::MessageHandlers::pMainMessageHandlers);
	tickSend();
}

//-------------------------------------------------------------------------------------
void ClientApp::handleTimers()
{
	timers().process(g_kbetime);
}

//-------------------------------------------------------------------------------------		
bool ClientApp::run(void)
{
	mainDispatcher_.processUntilBreak();
	return true;
}

//-------------------------------------------------------------------------------------		
int ClientApp::processOnce(bool shouldIdle)
{
	return mainDispatcher_.processOnce(shouldIdle);
}

//-------------------------------------------------------------------------------------		
PyObject* ClientApp::__py_getAppPublish(PyObject* self, PyObject* args)
{
	return PyLong_FromLong(g_appPublish);
}

//-------------------------------------------------------------------------------------		
PyObject* ClientApp::__py_fireEvent(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) < 1)
	{
		PyErr_Format(PyExc_AssertionError, "ClientApp::fireEvent: arg1(eventName) not found!\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyname = PyTuple_GetItem(args, 0);

	if(!pyname || !PyUnicode_Check(pyname))
	{
		PyErr_Format(PyExc_AssertionError, "ClientApp::fireEvent: arg1(eventName) not found!\n");
		PyErr_PrintEx(0);
		return NULL;
	}

	wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyname, NULL);
	char* name = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
	PyMem_Free(PyUnicode_AsWideCharStringRet0);

	EventData_Script eventdata;
	eventdata.name = name;
	eventdata.argsSize = PyTuple_Size(args) - 1;
	free(name);

	if(eventdata.argsSize > 0)
	{
		eventdata.pyDatas = PyTuple_New(eventdata.argsSize + 1);
		for(uint32 i=0; i<eventdata.argsSize; i++)
		{
			PyObject* pyitem = PyTuple_GetItem(args, i + 1);
			PyTuple_SetItem(eventdata.pyDatas, i, pyitem);
			Py_INCREF(pyitem);
		}
	}

	ClientApp::getSingleton().fireEvent(&eventdata);
	S_Return;
}

//-------------------------------------------------------------------------------------	
void ClientApp::shutDown()
{
	INFO_MSG( "ClientApp::shutDown: shutting down\n" );
	mainDispatcher_.breakProcessing();
}

//-------------------------------------------------------------------------------------	
void ClientApp::onChannelDeregister(Mercury::Channel * pChannel)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onChannelTimeOut(Mercury::Channel * pChannel)
{
	INFO_MSG(boost::format("ClientApp::onChannelTimeOut: "
		"Channel %1% timed out.\n") % pChannel->c_str());

	networkInterface_.deregisterChannel(pChannel);
	pChannel->destroy();
}

//-------------------------------------------------------------------------------------	
bool ClientApp::login(std::string accountName, std::string passwd, 
								   std::string ip, KBEngine::uint32 port)
{
	connectedGateway_ = false;

	bool exist = false;

	if(pServerChannel_->endpoint())
	{
		lastAddr = pServerChannel_->endpoint()->addr();
		getNetworkInterface().dispatcher().deregisterFileDescriptor(*pServerChannel_->endpoint());
		exist = getNetworkInterface().findChannel(pServerChannel_->endpoint()->addr()) != NULL;
	}

	bool ret = initLoginappChannel(accountName, passwd, ip, port) != NULL;
	if(ret)
	{
		if(!exist)
		{
			getNetworkInterface().registerChannel(pServerChannel_);
			pTCPPacketReceiver_ = new Mercury::TCPPacketReceiver(*pServerChannel_->endpoint(), getNetworkInterface());
		}
		else
		{
			pTCPPacketReceiver_->endpoint(pServerChannel_->endpoint());
		}

		getNetworkInterface().dispatcher().registerFileDescriptor(*pServerChannel_->endpoint(), pTCPPacketReceiver_);
		
		ret = ClientObjectBase::login();
	}

	return ret;
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s)
{
	ClientObjectBase::onLoginSuccessfully(pChannel, s);
	Config::getSingleton().writeAccountName(name_.c_str());

	bool exist = false;

	if(pServerChannel_->endpoint())
	{
		lastAddr = pServerChannel_->endpoint()->addr();
		getNetworkInterface().dispatcher().deregisterFileDescriptor(*pServerChannel_->endpoint());
		exist = getNetworkInterface().findChannel(pServerChannel_->endpoint()->addr()) != NULL;
	}

	bool ret = initBaseappChannel() != NULL;
	if(ret)
	{
		if(!exist)
		{
			getNetworkInterface().registerChannel(pServerChannel_);
			pTCPPacketReceiver_ = new Mercury::TCPPacketReceiver(*pServerChannel_->endpoint(), getNetworkInterface());
		}
		else
		{
			pTCPPacketReceiver_->endpoint(pServerChannel_->endpoint());
		}

		getNetworkInterface().dispatcher().registerFileDescriptor(*pServerChannel_->endpoint(), pTCPPacketReceiver_);
		
		ret = ClientObjectBase::loginGateWay();
	}
}

//-------------------------------------------------------------------------------------		
}
