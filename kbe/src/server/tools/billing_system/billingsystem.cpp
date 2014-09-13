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

#include "orders.hpp"
#include "billingsystem.hpp"
#include "billing_tasks.hpp"
#include "anonymous_channel.hpp"
#include "billingsystem_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"


#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"
#include "dbmgr/dbmgr_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(BillingSystem);

//-------------------------------------------------------------------------------------
BillingSystem::BillingSystem(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
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
BillingSystem::~BillingSystem()
{
	mainProcessTimer_.cancel();
	lockthread();

	if(reqCreateAccount_requests_.size() > 0)
	{	
		int i = 0;
		REQCREATE_MAP::iterator iter = reqCreateAccount_requests_.begin();
		for(; iter != reqCreateAccount_requests_.end(); iter++)
		{
			WARNING_MSG(fmt::format("BillingSystem::~BillingSystem(): Discarding {0}/{1} reqCreateAccount[{2:x}] tasks.\n", 
				++i, reqCreateAccount_requests_.size(), (uintptr)iter->second));
		}
	}

	if(reqAccountLogin_requests_.size() > 0)
	{
		int i = 0;
		REQLOGIN_MAP::iterator iter = reqAccountLogin_requests_.begin();
		for(; iter != reqAccountLogin_requests_.end(); iter++)
		{
			WARNING_MSG(fmt::format("BillingSystem::~BillingSystem(): Discarding {0}/{1} reqAccountLogin[{2:x}] tasks.\n", 
				++i, reqAccountLogin_requests_.size(), (uintptr)iter->second));
		}
	}

	unlockthread();
}

//-------------------------------------------------------------------------------------	
void BillingSystem::onShutdownEnd()
{
	ServerApp::onShutdownEnd();
}

//-------------------------------------------------------------------------------------
void BillingSystem::lockthread()
{
	mutex_.lockMutex();
}

//-------------------------------------------------------------------------------------
void BillingSystem::unlockthread()
{
	mutex_.unlockMutex();
}

//-------------------------------------------------------------------------------------
void BillingSystem::eraseOrders_s(std::string ordersid)
{
	lockthread();

	ORDERS::iterator iter = orders_.find(ordersid);
	if(iter != orders_.end())
	{
		ERROR_MSG(boost::format("BillingSystem::eraseOrders_s: chargeID=%1% not found!\n") % ordersid);
	}

	orders_.erase(iter);
	unlockthread();
}

//-------------------------------------------------------------------------------------
bool BillingSystem::hasOrders(std::string ordersid)
{
	bool ret = false;
	
	lockthread();
	ORDERS::iterator iter = orders_.find(ordersid);
	ret = (iter != orders_.end());
	unlockthread();
	
	return ret;
}

//-------------------------------------------------------------------------------------
bool BillingSystem::run()
{
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
void BillingSystem::handleTimeout(TimerHandle handle, void * arg)
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
void BillingSystem::handleMainTick()
{
	 //time_t t = ::time(NULL);
	 //DEBUG_MSG("BillingSystem::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	g_kbetime++;
	threadPool_.onMainThreadTick();
	networkInterface().processAllChannelPackets(&BillingSystemInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool BillingSystem::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool BillingSystem::inInitialize()
{
	// 广播自己的地址给网上上的所有kbemachine
	Componentbridge::getSingleton().getComponents().pHandler(this);
	this->mainDispatcher().addFrequentTask(&Componentbridge::getSingleton());
	return true;
}

//-------------------------------------------------------------------------------------
bool BillingSystem::initializeEnd()
{
	mainProcessTimer_ = this->mainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_TICK));

	// 不做频道超时检查
	CLOSE_CHANNEL_INACTIVITIY_DETECTION();

	this->threadPool().addTask(new AnonymousChannel());

	return initDB();
}

//-------------------------------------------------------------------------------------		
bool BillingSystem::initDB()
{
	return true;
}

//-------------------------------------------------------------------------------------
void BillingSystem::finalise()
{
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void BillingSystem::reqCreateAccount(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
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
	pinfo->success = false;
	pinfo->baseappID = cid;
	pinfo->dbmgrID = pChannel->componentID();
	pinfo->address = pChannel->addr();
	pinfo->enable = true;

	reqCreateAccount_requests_[pinfo->commitName] = pinfo;
	unlockthread();

	this->threadPool().addTask(pinfo);
}

//-------------------------------------------------------------------------------------
void BillingSystem::onAccountLogin(Mercury::Channel* pChannel, KBEngine::MemoryStream& s) 
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
	pinfo->success = false;
	pinfo->baseappID = cid;
	pinfo->dbmgrID = pChannel->componentID();
	pinfo->address = pChannel->addr();
	pinfo->enable = true;

	reqAccountLogin_requests_[pinfo->commitName] = pinfo;
	unlockthread();

	this->threadPool().addTask(pinfo);
}

//-------------------------------------------------------------------------------------
void BillingSystem::charge(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	OrdersCharge* pOrdersCharge = new OrdersCharge();

	pOrdersCharge->timeout = timestamp()  + uint64(g_kbeSrvConfig.billingSystem_orders_timeout_ * stampsPerSecond());

	pOrdersCharge->dbmgrID = pChannel->componentID();
	pOrdersCharge->address = pChannel->addr();

	s >> pOrdersCharge->baseappID;
	s >> pOrdersCharge->ordersID;
	s >> pOrdersCharge->dbid;
	s.readBlob(pOrdersCharge->postDatas);
	s >> pOrdersCharge->cbid;

	INFO_MSG(boost::format("BillingSystem::charge: componentID=%5%, chargeID=%1%, dbid=%2%, cbid=%3%, datas=%4%!\n") %
		pOrdersCharge->ordersID % pOrdersCharge->dbid % pOrdersCharge->cbid % pOrdersCharge->postDatas % pOrdersCharge->baseappID);

	lockthread();

	ORDERS::iterator iter = orders_.find(pOrdersCharge->ordersID);
	if(iter != orders_.end())
	{
		ERROR_MSG(boost::format("BillingSystem::charge: chargeID=%1% is exist!\n") % pOrdersCharge->ordersID);
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
void BillingSystem::eraseClientReq(Mercury::Channel* pChannel, std::string& logkey)
{
	lockthread();

	REQCREATE_MAP::iterator citer = reqCreateAccount_requests_.find(logkey);
	if(citer != reqCreateAccount_requests_.end())
	{
		citer->second->enable = false;
		DEBUG_MSG(boost::format("BillingSystem::eraseClientReq: reqCreateAccount_logkey=%1% set disabled!\n") % logkey);
	}

	REQLOGIN_MAP::iterator liter = reqAccountLogin_requests_.find(logkey);
	if(liter != reqAccountLogin_requests_.end())
	{
		liter->second->enable = false;
		DEBUG_MSG(boost::format("BillingSystem::eraseClientReq: reqAccountLogin_logkey=%1% set disabled!\n") % logkey);
	}

	unlockthread();
}

//-------------------------------------------------------------------------------------

}
