// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
