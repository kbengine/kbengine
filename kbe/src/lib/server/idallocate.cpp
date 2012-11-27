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


#include "helper/debug_helper.hpp"
#include "idallocate.hpp"
#include "serverapp.hpp"
#include "components.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
void EntityIDClient::onAlloc(void)
{
	if(hasReqServerAlloc() || getSize() > ID_ENOUGH_LIMIT || idList_.size() > 0)
		return;
	
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(DbmgrInterface::onReqAllocEntityID);
	DbmgrInterface::onReqAllocEntityIDArgs2::staticAddToBundle((*pBundle), pApp_->componentType(), pApp_->componentID());

	Components::COMPONENTS cts =  Components::getSingleton().getComponents(DBMGR_TYPE);
	KBE_ASSERT(cts.size() > 0);
	Components::ComponentInfos* cinfos = &(*cts.begin());
	KBE_ASSERT(cinfos->pChannel != NULL);

	(*pBundle).send(pApp_->getNetworkInterface(), cinfos->pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	ERROR_MSG(boost::format("EntityIDClient::onAlloc: not enough(%1%) entityIDs!\n") % ID_ENOUGH_LIMIT);
}

//-------------------------------------------------------------------------------------


}
