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

#include "idallocate.h"
#include "serverapp.h"
#include "components.h"
#include "helper/debug_helper.h"
#include "server/serverconfig.h"
#include "../../server/dbmgr/dbmgr_interface.h"

namespace KBEngine{

static size_t id_enough_limit = 0;

//-------------------------------------------------------------------------------------
EntityIDClient::EntityIDClient():
	IDClient<ENTITY_ID>(),
	pApp_(NULL)
{
	if(id_enough_limit <= 0)
	{
		if(g_componentType == BASEAPP_TYPE)
			id_enough_limit = (size_t)g_kbeSrvConfig.getBaseApp().ids_criticallyLowSize;
		else
			id_enough_limit = (size_t)g_kbeSrvConfig.getCellApp().ids_criticallyLowSize;
	}
}

//-------------------------------------------------------------------------------------
void EntityIDClient::onAlloc(void)
{
	if(size() > id_enough_limit)
	{
		setReqServerAllocFlag(false);
		return;
	}
	
	if(hasReqServerAlloc())
		return;

	Network::Channel* pChannel = Components::getSingleton().getDbmgrChannel();

	if(pChannel == NULL)
	{
		ERROR_MSG("EntityIDClient::onAlloc: not found dbmgr!\n");
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(DbmgrInterface::onReqAllocEntityID);
	DbmgrInterface::onReqAllocEntityIDArgs2::staticAddToBundle((*pBundle), pApp_->componentType(), pApp_->componentID());
	pChannel->send(pBundle);

	setReqServerAllocFlag(true);

	WARNING_MSG(fmt::format("EntityIDClient::onAlloc: not enough({}) entityIDs!\n", id_enough_limit));
}

//-------------------------------------------------------------------------------------


}
