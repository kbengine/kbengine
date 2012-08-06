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


#include "loginapp.hpp"
#include "loginapp_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "client_lib/client_interface.hpp"

#include "baseapp/baseapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "dbmgr/dbmgr_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Loginapp);

//-------------------------------------------------------------------------------------
Loginapp::Loginapp(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
	loopCheckTimerHandle_(),
	pendingLoginMgr_(ninterface)
{
}

//-------------------------------------------------------------------------------------
Loginapp::~Loginapp()
{
}

//-------------------------------------------------------------------------------------
bool Loginapp::run()
{
	bool ret = true;

	while(!this->getMainDispatcher().isBreakProcessing())
	{
		this->getMainDispatcher().processOnce(false);
		getNetworkInterface().handleChannels(&LoginappInterface::messageHandlers);
		KBEngine::sleep(100);
	};

	return ret;
}

//-------------------------------------------------------------------------------------
void Loginapp::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_CHECK_STATUS:
			this->handleCheckStatusTick();
			return;
		default:
			break;
	}

	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Loginapp::handleCheckStatusTick()
{
	pendingLoginMgr_.process();
}

//-------------------------------------------------------------------------------------
bool Loginapp::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Loginapp::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Loginapp::initializeEnd()
{
	// 添加一个timer， 每秒检查一些状态
	loopCheckTimerHandle_ = this->getMainDispatcher().addTimer(1000000, this,
							reinterpret_cast<void *>(TIMEOUT_CHECK_STATUS));

	return true;
}

//-------------------------------------------------------------------------------------
void Loginapp::finalise()
{
	loopCheckTimerHandle_.cancel();
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void Loginapp::onDbmgrInitCompleted(Mercury::Channel* pChannel, int32 startGlobalOrder, int32 startGroupOrder)
{
	if(pChannel->isExternal())
		return;

	INFO_MSG("Loginapp::onDbmgrInitCompleted:startGlobalOrder=%d, startGroupOrder=%d.\n",
		startGlobalOrder, startGroupOrder);

	startGlobalOrder_ = startGlobalOrder;
	startGroupOrder_ = startGroupOrder;
}

//-------------------------------------------------------------------------------------
void Loginapp::reqCreateAccount(Mercury::Channel* pChannel, std::string& accountName, 
							 std::string& password)
{
	DEBUG_MSG("Loginapp::reqCreateAccount: accountName=%s, password=%s.\n", 
		accountName.c_str(), password.c_str());

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.find(accountName);
	if(ptinfos != NULL)
	{
		Mercury::Bundle bundle;
		bundle.newMessage(ClientInterface::onCreateAccountResult);
		ClientInterface::onCreateAccountResultArgs1::staticAddToBundle(bundle, MERCURY_ERR_BUSY);
		bundle.send(this->getNetworkInterface(), pChannel);
		return;
	}
			
	ptinfos = new PendingLoginMgr::PLInfos;
	ptinfos->accountName = accountName;
	ptinfos->password = password;
	ptinfos->addr = pChannel->addr();
	pendingLoginMgr_.add(ptinfos);

	Components::COMPONENTS cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		Mercury::Bundle bundle;
		bundle.newMessage(ClientInterface::onCreateAccountResult);
		ClientInterface::onCreateAccountResultArgs1::staticAddToBundle(bundle, MERCURY_ERR_SRV_NO_READY);
		bundle.send(this->getNetworkInterface(), pChannel);
		return;
	}

	Mercury::Bundle bundle;
	bundle.newMessage(DbmgrInterface::reqCreateAccount);
	DbmgrInterface::reqCreateAccountArgs2::staticAddToBundle(bundle, accountName, password);
	bundle.send(this->getNetworkInterface(), dbmgrinfos->pChannel);
}

//-------------------------------------------------------------------------------------
void Loginapp::onReqCreateAccountResult(Mercury::Channel* pChannel, MERCURY_ERROR_CODE failedcode, std::string& accountName, 
							 std::string& password)
{
	DEBUG_MSG("Loginapp::onReqCreateAccountResult: accountName=%s, failedcode=%u.\n", 
		accountName.c_str(), failedcode);

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.remove(accountName);
	if(ptinfos == NULL)
		return;

	Mercury::Channel* pClientChannel = this->getNetworkInterface().findChannel(ptinfos->addr);
	if(pClientChannel == NULL)
		return;

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onCreateAccountResult);
	ClientInterface::onCreateAccountResultArgs1::staticAddToBundle(bundle, failedcode);
	bundle.send(this->getNetworkInterface(), pClientChannel);

	SAFE_RELEASE(ptinfos);
}

//-------------------------------------------------------------------------------------
void Loginapp::login(Mercury::Channel* pChannel, MemoryStream& s)
{
	COMPONENT_CLIENT_TYPE ctype;
	int8 tctype = 0;
	std::string accountName;
	std::string password;
	std::string datas;

	// 前端类别
	s >> tctype;
	ctype = static_cast<COMPONENT_CLIENT_TYPE>(tctype);
	
	// 附带数据
	s >> datas;

	// 帐号名
	s >> accountName;

	// 密码
	s >> password;
	
	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.find(accountName);
	if(ptinfos != NULL)
	{
		_loginFailed(pChannel, accountName, MERCURY_ERR_BUSY);
		return;
	}

	ptinfos = new PendingLoginMgr::PLInfos;
	ptinfos->ctype = ctype;
	ptinfos->datas = datas;
	ptinfos->accountName = accountName;
	ptinfos->password = password;
	ptinfos->addr = pChannel->addr();
	pendingLoginMgr_.add(ptinfos);

	INFO_MSG("Loginapp::login: new client[%s], accountName=%s, datas=%s.\n", 
		COMPONENT_CLIENT_NAME[ctype], accountName.c_str(), datas.c_str());

	// 首先必须baseappmgr和dbmgr都已经准备完毕了。
	Components::COMPONENTS cts = Components::getSingleton().getComponents(BASEAPPMGR_TYPE);
	Components::ComponentInfos* baseappmgrinfos = NULL;
	if(cts.size() > 0)
		baseappmgrinfos = &(*cts.begin());

	if(baseappmgrinfos == NULL || baseappmgrinfos->pChannel == NULL || baseappmgrinfos->cid == 0)
	{
		_loginFailed(pChannel, accountName, MERCURY_ERR_SRV_NO_READY);
		return;
	}

	cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		_loginFailed(pChannel, accountName, MERCURY_ERR_SRV_NO_READY);
		return;
	}

	// 向dbmgr查询用户合法性
	Mercury::Bundle bundle;
	bundle.newMessage(DbmgrInterface::onAccountLogin);
	DbmgrInterface::onAccountLoginArgs2::staticAddToBundle(bundle, accountName, password);
	bundle.send(this->getNetworkInterface(), dbmgrinfos->pChannel);
}

//-------------------------------------------------------------------------------------
void Loginapp::_loginFailed(Mercury::Channel* pChannel, std::string& accountName, MERCURY_ERROR_CODE failedcode)
{
	DEBUG_MSG("Loginapp::loginFailed: accountName=%s login is failed. failedcode=%d", accountName.c_str(), failedcode);
	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onLoginFailed);
	bundle << failedcode;
	bundle.send(this->getNetworkInterface(), pChannel);

	PendingLoginMgr::PLInfos* infos = pendingLoginMgr_.remove(accountName);
	SAFE_RELEASE(infos);
}

//-------------------------------------------------------------------------------------
void Loginapp::onLoginAccountQueryResultFromDbmgr(Mercury::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	std::string accountName, password;
	bool success = true;

	s >> success;
	s >> accountName;
	s >> password;

	if(!success)
	{
		_loginFailed(pChannel, accountName, MERCURY_ERR_NAME_PASSWORD);
		return;
	}

	// 获得baseappmgr地址。
	Components::COMPONENTS cts = Components::getSingleton().getComponents(BASEAPPMGR_TYPE);
	Components::ComponentInfos* baseappmgrinfos = NULL;
	if(cts.size() > 0)
		baseappmgrinfos = &(*cts.begin());

	if(baseappmgrinfos == NULL || baseappmgrinfos->pChannel == NULL || baseappmgrinfos->cid == 0)
	{
		_loginFailed(pChannel, accountName, MERCURY_ERR_SRV_NO_READY);
		return;
	}

	// 注册到baseapp并且获取baseapp的地址
	Mercury::Bundle bundle;
	bundle.newMessage(BaseappmgrInterface::registerPendingAccountToBaseapp);

	bundle << accountName;
	bundle << password;
	bundle.send(this->getNetworkInterface(), baseappmgrinfos->pChannel);
}

//-------------------------------------------------------------------------------------
void Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr(Mercury::Channel* pChannel, std::string& accountName, uint32 addr, uint16 port)
{
	if(pChannel->isExternal())
		return;

	Mercury::Address address(addr, port);
	DEBUG_MSG("Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr:%s.\n", address.c_str());

	PendingLoginMgr::PLInfos* infos = pendingLoginMgr_.remove(accountName);
	if(infos == NULL)
		return;

	Mercury::Channel* pClientChannel = this->getNetworkInterface().findChannel(infos->addr);
	SAFE_RELEASE(infos);

	if(pClientChannel == NULL)
		return;

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onLoginSuccessfully);
	uint16 fport = ntohs(port);
	bundle << inet_ntoa((struct in_addr&)addr);
	bundle << fport;
	bundle.send(this->getNetworkInterface(), pClientChannel);
}

//-------------------------------------------------------------------------------------

}
