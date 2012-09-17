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
#include "../../server/loginapp/loginapp_interface.hpp"
#include "../../server/resourcemgr/resourcemgr_interface.hpp"
#include "../../server/tools/message_log/messagelog_interface.hpp"


namespace KBEngine
{
	
KBE_SINGLETON_INIT(Components);
Components _g_components;

//-------------------------------------------------------------------------------------
Components::Components():
_baseapps(),
_cellapps(),
_dbmgrs(),
_loginapps(),
_cellappmgrs(),
_baseappmgrs(),
_machines(),
_messagelogs(),
_resourcemgrs(),
_consoles(),
_pNetworkInterface(NULL),
_globalOrderLog(),
_baseappGrouplOrderLog(),
_cellappGrouplOrderLog(),
_loginappGrouplOrderLog()
{
}

//-------------------------------------------------------------------------------------
Components::~Components()
{
}

//-------------------------------------------------------------------------------------
bool Components::checkComponents(int32 uid, COMPONENT_ID componentID)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	return true;
#endif

	if(componentID <= 0)
		return true;

	int idx = 0;

	while(true)
	{
		COMPONENT_TYPE ct = ALL_COMPONENT_TYPES[idx++];
		if(ct == UNKNOWN_COMPONENT_TYPE)
			break;

		ComponentInfos* cinfos = findComponent(ct, uid, componentID);
		if(cinfos != NULL)
		{
			ERROR_MSG("Components::checkComponents: uid:%u, componentType=%s, componentID:%"PRAppID" exist.\n", 
				uid, COMPONENT_NAME[ct],  componentID);

			KBE_ASSERT(false && "Components::checkComponents: componentID exist.\n");
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------		
void Components::addComponent(int32 uid, const char* username, 
			COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
			uint32 intaddr, uint16 intport, 
			uint32 extaddr, uint16 extport, 
			Mercury::Channel* pChannel)
{
	COMPONENTS& components = getComponents(componentType);

	if(!checkComponents(uid, componentID))
		return;

	ComponentInfos* cinfos = findComponent(componentType, uid, componentID);
	if(cinfos != NULL)
	{
		WARNING_MSG("Components::addComponent[%s]: uid:%d, username:%s, "
			"componentType:%d, componentID:%"PRAppID" is exist!\n", 
			COMPONENT_NAME[(uint8)componentType], uid, username, (int32)componentType, componentID);
		return;
	}
	
	ComponentInfos componentInfos;

	componentInfos.pIntAddr.reset(new Mercury::Address(intaddr, intport));
	componentInfos.pExtAddr.reset(new Mercury::Address(extaddr, extport));
	componentInfos.uid = uid;
	componentInfos.cid = componentID;
	componentInfos.pChannel = pChannel;
	componentInfos.componentType = componentType;

	strncpy(componentInfos.username, username, MAX_NAME);

	if(cinfos == NULL)
		components.push_back(componentInfos);
	else
		*cinfos = componentInfos;

	_globalOrderLog[uid]++;

	switch(componentType)
	{
	case BASEAPP_TYPE:
		_baseappGrouplOrderLog[uid]++;
		break;
	case CELLAPP_TYPE:
		_cellappGrouplOrderLog[uid]++;
		break;
	case LOGINAPP_TYPE:
		_loginappGrouplOrderLog[uid]++;
		break;
	default:
		break;
	};

	INFO_MSG("Components::addComponent[%s], uid:%d, "
		"componentID:%"PRAppID", totalcount=%d\n", 
			COMPONENT_NAME[(uint8)componentType], uid, 
			componentID, components.size());
}

//-------------------------------------------------------------------------------------		
void Components::delComponent(int32 uid, COMPONENT_TYPE componentType, 
							  COMPONENT_ID componentID, bool ignoreComponentID, bool shouldShowLog)
{
	COMPONENTS& components = getComponents(componentType);
	COMPONENTS::iterator iter = components.begin();
	for(; iter != components.end();)
	{
		if((*iter).uid == uid && (ignoreComponentID == true || (*iter).cid == componentID))
		{
			INFO_MSG("Components::delComponent[%s] componentID=%" PRAppID ", component:totalcount=%d.\n", 
				COMPONENT_NAME[(uint8)componentType], componentID, components.size());

			//SAFE_RELEASE((*iter).pIntAddr);
			//SAFE_RELEASE((*iter).pExtAddr);
			//(*iter).pChannel->decRef();
			iter = components.erase(iter);

			if(!ignoreComponentID)
				return;
		}
		else
			iter++;
	}

	if(shouldShowLog)
	{
		ERROR_MSG("Components::delComponent::not found [%s] component:totalcount:%d\n", 
			COMPONENT_NAME[(uint8)componentType], components.size());
	}
}

//-------------------------------------------------------------------------------------		
void Components::removeComponentFromChannel(Mercury::Channel * pChannel)
{
	int ifind = 0;
	while(ALL_COMPONENT_TYPES[ifind] != UNKNOWN_COMPONENT_TYPE)
	{
		COMPONENT_TYPE componentType = ALL_COMPONENT_TYPES[ifind++];
		COMPONENTS& components = getComponents(componentType);
		COMPONENTS::iterator iter = components.begin();

		for(; iter != components.end();)
		{
			if((*iter).pChannel == pChannel)
			{
				//SAFE_RELEASE((*iter).pIntAddr);
				//SAFE_RELEASE((*iter).pExtAddr);
				// (*iter).pChannel->decRef();

				WARNING_MSG("Components::removeComponentFromChannel: %s : %"PRAppID".\n", COMPONENT_NAME[componentType], (*iter).cid);
				iter = components.erase(iter);
				return;
			}
			else
				iter++;
		}
	}

	// KBE_ASSERT(false && "channel is not found!\n");
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
		delete pEndpoint;
		return -1;
	}

	pEndpoint->addr(*pComponentInfos->pIntAddr);
	int ret = pEndpoint->connect(pComponentInfos->pIntAddr->port, pComponentInfos->pIntAddr->ip);

	if(ret == 0)
	{
		pComponentInfos->pChannel = new Mercury::Channel(*_pNetworkInterface, pEndpoint, Mercury::Channel::INTERNAL);
		if(!_pNetworkInterface->registerChannel(pComponentInfos->pChannel))
		{
			ERROR_MSG("Components::connectComponent: registerChannel(%s) is failed!\n",
				pComponentInfos->pChannel->c_str());

			delete pComponentInfos->pChannel;
			pComponentInfos->pChannel = NULL;
			return -1;
		}
		else
		{
			Mercury::Bundle bundle(pComponentInfos->pChannel);
			if(componentType == BASEAPPMGR_TYPE)
			{
				bundle.newMessage(BaseappmgrInterface::onRegisterNewApp);
				
				BaseappmgrInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port);
			}
			else if(componentType == CELLAPPMGR_TYPE)
			{
				bundle.newMessage(CellappmgrInterface::onRegisterNewApp);
				
				CellappmgrInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port);
			}
			else if(componentType == CELLAPP_TYPE)
			{
				bundle.newMessage(CellappInterface::onRegisterNewApp);
				
				CellappInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
						_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port);
			}
			else if(componentType == BASEAPP_TYPE)
			{
				bundle.newMessage(BaseappInterface::onRegisterNewApp);
				
				BaseappInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port);
			}
			else if(componentType == DBMGR_TYPE)
			{
				bundle.newMessage(DbmgrInterface::onRegisterNewApp);
				
				DbmgrInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port);
			}
			else if(componentType == MESSAGELOG_TYPE)
			{
				bundle.newMessage(MessagelogInterface::onRegisterNewApp);
				
				MessagelogInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port);
			}
			else if(componentType == RESOURCEMGR_TYPE)
			{
				bundle.newMessage(ResourcemgrInterface::onRegisterNewApp);
				
				ResourcemgrInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), 
					Componentbridge::getSingleton().componentType(), Componentbridge::getSingleton().componentID(), 
					_pNetworkInterface->intaddr().ip, _pNetworkInterface->intaddr().port,
					_pNetworkInterface->extaddr().ip, _pNetworkInterface->extaddr().port);
			}
			else
			{
				KBE_ASSERT(false && "invalid componentType.\n");
			}

			bundle.send(*_pNetworkInterface, pComponentInfos->pChannel);
		}
	}
	else
	{
		ERROR_MSG("Components::connectComponent: connect(%s) is failed! %s.\n",
			pComponentInfos->pIntAddr->c_str(), kbe_strerror());

		return -1;
	}

	return ret;
}

//-------------------------------------------------------------------------------------		
void Components::clear(int32 uid, bool shouldShowLog)
{
	delComponent(uid, DBMGR_TYPE, uid, true, shouldShowLog);
	delComponent(uid, BASEAPPMGR_TYPE, uid, true, shouldShowLog);
	delComponent(uid, CELLAPPMGR_TYPE, uid, true, shouldShowLog);
	delComponent(uid, CELLAPP_TYPE, uid, true, shouldShowLog);
	delComponent(uid, BASEAPP_TYPE, uid, true, shouldShowLog);
	delComponent(uid, LOGINAPP_TYPE, uid, true, shouldShowLog);
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
	case MESSAGELOG_TYPE:
		return _messagelogs;		
	case RESOURCEMGR_TYPE:
		return _resourcemgrs;	
	default:
		break;
	};

	return _consoles;
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
Components::ComponentInfos* Components::findComponent(COMPONENT_ID componentID)
{
	int idx = 0;
	int32 uid = getUserUID();

	while(true)
	{
		COMPONENT_TYPE ct = ALL_COMPONENT_TYPES[idx++];
		if(ct == UNKNOWN_COMPONENT_TYPE)
			break;

		ComponentInfos* cinfos = findComponent(ct, uid, componentID);
		if(cinfos != NULL)
		{
			return cinfos;
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------		
}
