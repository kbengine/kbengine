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


#include "components.hpp"
#include "helper/debug_helper.hpp"
#include "network/channel.hpp"	

namespace KBEngine
{
	
KBE_SINGLETON_INIT(Components);
Components _g_components;

//-------------------------------------------------------------------------------------
Components::Components()
{
}

//-------------------------------------------------------------------------------------
Components::~Components()
{
}

//-------------------------------------------------------------------------------------		
void Components::addComponent(int32 uid, const char* username, 
			COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
			uint32 addr, uint16 port)
{
	KBEngine::thread::ThreadGuard tg(&this->myMutex); 
	COMPONENTS& components = getComponents(componentType);

	ComponentInfos* cinfos = findComponent(componentType, uid, componentID);
	if(cinfos != NULL)
	{
		WARNING_MSG("Components::addComponent: uid:%d, username:%s, "
			"componentType:%d, componentID:%"PRAppID" is exist!\n", 
			uid, username, (int32)componentType, componentID);
		return;
	}
	
	ComponentInfos componentInfos;
	componentInfos.uid = uid;
	componentInfos.addr = addr;
	componentInfos.port = port;
	componentInfos.cid = componentID;
	
	strncpy(componentInfos.username, username, MAX_NAME);

	if(cinfos == NULL)
		components.push_back(componentInfos);
	else
		*cinfos = componentInfos;

	INFO_MSG("Components::addComponent[%s], uid:%d, "
		"componentID:%"PRAppID", totalcount=%d\n", 
			COMPONENT_NAME[(uint8)componentType], uid, 
			componentID, components.size());
}

//-------------------------------------------------------------------------------------		
void Components::delComponent(int32 uid, COMPONENT_TYPE componentType, COMPONENT_ID componentID, bool ignoreComponentID)
{
	KBEngine::thread::ThreadGuard tg(&this->myMutex); 

	COMPONENTS& components = getComponents(componentType);
	COMPONENTS::iterator iter = components.begin();
	for(; iter != components.end(); iter++)
	{
		if((*iter).uid == uid && (ignoreComponentID == true || (*iter).cid == componentID))
		{
			components.erase(iter);
			INFO_MSG("Components::delComponent[%s] componentID=%" PRAppID ", component:totalcount=%d.\n", 
				COMPONENT_NAME[(uint8)componentType], componentID, components.size());
			return;
		}
	}

	ERROR_MSG("Components::delComponent::not found [%s] component:totalcount:%d\n", 
		COMPONENT_NAME[(uint8)componentType], components.size());
}

//-------------------------------------------------------------------------------------		
void Components::clear(int32 uid)
{
	KBEngine::thread::ThreadGuard tg(&this->myMutex); 

	delComponent(uid, DBMGR_TYPE, 0, true);
	delComponent(uid, BASEAPPMGR_TYPE, 0, true);
	delComponent(uid, CELLAPPMGR_TYPE, 0, true);
	delComponent(uid, CELLAPP_TYPE, 0, true);
	delComponent(uid, BASEAPP_TYPE, 0, true);
}

//-------------------------------------------------------------------------------------		
Components::COMPONENTS& Components::getComponents(COMPONENT_TYPE componentType)
{
	switch(componentType)
	{
	case DBMGR_TYPE:
		return _dbmgrs;
	case LOGINAPP_TYPE:
		return _loginapps;
	case BASEAPPMGR_TYPE:
		return _baseappmgrs;
	case CELLAPPMGR_TYPE:
		return _cellappmgrs;
	case CELLAPP_TYPE:
		return _cellapps;
	case BASEAPP_TYPE:
		return _baseapps;
	case MACHINE_TYPE:
		return _machines;
	case CENTER_TYPE:
		return _centers;		
	default:
		break;
	};

	return _baseapps;
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::findComponent(COMPONENT_TYPE componentType, int32 uid,
																			COMPONENT_ID componentID)
{
	COMPONENTS& components = getComponents(componentType);
	COMPONENTS::iterator iter = components.begin();
	for(; iter != components.end(); iter++)
	{
		if((*iter).uid == uid && (componentID == 0 || (*iter).cid == componentID))
			return &(*iter);
	}

	return NULL;
}

//-------------------------------------------------------------------------------------		
Components::ComponentInfos* Components::findComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID)
{
	COMPONENTS& components = getComponents(componentType);
	COMPONENTS::iterator iter = components.begin();
	for(; iter != components.end(); iter++)
	{
		if(componentID == 0 || (*iter).cid == componentID)
			return &(*iter);
	}

	return NULL;
}

//-------------------------------------------------------------------------------------		
}
