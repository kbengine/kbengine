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

#include "bots.hpp"
#include "clientobject.hpp"
#include "create_and_login_handler.hpp"
#include "network/network_interface.hpp"
#include "network/event_dispatcher.hpp"
#include "network/address.hpp"
#include "network/bundle.hpp"
#include "cstdkbe/memorystream.hpp"
#include "server/serverconfig.hpp"

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
