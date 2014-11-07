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

#include "baseapp.hpp"
#include "sync_entitystreamtemplate_handler.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "entitydef/entity_macro.hpp"
#include "network/fixed_messages.hpp"
#include "math/math.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"

#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
SyncEntityStreamTemplateHandler::SyncEntityStreamTemplateHandler(Mercury::NetworkInterface & networkInterface):
Task(),
networkInterface_(networkInterface)
{
	networkInterface.dispatcher().addFrequentTask(this);

	MemoryStream accountDefMemoryStream;

	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();

	ScriptDefModule* scriptModule = EntityDef::findScriptModule(dbcfg.dbAccountEntityScriptType);
	if(scriptModule != NULL)
	{
		ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule->getPersistentPropertyDescriptions();
		ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

		if(scriptModule->hasCell())
		{
			Vector3 pos, dir;
			ADD_POSDIR_TO_STREAM(accountDefMemoryStream, pos, dir);
		}

		for(; iter != propertyDescrs.end(); iter++)
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
	// networkInterface_.mainDispatcher().cancelFrequentTask(this);
	DEBUG_MSG("SyncEntityStreamTemplateHandler::~SyncEntityStreamTemplateHandler()\n");
}

//-------------------------------------------------------------------------------------
bool SyncEntityStreamTemplateHandler::process()
{
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Mercury::Channel* pChannel = NULL;

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

	ScriptDefModule* scriptModule = EntityDef::findScriptModule(dbcfg.dbAccountEntityScriptType);
	if(scriptModule == NULL)
	{
		delete this;
		return false;
	}

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	if(scriptModule->hasCell())
	{
		Vector3 pos, dir;
		ADD_POSDIR_TO_STREAM(accountDefMemoryStream, pos, dir);
	}

	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;
		accountDefMemoryStream << propertyDescription->getUType();
		propertyDescription->addPersistentToStream(&accountDefMemoryStream, NULL);
	}

	Mercury::Bundle::SmartPoolObjectPtr bundleptr = Mercury::Bundle::createSmartPoolObj();

	(*bundleptr)->newMessage(DbmgrInterface::syncEntityStreamTemplate);
	(*bundleptr)->append(accountDefMemoryStream);
	(*bundleptr)->send(networkInterface_, pChannel);
	delete this;
	return false;
}

//-------------------------------------------------------------------------------------

}
