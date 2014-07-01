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
void SyncAppDatasHandler::pushApp(COMPONENT_ID cid, int32 startGroupOrder, int32 startGlobalOrder)
{
	lastRegAppTime_ = timestamp();
	std::vector<ComponentInitInfo>::iterator iter = apps_.begin();
	for(; iter != apps_.end(); iter++)
	{
		if((*iter).cid == cid)
		{
			ERROR_MSG(boost::format("SyncAppDatasHandler::pushApp: cid(%1%) is exist!\n") % cid);
			return;
		}
	}

	ComponentInitInfo cinfo;
	cinfo.cid = cid;
	cinfo.startGroupOrder = startGroupOrder;
	cinfo.startGlobalOrder = startGlobalOrder;
	apps_.push_back(cinfo);
}

//-------------------------------------------------------------------------------------
bool SyncAppDatasHandler::process()
{
	if(lastRegAppTime_ == 0)
		return true;

	bool hasApp = false;

	std::vector<ComponentInitInfo>::iterator iter = apps_.begin();
	for(; iter != apps_.end(); iter++)
	{
		ComponentInitInfo cInitInfo = (*iter);
		Components::ComponentInfos* cinfos = Componentbridge::getComponents().findComponent(cInitInfo.cid);

		if(cinfos == NULL)
			continue;

		COMPONENT_TYPE tcomponentType = cinfos->componentType;
		if(tcomponentType == BASEAPP_TYPE || 
			tcomponentType == CELLAPP_TYPE)
		{
			hasApp = true;
			break;
		}
	}
	
	if(!hasApp)
	{
		lastRegAppTime_ = timestamp();
		return true;
	}

	if(timestamp() - lastRegAppTime_ < uint64( 3 * stampsPerSecond() ) )
		return true;

	std::string digest = EntityDef::md5().getDigestStr();

	// 如果是连接到dbmgr则需要等待接收app初始信息
	// 例如：初始会分配entityID段以及这个app启动的顺序信息（是否第一个baseapp启动）
	iter = apps_.begin();
	for(; iter != apps_.end(); iter++)
	{
		ComponentInitInfo cInitInfo = (*iter);
		Components::ComponentInfos* cinfos = Componentbridge::getComponents().findComponent(cInitInfo.cid);

		if(cinfos == NULL)
			continue;

		COMPONENT_TYPE tcomponentType = cinfos->componentType;

		if(tcomponentType == BASEAPP_TYPE || 
			tcomponentType == CELLAPP_TYPE || 
			tcomponentType == LOGINAPP_TYPE)
		{
			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			
			switch(tcomponentType)
			{
			case BASEAPP_TYPE:
				{
					Dbmgr::getSingleton().onGlobalDataClientLogon(cinfos->pChannel, BASEAPP_TYPE);

					std::pair<ENTITY_ID, ENTITY_ID> idRange = Dbmgr::getSingleton().idServer().allocRange();
					(*pBundle).newMessage(BaseappInterface::onDbmgrInitCompleted);
					BaseappInterface::onDbmgrInitCompletedArgs6::staticAddToBundle((*pBundle), g_kbetime, idRange.first, 
						idRange.second, cInitInfo.startGlobalOrder, cInitInfo.startGroupOrder, digest);
				}
				break;
			case CELLAPP_TYPE:
				{
					Dbmgr::getSingleton().onGlobalDataClientLogon(cinfos->pChannel, CELLAPP_TYPE);

					std::pair<ENTITY_ID, ENTITY_ID> idRange = Dbmgr::getSingleton().idServer().allocRange();
					(*pBundle).newMessage(CellappInterface::onDbmgrInitCompleted);
					CellappInterface::onDbmgrInitCompletedArgs6::staticAddToBundle((*pBundle), g_kbetime, idRange.first, 
						idRange.second, cInitInfo.startGlobalOrder, cInitInfo.startGroupOrder, digest);
				}
				break;
			case LOGINAPP_TYPE:
				(*pBundle).newMessage(LoginappInterface::onDbmgrInitCompleted);
				LoginappInterface::onDbmgrInitCompletedArgs3::staticAddToBundle((*pBundle), 
					cInitInfo.startGlobalOrder, cInitInfo.startGroupOrder, digest);

				break;
			default:
				break;
			}

			(*pBundle).send(networkInterface_, cinfos->pChannel);
			Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		}
	}

	apps_.clear();

	delete this;
	return false;
}

//-------------------------------------------------------------------------------------

}
