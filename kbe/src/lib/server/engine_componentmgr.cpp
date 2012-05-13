#include "engine_componentmgr.hpp"
#include "helper/debug_helper.hpp"
#include "network/channel.hpp"	

namespace KBEngine
{
	
KBE_SINGLETON_INIT(EngineComponentMgr);
EngineComponentMgr _g_engineComponentMgr;

//-------------------------------------------------------------------------------------
EngineComponentMgr::EngineComponentMgr()
{
}

//-------------------------------------------------------------------------------------
EngineComponentMgr::~EngineComponentMgr()
{
}

//-------------------------------------------------------------------------------------
uint32 EngineComponentMgr::allocComponentID(void)
{
	static COMPONENT_ID componentID = 1;
	return componentID++;
}

//-------------------------------------------------------------------------------------		
void EngineComponentMgr::addComponent(int32 uid, const char* username, 
			COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
			Mercury::Channel* lpChannel)
{
	KBEngine::thread::ThreadGuard tg(&this->myMutex); 
	COMPONENT_MAP& components = getComponents(componentType);
	COMPONENT_MAP::iterator iter = components.find(componentID);
	if(iter != components.end())
	{
		ERROR_MSG("EngineComponentMgr::addComponent: uid:%d, username:%s, "
			"componentType:%d, componentID:%ld is exist!\n", 
			uid, username, (int32)componentType, componentID);
		return;
	}
	
	ComponentInfos componentInfos;
	componentInfos.pChannel = lpChannel;
	componentInfos.uid = uid;
	strncpy(componentInfos.username, username, 256);
	components[componentID] = componentInfos;

	INFO_MSG("EngineComponentMgr::addComponent[%s], uid:%d, "
		"componentID:%ld, totalcount=%d\n", 
			uid, COMPONENT_NAME[(uint8)componentType], 
			componentID, components.size());
}

//-------------------------------------------------------------------------------------		
void EngineComponentMgr::delComponent(int32 uid, COMPONENT_TYPE componentType, COMPONENT_ID componentID)
{
	KBEngine::thread::ThreadGuard tg(&this->myMutex); 
	COMPONENT_MAP& components = getComponents(componentType);
	if(components.erase(componentID))
	{
		INFO_MSG("EngineComponentMgr::delComponent[%s] component:totalcount=%d\n", 
			COMPONENT_NAME[(uint8)componentType], components.size());
		return;
	}
	
	ERROR_MSG("EngineComponentMgr::delComponent::not found [%s] component:totalcount:%d\n", 
		COMPONENT_NAME[(uint8)componentType], components.size());
}

//-------------------------------------------------------------------------------------		
EngineComponentMgr::COMPONENT_MAP& EngineComponentMgr::getComponents(COMPONENT_TYPE componentType)
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
const EngineComponentMgr::ComponentInfos* EngineComponentMgr::findComponent(COMPONENT_TYPE componentType, 
																			COMPONENT_ID componentID)
{
	COMPONENT_MAP& components = getComponents(componentType);
	COMPONENT_MAP::iterator iter = components.find(componentID);
	if(iter != components.end())
		return &iter->second;

	return NULL;
}

//-------------------------------------------------------------------------------------		
}
