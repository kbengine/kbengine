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

#include "orders.h"
#include "interfaces.h"
#include "interfaces_tasks.h"
#include "anonymous_channel.h"
#include "interfaces_interface.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/components.h"


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
	ServerApp(dispatcher, ninterface, componentType, componentID),
	mainProcessTimer_(),
	reqCreateAccount_requests_(),
	reqAccountLogin_requests_(),
	mutex_()
{
}

//-------------------------------------------------------------------------------------
Interfaces::~Interfaces()
{
	mainProcessTimer_.cancel();
	lockthread();

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

	unlockthread();
}

//-------------------------------------------------------------------------------------	
void Interfaces::onShutdownEnd()
{
	ServerApp::onShutdownEnd();
}

//-------------------------------------------------------------------------------------
void Interfaces::lockthread()
{
	mutex_.lockMutex();
}

//-------------------------------------------------------------------------------------
void Interfaces::unlockthread()
{
	mutex_.unlockMutex();
}

//-------------------------------------------------------------------------------------
void Interfaces::eraseOrders_s(std::string ordersid)
{
	lockthread();

	ORDERS::iterator iter = orders_.find(ordersid);
	if(iter != orders_.end())
	{
		ERROR_MSG(fmt::format("Interfaces::eraseOrders_s: chargeID={} not found!\n", ordersid));
	}

	orders_.erase(iter);
	unlockthread();
}

//-------------------------------------------------------------------------------------
bool Interfaces::hasOrders(std::string ordersid)
{
	bool ret = false;
	
	lockthread();
	ORDERS::iterator iter = orders_.find(ordersid);
	ret = (iter != orders_.end());
	unlockthread();
	
	return ret;
}

//-------------------------------------------------------------------------------------
bool Interfaces::run()
{
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
void Interfaces::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_TICK:
			this->handleMainTick();
			break;
		default:
			break;
	}

	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Interfaces::handleMainTick()
{
	 //time_t t = ::time(NULL);
	 //DEBUG_MSG("Interfaces::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	g_kbetime++;
	threadPool_.onMainThreadTick();
	networkInterface().processAllChannelPackets(&InterfacesInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool Interfaces::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Interfaces::inInitialize()
{
	// 广播自己的地址给网上上的所有kbemachine
	Components::getSingleton().pHandler(this);
	this->dispatcher().addTask(&Components::getSingleton());
	return true;
}

//-------------------------------------------------------------------------------------
bool Interfaces::initializeEnd()
{
	mainProcessTimer_ = this->dispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_TICK));

	// 不做频道超时检查
	CLOSE_CHANNEL_INACTIVITIY_DETECTION();

	this->threadPool().addTask(new AnonymousChannel());

	return initDB();
}

//-------------------------------------------------------------------------------------		
bool Interfaces::initDB()
{
	return true;
}

//-------------------------------------------------------------------------------------
void Interfaces::finalise()
{
	ServerApp::finalise();
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

	lockthread();

	REQCREATE_MAP::iterator iter = reqCreateAccount_requests_.find(registerName);
	if(iter != reqCreateAccount_requests_.end())
	{
		unlockthread();
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
	unlockthread();

	this->threadPool().addTask(pinfo);
}

//-------------------------------------------------------------------------------------
void Interfaces::onAccountLogin(Network::Channel* pChannel, KBEngine::MemoryStream& s) 
{
	std::string loginName, accountName, password, datas;
	COMPONENT_ID cid;

	s >> cid >> loginName >> password;
	s.readBlob(datas);

	lockthread();

	REQLOGIN_MAP::iterator iter = reqAccountLogin_requests_.find(loginName);
	if(iter != reqAccountLogin_requests_.end())
	{
		unlockthread();
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
	unlockthread();

	this->threadPool().addTask(pinfo);
}

//-------------------------------------------------------------------------------------
void Interfaces::charge(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	OrdersCharge* pOrdersCharge = new OrdersCharge();

	pOrdersCharge->timeout = timestamp()  + uint64(g_kbeSrvConfig.interfaces_orders_timeout_ * stampsPerSecond());

	pOrdersCharge->dbmgrID = pChannel->componentID();
	pOrdersCharge->address = pChannel->addr();

	s >> pOrdersCharge->baseappID;
	s >> pOrdersCharge->ordersID;
	s >> pOrdersCharge->dbid;
	s.readBlob(pOrdersCharge->postDatas);
	s >> pOrdersCharge->cbid;

	INFO_MSG(fmt::format("Interfaces::charge: componentID={4}, chargeID={0}, dbid={1}, cbid={2}, datas={3}!\n",
		pOrdersCharge->ordersID, pOrdersCharge->dbid, pOrdersCharge->cbid, pOrdersCharge->postDatas, pOrdersCharge->baseappID));

	lockthread();

	ORDERS::iterator iter = orders_.find(pOrdersCharge->ordersID);
	if(iter != orders_.end())
	{
		ERROR_MSG(fmt::format("Interfaces::charge: chargeID={} is exist!\n", pOrdersCharge->ordersID));
		delete pOrdersCharge;
		unlockthread();
		return;
	}

	ChargeTask* pinfo = new ChargeTask();
	pinfo->orders = *pOrdersCharge;
	pinfo->pOrders = pOrdersCharge;
	orders_[pOrdersCharge->ordersID].reset(pOrdersCharge);
	unlockthread();
	
	this->threadPool().addTask(pinfo);
}

//-------------------------------------------------------------------------------------
void Interfaces::eraseClientReq(Network::Channel* pChannel, std::string& logkey)
{
	lockthread();

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

	unlockthread();
}

//-------------------------------------------------------------------------------------

}
