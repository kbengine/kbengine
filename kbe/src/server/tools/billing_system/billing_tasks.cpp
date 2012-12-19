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

#include "billingsystem.hpp"
#include "billing_tasks.hpp"
#include "network/common.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "server/serverconfig.hpp"
#include "openssl/md5.h"

#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"
#include "dbmgr/dbmgr_interface.hpp"

#if KBE_PLATFORM == PLATFORM_WIN32
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif

namespace KBEngine{

//-------------------------------------------------------------------------------------
BillingTask::BillingTask()
{
}

//-------------------------------------------------------------------------------------
BillingTask::~BillingTask()
{
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState BillingTask::presentMainThread()
{
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
CreateAccountTask::CreateAccountTask():
BillingTask()
{
}

//-------------------------------------------------------------------------------------
CreateAccountTask::~CreateAccountTask()
{
}

//-------------------------------------------------------------------------------------
bool CreateAccountTask::process()
{
	// 默认我们总是成功的
	success = true;

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState CreateAccountTask::presentMainThread()
{
	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	// 默认我们总是成功的
	bool success = true;
	
	(*(*bundle)).newMessage(DbmgrInterface::onCreateAccountCBFromBilling);
	(*(*bundle)) << baseappID << commitName << accountName << password << success;

	//  默认提交什么返回什么
	getDatas = postDatas;

	(*(*bundle)).appendBlob(getDatas);

	Mercury::Channel* pChannel = BillingSystem::getSingleton().getNetworkInterface().findChannel(address);

	if(pChannel)
	{
		(*(*bundle)).send(BillingSystem::getSingleton().getNetworkInterface(), pChannel);
	}
	else
	{
		ERROR_MSG(boost::format("BillingTask::process: not found channel. commitName=%1%\n") % commitName);
	}

	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
LoginAccountTask::LoginAccountTask():
BillingTask()
{
}

//-------------------------------------------------------------------------------------
LoginAccountTask::~LoginAccountTask()
{
}

//-------------------------------------------------------------------------------------
bool LoginAccountTask::process()
{
	// 默认我们总是成功的
	success = true;

	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState LoginAccountTask::presentMainThread()
{
	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();
	
	(*(*bundle)).newMessage(DbmgrInterface::onLoginAccountCBBFromBilling);
	(*(*bundle)) << baseappID << commitName << accountName << password << success;

	//  默认提交什么返回什么
	getDatas = postDatas;

	(*(*bundle)).appendBlob(getDatas);

	Mercury::Channel* pChannel = BillingSystem::getSingleton().getNetworkInterface().findChannel(address);

	if(pChannel)
	{
		(*(*bundle)).send(BillingSystem::getSingleton().getNetworkInterface(), pChannel);
	}
	else
	{
		ERROR_MSG(boost::format("BillingTask::process: not found channel. commitName=%1%\n") % commitName);
	}

	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
}
