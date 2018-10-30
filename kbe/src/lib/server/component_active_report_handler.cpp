// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "component_active_report_handler.h"
#include "network/network_interface.h"
#include "network/bundle.h"
#include "server/serverapp.h"
#include "server/components.h"

#include "../../server/baseappmgr/baseappmgr_interface.h"
#include "../../server/cellappmgr/cellappmgr_interface.h"
#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"
#include "../../server/loginapp/loginapp_interface.h"
#include "../../server/tools/logger/logger_interface.h"
#include "../../server/tools/interfaces/interfaces_interface.h"

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
	pActiveTimerHandle_ = pApp_->dispatcher().addTimer(int(period * 1000000),
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
								BASEAPP_TYPE, LOGINAPP_TYPE, LOGGER_TYPE, UNKNOWN_COMPONENT_TYPE};
			
			int ifind = 0;
			while(findComponentTypes[ifind] != UNKNOWN_COMPONENT_TYPE)
			{
				COMPONENT_TYPE componentType = (COMPONENT_TYPE)findComponentTypes[ifind];

				Components::COMPONENTS& components = Components::getSingleton().getComponents(componentType);
				Components::COMPONENTS::iterator iter = components.begin();
				for(; iter != components.end(); ++iter)
				{
					Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
					COMMON_NETWORK_MESSAGE(componentType, (*pBundle), onAppActiveTick);
					
					(*pBundle) << g_componentType;
					(*pBundle) << g_componentID;

					if((*iter).pChannel != NULL)
						(*iter).pChannel->send(pBundle);
					else
						Network::Bundle::reclaimPoolObject(pBundle);
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
