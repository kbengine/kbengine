/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/components.h"
#include "server/serverconfig.h"
#include "openssl/md5.h"

#include "baseapp/baseapp_interface.h"
#include "cellapp/cellapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "cellappmgr/cellappmgr_interface.h"
#include "loginapp/loginapp_interface.h"
#include "dbmgr/dbmgr_interface.h"

#if KBE_PLATFORM == PLATFORM_WIN32
#ifdef _DEBUG
#pragma comment(lib, "libeay32_d.lib")
#pragma comment(lib, "ssleay32_d.lib")
#else
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif
#endif


namespace KBEngine{

//-------------------------------------------------------------------------------------
InterfacesTask::InterfacesTask()
{
}

//-------------------------------------------------------------------------------------
InterfacesTask::~InterfacesTask()
{
}

//-------------------------------------------------------------------------------------
CreateAccountTask::CreateAccountTask():
InterfacesTask()
{
}

//-------------------------------------------------------------------------------------
CreateAccountTask::~CreateAccountTask()
{
}

//-------------------------------------------------------------------------------------
LoginAccountTask::LoginAccountTask():
CreateAccountTask()
{
}

//-------------------------------------------------------------------------------------
LoginAccountTask::~LoginAccountTask()
{
}

//-------------------------------------------------------------------------------------
ChargeTask::ChargeTask():
InterfacesTask(),
pOrders(NULL),
retcode(SERVER_ERR_OP_FAILED)
{
}

//-------------------------------------------------------------------------------------
ChargeTask::~ChargeTask()
{
}

//-------------------------------------------------------------------------------------
}
