// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "bots.h"
#include "clientobject.h"
#include "create_and_login_handler.h"
#include "network/network_interface.h"
#include "network/event_dispatcher.h"
#include "network/address.h"
#include "network/bundle.h"
#include "common/memorystream.h"
#include "server/serverconfig.h"

namespace KBEngine { 

uint64 g_accountID = 0;

//-------------------------------------------------------------------------------------
CreateAndLoginHandler::CreateAndLoginHandler()
{
	timerHandle_ = Bots::getSingleton().networkInterface().dispatcher().addTimer(
							1 * 1000000, this);

	g_accountID = KBEngine::genUUID64() * 100000;
	if(g_kbeSrvConfig.getBots().bots_account_name_suffix_inc > 0)
	{
		g_accountID = g_kbeSrvConfig.getBots().bots_account_name_suffix_inc;
	}
}

//-------------------------------------------------------------------------------------
CreateAndLoginHandler::~CreateAndLoginHandler()
{
	timerHandle_.cancel();
}

//-------------------------------------------------------------------------------------
void CreateAndLoginHandler::handleTimeout(TimerHandle handle, void * arg)
{
	KBE_ASSERT(handle == timerHandle_);
	
	Bots& bots = Bots::getSingleton();

	static float lasttick = bots.reqCreateAndLoginTickTime();

	if(lasttick > 0.f)
	{
		// 每个tick减去0.1秒， 为0则可以创建一次且重置;
		lasttick -= 0.1f;
		return;
	}

	uint32 count = bots.reqCreateAndLoginTickCount();

	while(bots.reqCreateAndLoginTotalCount() - bots.clients().size() > 0 && count-- > 0)
	{
		ClientObject* pClient = new ClientObject(g_kbeSrvConfig.getBots().bots_account_name_prefix + 
			KBEngine::StringConv::val2str(g_componentID) + "_" + KBEngine::StringConv::val2str(g_accountID++), 
			Bots::getSingleton().networkInterface());

		Bots::getSingleton().addClient(pClient);
	}
}

//-------------------------------------------------------------------------------------

}
