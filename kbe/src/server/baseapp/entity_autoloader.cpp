/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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
#include "entity_autoloader.h"
#include "initprogress_handler.h"
#include "network/bundle.h"
#include "network/channel.h"
#include "entitydef/entitydef.h"

#include "../../server/dbmgr/dbmgr_interface.h"

namespace KBEngine{	

#define LOAD_ENTITY_SIZE 32

//-------------------------------------------------------------------------------------
EntityAutoLoader::EntityAutoLoader(Network::NetworkInterface & networkInterface, InitProgressHandler* pInitProgressHandler):
networkInterface_(networkInterface),
pInitProgressHandler_(pInitProgressHandler),
entityTypes_(),
start_(0),
end_(0),
querying_(false)
{
	ENGINE_COMPONENT_INFO& dbcfg = g_kbeSrvConfig.getDBMgr();

	std::vector<DBInterfaceInfo>::iterator dbinfo_iter = dbcfg.dbInterfaceInfos.begin();
	for (; dbinfo_iter != dbcfg.dbInterfaceInfos.end(); ++dbinfo_iter)
	{
		entityTypes_.push_back(std::vector<ENTITY_SCRIPT_UID>());

		const EntityDef::SCRIPT_MODULES& modules = EntityDef::getScriptModules();
		EntityDef::SCRIPT_MODULES::const_iterator iter = modules.begin();
		for (; iter != modules.end(); ++iter)
		{
			entityTypes_[entityTypes_.size() - 1].push_back((*iter)->getUType());
		}
	}
}

//-------------------------------------------------------------------------------------
EntityAutoLoader::~EntityAutoLoader()
{
	DEBUG_MSG("EntityAutoLoader::~EntityAutoLoader()\n");

	if(pInitProgressHandler_)
		pInitProgressHandler_->setAutoLoadState(1);
}

//-------------------------------------------------------------------------------------
void EntityAutoLoader::onEntityAutoLoadCBFromDBMgr(Network::Channel* pChannel, MemoryStream& s)
{
	uint16 dbInterfaceIndex = 0;
	s >> dbInterfaceIndex;

	int size = 0;
	s >> size;

	if(size >= LOAD_ENTITY_SIZE)
	{
		start_ = end_;
		end_ += LOAD_ENTITY_SIZE;
	}
	else
	{
		start_ = 0;
		end_ = 0;

		(*entityTypes_.begin()).erase((*entityTypes_.begin()).begin());
	}

	querying_ = false;

	ENTITY_SCRIPT_UID entityType;
	s >> entityType;

	for(int i=0; i<size; ++i)
	{
		DBID dbid;
		s >> dbid;

		if(PyObject_HasAttrString(Baseapp::getSingleton().getEntryScript().get(), "onAutoLoadEntityCreate") > 0)
		{
			PyObject* pyResult = PyObject_CallMethod(Baseapp::getSingleton().getEntryScript().get(), 
												const_cast<char*>("onAutoLoadEntityCreate"), 
												const_cast<char*>("sK"), 
												EntityDef::findScriptModule(entityType)->getName(),
												dbid);

			if(pyResult != NULL)
			{
				Py_DECREF(pyResult);
			}
			else
			{
				SCRIPT_ERROR_CHECK();

				if(pInitProgressHandler_)
					pInitProgressHandler_->setError();
			}
		}
		else
		{
			Baseapp::getSingleton().createBaseAnywhereFromDBID(EntityDef::findScriptModule(entityType)->getName(), dbid, NULL, 
				g_kbeSrvConfig.dbInterfaceIndex2dbInterfaceName(dbInterfaceIndex));
		}
	}
}

//-------------------------------------------------------------------------------------
bool EntityAutoLoader::process()
{
	Network::Channel* pChannel = Components::getSingleton().getDbmgrChannel();
	if(pChannel == NULL || querying_)
		return true;

	if(entityTypes_.size() > 0)
	{
		if ((*entityTypes_.begin()).size() > 0)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject();

			if (start_ == 0 && end_ == 0)
				end_ = LOAD_ENTITY_SIZE;

			uint16 dbInterfaceIndex = (uint16)(g_kbeSrvConfig.getDBMgr().dbInterfaceInfos.size() - entityTypes_.size());
			(*pBundle).newMessage(DbmgrInterface::entityAutoLoad);
			(*pBundle) << dbInterfaceIndex << g_componentID << (*(*entityTypes_.begin()).begin()) << start_ << end_;
			pChannel->send(pBundle);
			querying_ = true;
		}
		else
		{
			entityTypes_.erase(entityTypes_.begin());
		}

		return true;
	}

	delete this;
	return false;
}

//-------------------------------------------------------------------------------------

}
