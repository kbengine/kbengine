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
#include "componentbridge.hpp"
#include "helper/debug_helper.hpp"
#include "network/channel.hpp"	
#include "network/address.hpp"	
#include "network/bundle.hpp"	
#include "network/network_interface.hpp"

#include "../../server/baseappmgr/baseappmgr_interface.hpp"
#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine
{
	
KBE_SINGLETON_INIT(Components);
Components _g_components;

//-------------------------------------------------------------------------------------
Components::Components():
pNetworkInterface_(NULL)
{
}

//-------------------------------------------------------------------------------------
Components::~Components()
{
}

//-------------------------------------------------------------------------------------		
void Components::addComponent(int32 uid, const char* username, 
			COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
			uint32 addr, uint16 port, Mercury::Channel* pChannel)
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

	componentInfos.pAddr = new Mercury::Address(addr, port);
	componentInfos.uid = uid;
	componentInfos.cid = componentID;
	componentInfos.pChannel = pChannel;

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
			SAFE_RELEASE((*iter).pAddr);
			SAFE_RELEASE((*iter).pChannel);
			components.erase(iter);
			INFO_MSG("Components::delComponent[%s] componentID=%" PRAppID ", component:totalcount=%d.\n", 
				COMPONENT_NAME[(uint8)componentType], componentID, components.size());

			if(!ignoreComponentID)
				return;
		}
	}

	ERROR_MSG("Components::delComponent::not found [%s] component:totalcount:%d\n", 
		COMPONENT_NAME[(uint8)componentType], components.size());
}

//-------------------------------------------------------------------------------------		
int Components::connectComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID)
{
	Components::ComponentInfos* pComponentInfos = findComponent(componentType, uid, componentID);
	KBE_ASSERT(pComponentInfos != NULL);

	Mercury::EndPoint * pEndpoint = new Mercury::EndPoint;
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("Components::connectComponent: couldn't create a socket\n");
		return -1;
	}

	pEndpoint->addr(*pComponentInfos->pAddr);
	int ret = pEndpoint->connect(pComponentInfos->pAddr->port, pComponentInfos->pAddr->ip);

	if(ret == 0)
	{
		pComponentInfos->pChannel = new Mercury::Channel(*pNetworkInterface_, pEndpoint, Mercury::Channel::INTERNAL);
		if(!pNetworkInterface_->registerChannel(pComponentInfos->pChannel))
		{
			ERROR_MSG("Components::connectComponent: registerChannel(%s) is failed!\n",
				pComponentInfos->pChannel->c_str());
			return -1;
		}
		else
		{
			Mercury::Bundle bundle(pComponentInfos->pChannel);
			if(componentType == BASEAPPMGR_TYPE)
			{
				bundle.newMessage(BaseappmgrInterface::onRegisterNewApp);
				
				BaseappmgrInterface::onRegisterNewAppArgs6::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					pNetworkInterface_->addr().ip, pNetworkInterface_->addr().port);
			}
			else if(componentType == CELLAPPMGR_TYPE)
			{
				bundle.newMessage(CellappmgrInterface::onRegisterNewApp);
				
				CellappmgrInterface::onRegisterNewAppArgs6::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					pNetworkInterface_->addr().ip, pNetworkInterface_->addr().port);
			}
			else if(componentType == CELLAPP_TYPE)
			{
				bundle.newMessage(CellappInterface::onRegisterNewApp);
				
				CellappInterface::onRegisterNewAppArgs6::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					pNetworkInterface_->addr().ip, pNetworkInterface_->addr().port);
			}
			else if(componentType == BASEAPP_TYPE)
			{
				bundle.newMessage(BaseappInterface::onRegisterNewApp);
				
				BaseappInterface::onRegisterNewAppArgs6::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					pNetworkInterface_->addr().ip, pNetworkInterface_->addr().port);
			}
			else if(componentType == DBMGR_TYPE)
			{
				bundle.newMessage(DbmgrInterface::onRegisterNewApp);
				
				DbmgrInterface::onRegisterNewAppArgs6::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					pNetworkInterface_->addr().ip, pNetworkInterface_->addr().port);
			}
			else
			{
				KBE_ASSERT(false && "invalid componentType.\n");
			}

			bundle.send(*pNetworkInterface_, pComponentInfos->pChannel);
		}
	}
	else
	{
			ERROR_MSG("Components::connectComponent: connect(%s) is failed! %s.\n",
				pComponentInfos->pChannel->c_str(), kbe_strerror());
	}

	return ret;
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
