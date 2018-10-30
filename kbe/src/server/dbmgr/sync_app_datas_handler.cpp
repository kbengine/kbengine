// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "dbmgr.h"
#include "sync_app_datas_handler.h"
#include "entitydef/scriptdef_module.h"
#include "entitydef/entity_macro.h"
#include "network/fixed_messages.h"
#include "math/math.h"
#include "network/bundle.h"
#include "network/channel.h"
#include "server/components.h"

#include "baseapp/baseapp_interface.h"
#include "cellapp/cellapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "cellappmgr/cellappmgr_interface.h"
#include "loginapp/loginapp_interface.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
SyncAppDatasHandler::SyncAppDatasHandler(Network::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface),
lastRegAppTime_(0),
apps_()
{
	networkInterface.dispatcher().addTask(this);
}

//-------------------------------------------------------------------------------------
SyncAppDatasHandler::~SyncAppDatasHandler()
{
	// networkInterface_.dispatcher().cancelTask(this);
	DEBUG_MSG("SyncAppDatasHandler::~SyncAppDatasHandler()\n");

	Dbmgr::getSingleton().pSyncAppDatasHandler(NULL);
}

//-------------------------------------------------------------------------------------
void SyncAppDatasHandler::pushApp(COMPONENT_ID cid, COMPONENT_ORDER startGroupOrder, COMPONENT_ORDER startGlobalOrder)
{
	lastRegAppTime_ = timestamp();
	std::vector<ComponentInitInfo>::iterator iter = apps_.begin();
	for(; iter != apps_.end(); ++iter)
	{
		if((*iter).cid == cid)
		{
			ERROR_MSG(fmt::format("SyncAppDatasHandler::pushApp: cid({}) is exist!\n", cid));
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
	for(; iter != apps_.end(); ++iter)
	{
		ComponentInitInfo cInitInfo = (*iter);
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(cInitInfo.cid);

		if(cinfos == NULL)
			continue;

		COMPONENT_TYPE tcomponentType = cinfos->componentType;
		if(tcomponentType == BASEAPP_TYPE || 
			tcomponentType == CELLAPP_TYPE ||
			tcomponentType == LOGINAPP_TYPE)
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
	for(; iter != apps_.end(); ++iter)
	{
		ComponentInitInfo cInitInfo = (*iter);
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(cInitInfo.cid);

		if(cinfos == NULL)
			continue;

		COMPONENT_TYPE tcomponentType = cinfos->componentType;

		if(tcomponentType == BASEAPP_TYPE || 
			tcomponentType == CELLAPP_TYPE || 
			tcomponentType == LOGINAPP_TYPE)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			
			switch(tcomponentType)
			{
			case BASEAPP_TYPE:
				{
					Dbmgr::getSingleton().onGlobalDataClientLogon(cinfos->pChannel, BASEAPP_TYPE);

					std::pair<ENTITY_ID, ENTITY_ID> idRange = Dbmgr::getSingleton().idServer().allocRange();
					(*pBundle).newMessage(BaseappInterface::onDbmgrInitCompleted);
					BaseappInterface::onDbmgrInitCompletedArgs6::staticAddToBundle((*pBundle), g_kbetime, idRange.first, 
						idRange.second, cInitInfo.startGlobalOrder, cInitInfo.startGroupOrder, 
						digest);
				}
				break;
			case CELLAPP_TYPE:
				{
					Dbmgr::getSingleton().onGlobalDataClientLogon(cinfos->pChannel, CELLAPP_TYPE);

					std::pair<ENTITY_ID, ENTITY_ID> idRange = Dbmgr::getSingleton().idServer().allocRange();
					(*pBundle).newMessage(CellappInterface::onDbmgrInitCompleted);
					CellappInterface::onDbmgrInitCompletedArgs6::staticAddToBundle((*pBundle), g_kbetime, idRange.first, 
						idRange.second, cInitInfo.startGlobalOrder, cInitInfo.startGroupOrder, 
						digest);
				}
				break;
			case LOGINAPP_TYPE:
				(*pBundle).newMessage(LoginappInterface::onDbmgrInitCompleted);
				LoginappInterface::onDbmgrInitCompletedArgs3::staticAddToBundle((*pBundle), 
						cInitInfo.startGlobalOrder, cInitInfo.startGroupOrder, 
						digest);

				break;
			default:
				break;
			}

			cinfos->pChannel->send(pBundle);
		}
	}

	apps_.clear();

	delete this;
	return false;
}

//-------------------------------------------------------------------------------------

}
