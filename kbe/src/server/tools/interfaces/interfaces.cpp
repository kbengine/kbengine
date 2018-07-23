// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "orders.h"
#include "profile.h"
#include "interfaces.h"
#include "interfaces_tasks.h"
#include "interfaces_interface.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/components.h"
#include "server/telnet_server.h"

#include "baseapp/baseapp_interface.h"
#include "cellapp/cellapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "cellappmgr/cellappmgr_interface.h"
#include "loginapp/loginapp_interface.h"
#include "dbmgr/dbmgr_interface.h"	

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Interfaces);


//-------------------------------------------------------------------------------------
Interfaces::Interfaces(Network::EventDispatcher& dispatcher, 
			 Network::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	PythonApp(dispatcher, ninterface, componentType, componentID),
	mainProcessTimer_(),
	reqCreateAccount_requests_(),
	reqAccountLogin_requests_(),
	pTelnetServer_(NULL)
{
	KBEngine::Network::MessageHandlers::pMainMessageHandlers = &InterfacesInterface::messageHandlers;
}

//-------------------------------------------------------------------------------------
Interfaces::~Interfaces()
{
	mainProcessTimer_.cancel();

	if(reqCreateAccount_requests_.size() > 0)
	{	
		int i = 0;
		REQCREATE_MAP::iterator iter = reqCreateAccount_requests_.begin();
		for(; iter != reqCreateAccount_requests_.end(); ++iter)
		{
			WARNING_MSG(fmt::format("Interfaces::~Interfaces(): Discarding {0}/{1} reqCreateAccount[{2:p}] tasks.\n", 
				++i, reqCreateAccount_requests_.size(), (void*)iter->second));
		}
	}

	if(reqAccountLogin_requests_.size() > 0)
	{
		int i = 0;
		REQLOGIN_MAP::iterator iter = reqAccountLogin_requests_.begin();
		for(; iter != reqAccountLogin_requests_.end(); ++iter)
		{
			WARNING_MSG(fmt::format("Interfaces::~Interfaces(): Discarding {0}/{1} reqAccountLogin[{2:p}] tasks.\n", 
				++i, reqAccountLogin_requests_.size(), (void*)iter->second));
		}
	}
}

//-------------------------------------------------------------------------------------	
void Interfaces::onShutdownBegin()
{
	PythonApp::onShutdownBegin();

	// 通知脚本
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	SCRIPT_OBJECT_CALL_ARGS0(getEntryScript().get(), const_cast<char*>("onInterfaceAppShutDown"), false);
}

//-------------------------------------------------------------------------------------	
void Interfaces::onShutdownEnd()
{
	PythonApp::onShutdownEnd();
}

//-------------------------------------------------------------------------------------
bool Interfaces::run()
{
	return PythonApp::run();
}

//-------------------------------------------------------------------------------------
void Interfaces::handleTimeout(TimerHandle handle, void * arg)
{
	PythonApp::handleTimeout(handle, arg);

	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_TICK:
			this->handleMainTick();
			break;
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------
void Interfaces::handleMainTick()
{
	 //time_t t = ::time(NULL);
	 //DEBUG_MSG("Interfaces::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	threadPool_.onMainThreadTick();
	networkInterface().processChannels(&InterfacesInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool Interfaces::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Interfaces::inInitialize()
{
	PythonApp::inInitialize();
	// 广播自己的地址给网上上的所有kbemachine
	Components::getSingleton().pHandler(this);
	return true;
}

//-------------------------------------------------------------------------------------
bool Interfaces::initializeEnd()
{
	PythonApp::initializeEnd();

	mainProcessTimer_ = this->dispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_TICK));

	// 不做频道超时检查
	CLOSE_CHANNEL_INACTIVITIY_DETECTION();

	if (!initDB())
		return false;

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	// 所有脚本都加载完毕
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onInterfaceAppReady"), 
										const_cast<char*>(""));

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();

	pTelnetServer_ = new TelnetServer(&this->dispatcher(), &this->networkInterface());
	pTelnetServer_->pScript(&this->getScript());

	bool ret = pTelnetServer_->start(g_kbeSrvConfig.getInterfaces().telnet_passwd,
		g_kbeSrvConfig.getInterfaces().telnet_deflayer,
		g_kbeSrvConfig.getInterfaces().telnet_port);

	Components::getSingleton().extraData4(pTelnetServer_->port());
	return ret;
}

//-------------------------------------------------------------------------------------		
void Interfaces::onInstallPyModules()
{
	PyObject * module = getScript().getModule();

	for (int i = 0; i < SERVER_ERR_MAX; i++)
	{
		if(PyModule_AddIntConstant(module, SERVER_ERR_STR[i], i))
		{
			ERROR_MSG( fmt::format("Interfaces::onInstallPyModules: Unable to set KBEngine.{}.\n", SERVER_ERR_STR[i]));
		}
	}

	APPEND_SCRIPT_MODULE_METHOD(module,		chargeResponse,					__py_chargeResponse,									METH_VARARGS,	0);
	APPEND_SCRIPT_MODULE_METHOD(module,		accountLoginResponse,			__py_accountLoginResponse,								METH_VARARGS,	0);
	APPEND_SCRIPT_MODULE_METHOD(module,		createAccountResponse,			__py_createAccountResponse,								METH_VARARGS,	0);
}

//-------------------------------------------------------------------------------------		
bool Interfaces::initDB()
{
	return true;
}

//-------------------------------------------------------------------------------------
void Interfaces::finalise()
{
	if (pTelnetServer_)
	{
		pTelnetServer_->stop();
		SAFE_RELEASE(pTelnetServer_);
	}

	PythonApp::finalise();
}

//-------------------------------------------------------------------------------------
void Interfaces::eraseOrders(std::string ordersid)
{
	ORDERS::iterator iter = orders_.find(ordersid);
	if(iter != orders_.end())
	{
		ERROR_MSG(fmt::format("Interfaces::eraseOrders: chargeID={} not found!\n", ordersid));
	}

	orders_.erase(iter);
}

//-------------------------------------------------------------------------------------
bool Interfaces::hasOrders(std::string ordersid)
{
	bool ret = false;
	
	ORDERS::iterator iter = orders_.find(ordersid);
	ret = (iter != orders_.end());
	
	return ret;
}

//-------------------------------------------------------------------------------------
void Interfaces::reqCreateAccount(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string registerName, accountName, password, datas;
	COMPONENT_ID cid;
	uint8 accountType = 0;

	s >> cid >> registerName >> password >> accountType;
	s.readBlob(datas);
	
	if(accountType == (uint8)ACCOUNT_TYPE_MAIL)
	{
	}

	REQCREATE_MAP::iterator iter = reqCreateAccount_requests_.find(registerName);
	if(iter != reqCreateAccount_requests_.end())
	{
		return;
	}

	CreateAccountTask* pinfo = new CreateAccountTask();
	pinfo->commitName = registerName;
	pinfo->accountName = registerName;
	pinfo->getDatas = "";
	pinfo->password = password;
	pinfo->postDatas = datas;
	pinfo->retcode = SERVER_ERR_OP_FAILED;
	pinfo->baseappID = cid;
	pinfo->dbmgrID = pChannel->componentID();
	pinfo->address = pChannel->addr();
	pinfo->enable = true;

	reqCreateAccount_requests_[pinfo->commitName] = pinfo;

	// 把请求交由脚本处理
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onRequestCreateAccount"), 
										const_cast<char*>("ssy#"), 
										registerName.c_str(), 
										password.c_str(),
										datas.c_str(), datas.length());

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Interfaces::createAccountResponse(std::string commitName, std::string realAccountName, 
	std::string extraDatas, KBEngine::SERVER_ERROR_CODE errorCode)
{
	REQCREATE_MAP::iterator iter = reqCreateAccount_requests_.find(commitName);
	if (iter == reqCreateAccount_requests_.end())
	{
		// 理论上不可能找不到，但如果真找不到，这是个很恐怖的事情，必须写日志记录下来
		ERROR_MSG(fmt::format("Interfaces::createAccountResponse: accountName '{}' not found!" \
			"realAccountName = '{}', extra datas = '{}', error code = '{}'\n", 
			commitName, 
			realAccountName, 
			extraDatas, 
			errorCode));

		return;
	}

	CreateAccountTask *task = iter->second;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();

	(*pBundle).newMessage(DbmgrInterface::onCreateAccountCBFromInterfaces);
	(*pBundle) << task->baseappID << commitName << realAccountName << task->password << errorCode;

	(*pBundle).appendBlob(task->postDatas);
	(*pBundle).appendBlob(extraDatas);

	Network::Channel* pChannel = Interfaces::getSingleton().networkInterface().findChannel(task->address);

	if(pChannel)
	{
		pChannel->send(pBundle);
	}
	else
	{
		ERROR_MSG(fmt::format("Interfaces::createAccountResponse: not found channel. commitName={}\n", commitName));
		Network::Bundle::reclaimPoolObject(pBundle);
	}

	// 清理
	reqCreateAccount_requests_.erase(iter);
	delete task;
}

//-------------------------------------------------------------------------------------
PyObject* Interfaces::__py_createAccountResponse(PyObject* self, PyObject* args)
{
	const char *commitName;
	const char *realAccountName;
    char *extraDatas = NULL;
    Py_ssize_t extraDatas_size = 0;
	KBEngine::SERVER_ERROR_CODE errCode;

	if (!PyArg_ParseTuple(args, "ssy#H", &commitName, &realAccountName, &extraDatas, &extraDatas_size, &errCode))
		return NULL;

	Interfaces::getSingleton().createAccountResponse(std::string(commitName),
		std::string(realAccountName),
		(extraDatas && extraDatas_size > 0) ? std::string(extraDatas, extraDatas_size) : std::string(""),
		errCode);

	SCRIPT_ERROR_CHECK();
	S_Return;
}

//-------------------------------------------------------------------------------------
void Interfaces::onAccountLogin(Network::Channel* pChannel, KBEngine::MemoryStream& s) 
{
	std::string loginName, accountName, password, datas;
	COMPONENT_ID cid;

	s >> cid >> loginName >> password;
	s.readBlob(datas);

	REQLOGIN_MAP::iterator iter = reqAccountLogin_requests_.find(loginName);
	if(iter != reqAccountLogin_requests_.end())
	{
		return;
	}

	LoginAccountTask* pinfo = new LoginAccountTask();
	pinfo->commitName = loginName;
	pinfo->accountName = loginName;
	pinfo->getDatas = "";
	pinfo->password = password;
	pinfo->postDatas = datas;
	pinfo->retcode = SERVER_ERR_OP_FAILED;
	pinfo->baseappID = cid;
	pinfo->dbmgrID = pChannel->componentID();
	pinfo->address = pChannel->addr();
	pinfo->enable = true;

	reqAccountLogin_requests_[pinfo->commitName] = pinfo;

	// 把请求交由脚本处理
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onRequestAccountLogin"), 
										const_cast<char*>("ssy#"), 
										loginName.c_str(), 
										password.c_str(), 
										datas.c_str(), datas.length());

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Interfaces::accountLoginResponse(std::string commitName, std::string realAccountName, 
	std::string extraDatas, KBEngine::SERVER_ERROR_CODE errorCode)
{
	REQLOGIN_MAP::iterator iter = reqAccountLogin_requests_.find(commitName);
	if (iter == reqAccountLogin_requests_.end())
	{
		// 理论上不可能找不到，但如果真找不到，这是个很恐怖的事情，必须写日志记录下来
		ERROR_MSG(fmt::format("Interfaces::accountLoginResponse: commitName '{}' not found!" \
			"realAccountName = '{}', extra datas = '{}', error code = '{}'\n", 
			commitName, 
			realAccountName, 
			extraDatas, 
			errorCode));

		return;
	}

	LoginAccountTask *task = iter->second;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	
	(*pBundle).newMessage(DbmgrInterface::onLoginAccountCBBFromInterfaces);
	(*pBundle) << task->baseappID << commitName << realAccountName << task->password << errorCode;

	(*pBundle).appendBlob(task->postDatas);
	(*pBundle).appendBlob(extraDatas);

	Network::Channel* pChannel = Interfaces::getSingleton().networkInterface().findChannel(task->address);

	if(pChannel)
	{
		pChannel->send(pBundle);
	}
	else
	{
		ERROR_MSG(fmt::format("Interfaces::accountLoginResponse: not found channel. commitName={}\n", commitName));
		Network::Bundle::reclaimPoolObject(pBundle);
	}

	// 清理
	reqAccountLogin_requests_.erase(iter);
	delete task;
}

//-------------------------------------------------------------------------------------
PyObject* Interfaces::__py_accountLoginResponse(PyObject* self, PyObject* args)
{
	const char *commitName;
	const char *realAccountName;
    char *extraDatas = NULL;
    Py_ssize_t extraDatas_size = 0;
	KBEngine::SERVER_ERROR_CODE errCode;

	if (!PyArg_ParseTuple(args, "ssy#H", &commitName, &realAccountName, &extraDatas, &extraDatas_size, &errCode))
		return NULL;

	Interfaces::getSingleton().accountLoginResponse(std::string(commitName),
		std::string(realAccountName),
		(extraDatas && extraDatas_size > 0) ? std::string(extraDatas, extraDatas_size) : std::string(""),
		errCode);

	SCRIPT_ERROR_CHECK();
	S_Return;
}

//-------------------------------------------------------------------------------------
void Interfaces::charge(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	OrdersCharge* pOrdersCharge = new OrdersCharge();

	pOrdersCharge->timeout = timestamp() + uint64(g_kbeSrvConfig.interfaces_orders_timeout_ * stampsPerSecond());

	pOrdersCharge->dbmgrID = pChannel->componentID();
	pOrdersCharge->address = pChannel->addr();

	s >> pOrdersCharge->baseappID;
	s >> pOrdersCharge->ordersID;
	s >> pOrdersCharge->dbid;
	s.readBlob(pOrdersCharge->postDatas);
	s >> pOrdersCharge->cbid;

	INFO_MSG(fmt::format("Interfaces::charge: componentID={4}, chargeID={0}, dbid={1}, cbid={2}, datas={3}!\n",
		pOrdersCharge->ordersID, pOrdersCharge->dbid, pOrdersCharge->cbid, pOrdersCharge->postDatas, pOrdersCharge->baseappID));

	ORDERS::iterator iter = orders_.find(pOrdersCharge->ordersID);
	if(iter != orders_.end())
	{
		ERROR_MSG(fmt::format("Interfaces::charge: chargeID={} is exist!\n", pOrdersCharge->ordersID));
		delete pOrdersCharge;
		return;
	}

	ChargeTask* pinfo = new ChargeTask();
	pinfo->orders = *pOrdersCharge;
	pinfo->pOrders = pOrdersCharge;
	orders_[pOrdersCharge->ordersID].reset(pOrdersCharge);
	
	// 把请求交由脚本处理
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onRequestCharge"), 
										const_cast<char*>("sKy#"), 
										pOrdersCharge->ordersID.c_str(),
										pOrdersCharge->dbid, 
										pOrdersCharge->postDatas.c_str(), pOrdersCharge->postDatas.length());

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Interfaces::chargeResponse(std::string orderID, std::string extraDatas, KBEngine::SERVER_ERROR_CODE errorCode)
{
	ORDERS::iterator iter = orders_.find(orderID);
	if (iter == orders_.end())
	{
		ERROR_MSG(fmt::format("Interfaces::chargeResponse: order id '{}' not found! extra datas = '{}', error code = '{}'\n", 
			orderID, 
			extraDatas, 
			errorCode));
		
		// 这种情况也需要baseapp处理onLoseChargeCB
		// 例如某些时候客户端出问题未向服务器注册这个订单号，但是计费平台有返回的情况
		// 将订单发送给注册的所有的dbmgr
		const Network::NetworkInterface::ChannelMap& channels = Interfaces::getSingleton().networkInterface().channels();
		if(channels.size() > 0)
		{
			Network::NetworkInterface::ChannelMap::const_iterator channeliter = channels.begin();
			for(; channeliter != channels.end(); ++channeliter)
			{
				Network::Channel* pChannel = channeliter->second;
				if(pChannel)
				{
					COMPONENT_ID baseappID = 0;
					DBID dbid = 0;
					CALLBACK_ID cbid = 0;

					Network::Bundle* pBundle = Network::Bundle::createPoolObject();

					(*pBundle).newMessage(DbmgrInterface::onChargeCB);
					(*pBundle) << baseappID << orderID << dbid;
					(*pBundle).appendBlob(extraDatas);
					(*pBundle) << cbid;
					(*pBundle) << errorCode;
					pChannel->send(pBundle);
				}
			}
		}
		else
		{
			ERROR_MSG(fmt::format("Interfaces::chargeResponse: not found channels. orders={}, datas={}\n", 
				orderID, extraDatas));
		}

		return;
	}

	KBEShared_ptr<Orders> orders = iter->second;
	orders->getDatas = extraDatas;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();

	(*pBundle).newMessage(DbmgrInterface::onChargeCB);
	(*pBundle) << orders->baseappID << orders->ordersID << orders->dbid;
	(*pBundle).appendBlob(orders->getDatas);
	(*pBundle) << orders->cbid;
	(*pBundle) << errorCode;

	Network::Channel* pChannel = networkInterface().findChannel(orders->address);

	if(pChannel)
	{
		pChannel->send(pBundle);
	}
	else
	{
		ERROR_MSG(fmt::format("Interfaces::chargeResponse: not found channels. orders={}, datas={}\n", 
			orderID, extraDatas));

		Network::Bundle::reclaimPoolObject(pBundle);
	}

	orders_.erase(iter);
}

//-------------------------------------------------------------------------------------
PyObject* Interfaces::__py_chargeResponse(PyObject* self, PyObject* args)
{
	const char *orderID;
    char *extraDatas = NULL;
    Py_ssize_t extraDatas_size = 0;
	KBEngine::SERVER_ERROR_CODE errCode;

	if (!PyArg_ParseTuple(args, "sy#H", &orderID, &extraDatas, &extraDatas_size, &errCode))
		return NULL;

	if (errCode < SERVER_ERR_MAX)
	{
		Interfaces::getSingleton().chargeResponse(std::string(orderID),
			(extraDatas && extraDatas_size > 0) ? std::string(extraDatas, extraDatas_size) : std::string(""),
			errCode);
	}
	else
	{
		//ERROR_MSG(fmt::format());
	}

	SCRIPT_ERROR_CHECK();
	S_Return;
}

//-------------------------------------------------------------------------------------
void Interfaces::eraseClientReq(Network::Channel* pChannel, std::string& logkey)
{
	REQCREATE_MAP::iterator citer = reqCreateAccount_requests_.find(logkey);
	if(citer != reqCreateAccount_requests_.end())
	{
		citer->second->enable = false;
		DEBUG_MSG(fmt::format("Interfaces::eraseClientReq: reqCreateAccount_logkey={} set disabled!\n", logkey));
	}

	REQLOGIN_MAP::iterator liter = reqAccountLogin_requests_.find(logkey);
	if(liter != reqAccountLogin_requests_.end())
	{
		liter->second->enable = false;
		DEBUG_MSG(fmt::format("Interfaces::eraseClientReq: reqAccountLogin_logkey={} set disabled!\n", logkey));
	}
}

//-------------------------------------------------------------------------------------
}
