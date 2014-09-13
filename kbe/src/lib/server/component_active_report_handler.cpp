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


#include "component_active_report_handler.hpp"
#include "network/network_interface.hpp"
#include "network/bundle.hpp"
#include "server/serverapp.hpp"
#include "server/components.hpp"

#include "../../server/baseappmgr/baseappmgr_interface.hpp"
#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"
#include "../../server/tools/message_log/messagelog_interface.hpp"
#include "../../server/tools/billing_system/billingsystem_interface.hpp"

namespace KBEngine { 


//-------------------------------------------------------------------------------------
ComponentActiveReportHandler::ComponentActiveReportHandler(ServerApp* pApp) :
	pApp_(pApp),
	pActiveTimerHandle_()
{
}

//-------------------------------------------------------------------------------------
ComponentActiveReportHandler::~ComponentActiveReportHandler()
{
	cancel();
}

//-------------------------------------------------------------------------------------
void ComponentActiveReportHandler::cancel()
{
	pActiveTimerHandle_.cancel();
}

//-------------------------------------------------------------------------------------
void ComponentActiveReportHandler::startActiveTick(float period)
{
	cancel();
	pActiveTimerHandle_ = pApp_->mainDispatcher().addTimer(int(period * 1000000),
									this, (void *)TIMEOUT_ACTIVE_TICK);
}

//-------------------------------------------------------------------------------------
void ComponentActiveReportHandler::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_ACTIVE_TICK:
		{
			int8 findComponentTypes[] = {BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, DBMGR_TYPE, CELLAPP_TYPE, 
								BASEAPP_TYPE, LOGINAPP_TYPE, MESSAGELOG_TYPE, UNKNOWN_COMPONENT_TYPE};
			
			int ifind = 0;
			while(findComponentTypes[ifind] != UNKNOWN_COMPONENT_TYPE)
			{
				COMPONENT_TYPE componentType = (COMPONENT_TYPE)findComponentTypes[ifind];

				Components::COMPONENTS& components = Components::getSingleton().getComponents(componentType);
				Components::COMPONENTS::iterator iter = components.begin();
				for(; iter != components.end(); iter++)
				{
					Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
					COMMON_MERCURY_MESSAGE(componentType, (*pBundle), onAppActiveTick);
					
					(*pBundle) << g_componentType;
					(*pBundle) << g_componentID;
					if((*iter).pChannel != NULL)
						(*pBundle).send(pApp_->networkInterface(), (*iter).pChannel);

					Mercury::Bundle::ObjPool().reclaimObject(pBundle);
				}

				ifind++;
			}
			break;
		}
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------

}
