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

#include "idallocate.hpp"
#include "serverapp.hpp"
#include "components.hpp"
#include "helper/debug_helper.hpp"
#include "server/serverconfig.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"

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
			id_enough_limit = (size_t)g_kbeSrvConfig.getBaseApp().criticallyLowSize;
		else
			id_enough_limit = (size_t)g_kbeSrvConfig.getCellApp().criticallyLowSize;
	}
}

//-------------------------------------------------------------------------------------
void EntityIDClient::onAlloc(void)
{
	if(getSize() > id_enough_limit)
	{
		setReqServerAllocFlag(false);
		return;
	}
	
	if(hasReqServerAlloc())
		return;

	Mercury::Channel* pChannel = Components::getSingleton().getDbmgrChannel();

	if(pChannel == NULL)
	{
		ERROR_MSG("EntityIDClient::onAlloc: not found dbmgr!\n");
		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(DbmgrInterface::onReqAllocEntityID);
	DbmgrInterface::onReqAllocEntityIDArgs2::staticAddToBundle((*pBundle), pApp_->componentType(), pApp_->componentID());
	(*pBundle).send(pApp_->networkInterface(), pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	setReqServerAllocFlag(true);

	WARNING_MSG(boost::format("EntityIDClient::onAlloc: not enough(%1%) entityIDs!\n") % id_enough_limit);
}

//-------------------------------------------------------------------------------------


}
