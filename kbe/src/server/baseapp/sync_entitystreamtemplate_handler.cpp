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
			accountDefMemoryStream << propertyDescription->getUType();
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
		accountDefMemoryStream << propertyDescription->getUType();
		propertyDescription->addPersistentToStream(&accountDefMemoryStream, NULL);
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();

	(*pBundle).newMessage(DbmgrInterface::syncEntityStreamTemplate);
	(*pBundle).append(accountDefMemoryStream);
	pChannel->send(pBundle);
	delete this;
	return false;
}

//-------------------------------------------------------------------------------------

}
