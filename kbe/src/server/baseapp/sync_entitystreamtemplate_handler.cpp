// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "baseapp.h"
#include "sync_entitystreamtemplate_handler.h"
#include "entitydef/scriptdef_module.h"
#include "entitydef/entity_macro.h"
#include "network/fixed_messages.h"
#include "math/math.h"
#include "network/bundle.h"
#include "network/channel.h"

#include "../../server/dbmgr/dbmgr_interface.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
SyncEntityStreamTemplateHandler::SyncEntityStreamTemplateHandler(Network::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface)
{
	networkInterface.dispatcher().addTask(this);

	MemoryStream accountDefMemoryStream;

	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();

	ScriptDefModule* pScriptModule = EntityDef::findScriptModule(dbcfg.dbAccountEntityScriptType);
	if(pScriptModule != NULL)
	{
		ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = pScriptModule->getPersistentPropertyDescriptions();
		ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

		if(pScriptModule->hasCell())
		{
			Vector3 pos, dir;
			ADD_POSDIR_TO_STREAM(accountDefMemoryStream, pos, dir);
		}

		for(; iter != propertyDescrs.end(); ++iter)
		{
			PropertyDescription* propertyDescription = iter->second;

			// 如果某个实体没有cell部分， 而组件属性没有base部分则忽略
			if (!pScriptModule->hasCell())
			{
				if (propertyDescription->getDataType()->type() == DATA_TYPE_ENTITY_COMPONENT && !propertyDescription->hasBase())
					continue;
			}

			accountDefMemoryStream << (ENTITY_PROPERTY_UID)0 << propertyDescription->getUType();
			
			if (propertyDescription->getDataType()->type() == DATA_TYPE_ENTITY_COMPONENT)
				((EntityComponentDescription*)propertyDescription)->addPersistentToStreamTemplates(pScriptModule, &accountDefMemoryStream);
			else
				propertyDescription->addPersistentToStream(&accountDefMemoryStream, NULL);
		}
	}
}

//-------------------------------------------------------------------------------------
SyncEntityStreamTemplateHandler::~SyncEntityStreamTemplateHandler()
{
	// networkInterface_.dispatcher().cancelTask(this);
	DEBUG_MSG("SyncEntityStreamTemplateHandler::~SyncEntityStreamTemplateHandler()\n");
}

//-------------------------------------------------------------------------------------
bool SyncEntityStreamTemplateHandler::process()
{
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Network::Channel* pChannel = NULL;

	if(cts.size() > 0)
	{
		Components::COMPONENTS::iterator ctiter = cts.begin();
		if((*ctiter).pChannel == NULL)
			return true;

		pChannel = (*ctiter).pChannel;
	}

	if(pChannel == NULL)
		return true;

	MemoryStream accountDefMemoryStream;

	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();

	ScriptDefModule* pScriptModule = EntityDef::findScriptModule(dbcfg.dbAccountEntityScriptType);
	if(pScriptModule == NULL)
	{
		delete this;
		return false;
	}

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = pScriptModule->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	if(pScriptModule->hasCell())
	{
		Vector3 pos, dir;
		ADD_POSDIR_TO_STREAM(accountDefMemoryStream, pos, dir);
	}

	for(; iter != propertyDescrs.end(); ++iter)
	{
		PropertyDescription* propertyDescription = iter->second;

		// 如果某个实体没有cell部分， 而组件属性没有base部分则忽略
		if (!pScriptModule->hasCell())
		{
			if (propertyDescription->getDataType()->type() == DATA_TYPE_ENTITY_COMPONENT && !propertyDescription->hasBase())
				continue;
		}

		accountDefMemoryStream << (ENTITY_PROPERTY_UID)0 << propertyDescription->getUType();

		if (propertyDescription->getDataType()->type() == DATA_TYPE_ENTITY_COMPONENT)
			((EntityComponentDescription*)propertyDescription)->addPersistentToStreamTemplates(pScriptModule, &accountDefMemoryStream);
		else
			propertyDescription->addPersistentToStream(&accountDefMemoryStream, NULL);
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	(*pBundle).newMessage(DbmgrInterface::syncEntityStreamTemplate);
	(*pBundle).append(accountDefMemoryStream);
	pChannel->send(pBundle);
	delete this;
	return false;
}

//-------------------------------------------------------------------------------------

}
