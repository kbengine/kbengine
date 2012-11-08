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
	networkInterface.mainDispatcher().addFrequentTask(this);

	MemoryStream accountDefMemoryStream;

	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();

	ScriptDefModule* scriptModule = EntityDef::findScriptModule(dbcfg.dbAccountEntityScriptType);
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

//-------------------------------------------------------------------------------------
SyncEntityStreamTemplateHandler::~SyncEntityStreamTemplateHandler()
{
	// networkInterface_.mainDispatcher().cancelFrequentTask(this);
	DEBUG_MSG("SyncEntityStreamTemplateHandler::~SyncEntityStreamTemplateHandler()\n");
}

//-------------------------------------------------------------------------------------
bool SyncEntityStreamTemplateHandler::process()
{
	Components::COMPONENTS cts = Components::getSingleton().getComponents(DBMGR_TYPE);
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
