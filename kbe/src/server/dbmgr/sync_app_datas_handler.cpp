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

#include "dbmgr.hpp"
#include "sync_app_datas_handler.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "entitydef/entity_macro.hpp"
#include "network/fixed_messages.hpp"
#include "math/math.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"

#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
SyncAppDatasHandler::SyncAppDatasHandler(Mercury::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface),
lastRegAppTime_(0),
apps_()
{
	networkInterface.mainDispatcher().addFrequentTask(this);
}

//-------------------------------------------------------------------------------------
SyncAppDatasHandler::~SyncAppDatasHandler()
{
	// networkInterface_.mainDispatcher().cancelFrequentTask(this);
	DEBUG_MSG("SyncAppDatasHandler::~SyncAppDatasHandler()\n");

	Dbmgr::getSingleton().pSyncAppDatasHandler(NULL);
}

//-------------------------------------------------------------------------------------
void SyncAppDatasHandler::pushApp(COMPONENT_ID cid)
{
	lastRegAppTime_ = timestamp();
	if(std::find(apps_.begin(), apps_.end(), cid) != apps_.end())
		return;

	apps_.push_back(cid);
}

//-------------------------------------------------------------------------------------
bool SyncAppDatasHandler::process()
{
	if(lastRegAppTime_ == 0)
		return true;

	if(timestamp() - lastRegAppTime_ < uint64( 3 * stampsPerSecond() ) )
		return true;

	bool hasDone = false;

	std::vector<COMPONENT_ID>::iterator iter = apps_.begin();
	for(; iter != apps_.end(); iter++)
	{
		COMPONENT_ID componentID = (*iter);
		Components::ComponentInfos* cinfos = Componentbridge::getComponents().findComponent(componentID);

		if(cinfos == NULL)
			continue;

		switch(cinfos->componentType)
		{
		case BASEAPP_TYPE:
			{
				Dbmgr::getSingleton().onGlobalDataClientLogon(cinfos->pChannel, cinfos->componentType);
				hasDone = true;
			}
			break;
		case CELLAPP_TYPE:
			{
				Dbmgr::getSingleton().onGlobalDataClientLogon(cinfos->pChannel, cinfos->componentType);
				hasDone = true;
			}
			break;
		default:
			break;
		}
	}

	apps_.clear();
	lastRegAppTime_ = timestamp();

	if(!hasDone)
	{
		return true;
	}

	delete this;
	return false;
}

//-------------------------------------------------------------------------------------

}
