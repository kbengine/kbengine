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
		threadPool_.onMainThreadTick();
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

	INFO_MSG(boost::format("Loginapp::onDbmgrInitCompleted:startGlobalOrder=%1%, startGroupOrder=%2%.\n") %
		startGlobalOrder % startGroupOrder);

	startGlobalOrder_ = startGlobalOrder;
	startGroupOrder_ = startGroupOrder;
}

//-------------------------------------------------------------------------------------
void Loginapp::reqCreateAccount(Mercury::Channel* pChannel, MemoryStream& s)
{
	std::string accountName, password, datas, retdatas = "";

	s >> accountName >> password;
	s.readBlob(datas);

	DEBUG_MSG(boost::format("Loginapp::reqCreateAccount: accountName=%1%, passwordsize=%2%.\n") %
		accountName.c_str() % password.size());

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.find(accountName);
	if(ptinfos != NULL)
	{
		Mercury::Bundle bundle;
		bundle.newMessage(ClientInterface::onCreateAccountResult);
		bundle << SERVER_ERR_BUSY;
		bundle.appendBlob(retdatas);
		bundle.send(this->getNetworkInterface(), pChannel);
		return;
	}
			
	ptinfos = new PendingLoginMgr::PLInfos;
	ptinfos->accountName = accountName;
	ptinfos->password = password;
	ptinfos->datas = datas;
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
		bundle << SERVER_ERR_SRV_NO_READY;
		bundle.appendBlob(retdatas);
		bundle.send(this->getNetworkInterface(), pChannel);
		return;
	}

	Mercury::Bundle bundle;
	bundle.newMessage(DbmgrInterface::reqCreateAccount);
	bundle << accountName << password;
	bundle.appendBlob(datas);
	bundle.send(this->getNetworkInterface(), dbmgrinfos->pChannel);
}

//-------------------------------------------------------------------------------------
void Loginapp::onReqCreateAccountResult(Mercury::Channel* pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;
	std::string accountName;
	std::string password;
	std::string retdatas = "";

	s >> failedcode >> accountName >> password;
	s.readBlob(retdatas);

	DEBUG_MSG(boost::format("Loginapp::onReqCreateAccountResult: accountName=%1%, failedcode=%2%.\n") %
		accountName.c_str() % failedcode);

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.remove(accountName);
	if(ptinfos == NULL)
		return;

	Mercury::Channel* pClientChannel = this->getNetworkInterface().findChannel(ptinfos->addr);
	if(pClientChannel == NULL)
		return;

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onCreateAccountResult);
	bundle << failedcode;
	bundle.appendBlob(retdatas);

	bundle.send(this->getNetworkInterface(), pClientChannel);

	SAFE_RELEASE(ptinfos);
}

//-------------------------------------------------------------------------------------
void Loginapp::login(Mercury::Channel* pChannel, MemoryStream& s)
{
	COMPONENT_CLIENT_TYPE ctype;
	int8 tctype = 0;
	std::string loginName;
	std::string password;
	std::string datas;

	// 前端类别
	s >> tctype;
	ctype = static_cast<COMPONENT_CLIENT_TYPE>(tctype);
	
	// 附带数据
	s.readBlob(datas);

	// 帐号登录名
	s >> loginName;

	// 密码
	s >> password;
	
	if(loginName.size() == 0)
	{
		ERROR_MSG("Loginapp::login: loginName is NULL.\n");
		return;
	}

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.find(loginName);
	if(ptinfos != NULL)
	{
		datas = "";
		_loginFailed(pChannel, loginName, SERVER_ERR_BUSY, datas);
		return;
	}

	ptinfos = new PendingLoginMgr::PLInfos;
	ptinfos->ctype = ctype;
	ptinfos->datas = datas;
	ptinfos->accountName = loginName;
	ptinfos->password = password;
	ptinfos->addr = pChannel->addr();
	pendingLoginMgr_.add(ptinfos);

	INFO_MSG(boost::format("Loginapp::login: new client[%1%], loginName=%2%, datas=%3%.\n") %
		COMPONENT_CLIENT_NAME[ctype] % loginName.c_str() % datas.c_str());

	// 首先必须baseappmgr和dbmgr都已经准备完毕了。
	Components::COMPONENTS cts = Components::getSingleton().getComponents(BASEAPPMGR_TYPE);
	Components::ComponentInfos* baseappmgrinfos = NULL;
	if(cts.size() > 0)
		baseappmgrinfos = &(*cts.begin());

	if(baseappmgrinfos == NULL || baseappmgrinfos->pChannel == NULL || baseappmgrinfos->cid == 0)
	{
		datas = "";
		_loginFailed(pChannel, loginName, SERVER_ERR_SRV_NO_READY, datas);
		return;
	}

	cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		datas = "";
		_loginFailed(pChannel, loginName, SERVER_ERR_SRV_NO_READY, datas);
		return;
	}

	// 向dbmgr查询用户合法性
	Mercury::Bundle bundle;
	bundle.newMessage(DbmgrInterface::onAccountLogin);
	bundle << loginName << password;
	bundle.appendBlob(datas);
	bundle.send(this->getNetworkInterface(), dbmgrinfos->pChannel);
}

//-------------------------------------------------------------------------------------
void Loginapp::_loginFailed(Mercury::Channel* pChannel, std::string& loginName, SERVER_ERROR_CODE failedcode, std::string& datas)
{
	DEBUG_MSG(boost::format("Loginapp::loginFailed: loginName=%1% login is failed. failedcode=%2%, datas=%3%.\n") %
		loginName.c_str() % SERVER_ERR_STR[failedcode] % datas);
	
	PendingLoginMgr::PLInfos* infos = pendingLoginMgr_.remove(loginName);
	if(infos == NULL)
		return;

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onLoginFailed);
	bundle << failedcode;
	bundle.appendBlob(datas);

	if(pChannel)
	{
		bundle.send(this->getNetworkInterface(), pChannel);
	}
	else 
	{
		Mercury::Channel* pClientChannel = this->getNetworkInterface().findChannel(infos->addr);
		if(pClientChannel)
			bundle.send(this->getNetworkInterface(), pClientChannel);
	}

	SAFE_RELEASE(infos);
}

//-------------------------------------------------------------------------------------
void Loginapp::onLoginAccountQueryResultFromDbmgr(Mercury::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	std::string loginName, accountName, password, datas;
	bool success = true;
	COMPONENT_ID componentID;
	ENTITY_ID entityID;
	DBID dbid;

	s >> success;

	// 登录名既登录时客户端输入的名称， 账号名则是dbmgr查询得到的名称
	// 这个机制用于一个账号多名称系统或者多个第三方账号系统登入服务器
	// accountName为本游戏服务器账号所绑定的终身名称
	// 客户端得到baseapp地址的同时也会返回这个账号名称
	// 客户端登陆baseapp应该使用这个账号名称登陆
	s >> loginName;
	s >> accountName;

	s >> password;
	s >> componentID;
	s >> entityID;
	s >> dbid;

	s.readBlob(datas);

	PendingLoginMgr::PLInfos* infos = pendingLoginMgr_.find(loginName);
	if(infos == NULL)
	{
		_loginFailed(NULL, loginName, SERVER_ERR_SRV_OVERLOAD, datas);
		return;
	}

	infos->datas = datas;

	if(!success && entityID == 0 && componentID == 0)
	{
		_loginFailed(NULL, loginName, SERVER_ERR_NAME_PASSWORD, datas);
		return;
	}

	// 获得baseappmgr地址。
	Components::COMPONENTS cts = Components::getSingleton().getComponents(BASEAPPMGR_TYPE);
	Components::ComponentInfos* baseappmgrinfos = NULL;
	if(cts.size() > 0)
		baseappmgrinfos = &(*cts.begin());

	if(baseappmgrinfos == NULL || baseappmgrinfos->pChannel == NULL || baseappmgrinfos->cid == 0)
	{
		_loginFailed(NULL, loginName, SERVER_ERR_SRV_NO_READY, datas);
		return;
	}

	// 如果大于0则说明当前账号仍然存活于某个baseapp上
	if(componentID > 0)
	{
		Mercury::Bundle bundle;
		bundle.newMessage(BaseappmgrInterface::registerPendingAccountToBaseappAddr);
		bundle << componentID << loginName << accountName << password << entityID << dbid;
		bundle.send(this->getNetworkInterface(), baseappmgrinfos->pChannel);
		return;
	}
	else
	{
		// 注册到baseapp并且获取baseapp的地址
		Mercury::Bundle bundle;
		bundle.newMessage(BaseappmgrInterface::registerPendingAccountToBaseapp);

		bundle << loginName;
		bundle << accountName;
		bundle << password;
		bundle << dbid;
		bundle.send(this->getNetworkInterface(), baseappmgrinfos->pChannel);
	}
}

//-------------------------------------------------------------------------------------
void Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr(Mercury::Channel* pChannel, std::string& loginName, 
															std::string& accountName, uint32 addr, uint16 port)
{
	if(pChannel->isExternal())
		return;

	Mercury::Address address(addr, port);

	DEBUG_MSG(boost::format("Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr:%1%.\n") % 
		address.c_str());

	// 这里可以不做删除， 仍然使其保留一段时间避免同一时刻同时登录造成意外影响
	PendingLoginMgr::PLInfos* infos = pendingLoginMgr_.remove(loginName);
	if(infos == NULL)
		return;
	
	infos->lastProcessTime = timestamp();
	Mercury::Channel* pClientChannel = this->getNetworkInterface().findChannel(infos->addr);

	if(pClientChannel == NULL)
	{
		SAFE_RELEASE(infos);
		return;
	}

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onLoginSuccessfully);
	uint16 fport = ntohs(port);
	bundle << accountName;
	bundle << inet_ntoa((struct in_addr&)addr);
	bundle << fport;
	bundle.appendBlob(infos->datas);
	bundle.send(this->getNetworkInterface(), pClientChannel);

	SAFE_RELEASE(infos);
}

//-------------------------------------------------------------------------------------

}
