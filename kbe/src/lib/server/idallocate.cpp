// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(DbmgrInterface::onReqAllocEntityID);
	DbmgrInterface::onReqAllocEntityIDArgs2::staticAddToBundle((*pBundle), pApp_->componentType(), pApp_->componentID());
	pChannel->send(pBundle);

	setReqServerAllocFlag(true);

	WARNING_MSG(fmt::format("EntityIDClient::onAlloc: not enough({}) entityIDs!\n", id_enough_limit));
}

//-------------------------------------------------------------------------------------


}
