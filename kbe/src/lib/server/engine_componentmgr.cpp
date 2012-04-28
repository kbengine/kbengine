#include "engine_componentmgr.hpp"
#include "helper/debug_helper.hpp"
#include "network/channel.hpp"	

namespace KBEngine
{
	
KBE_SINGLETON_INIT(EngineComponentMgr);

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
void EngineComponentMgr::addComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID, Mercury::Channel* lpChannel)
{
	KBEngine::thread::ThreadGuard tg(&this->myMutex); 
	COMPONENT_MAP& components = getComponents(componentType);
	COMPONENT_MAP::iterator iter = components.find(componentID);
	if(iter != components.end())
	{
		ERROR_MSG("EngineComponentMgr::addComponent: componentID %ld is exist!\n", componentID);
		return;
	}
	
	components[componentID] = lpChannel;
	INFO_MSG("EngineComponentMgr::addComponent[%s], component: id=%ld, totalcount=%d\n", COMPONENT_NAME[(uint8)componentType], 
			componentID, components.size());
}

//-------------------------------------------------------------------------------------		
void EngineComponentMgr::delComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID)
{
	KBEngine::thread::ThreadGuard tg(&this->myMutex); 
	COMPONENT_MAP& components = getComponents(componentType);
	if(components.erase(componentID))
	{
		INFO_MSG("EngineComponentMgr::delComponent[%s] component:totalcount=%d\n", COMPONENT_NAME[(uint8)componentType], components.size());
		return;
	}
	
	ERROR_MSG("EngineComponentMgr::delComponent::not found [%s] component:totalcount:%d\n", COMPONENT_NAME[(uint8)componentType], components.size());
}

//-------------------------------------------------------------------------------------		
EngineComponentMgr::COMPONENT_MAP& EngineComponentMgr::getComponents(COMPONENT_TYPE componentType)
{
	switch(componentType)
	{
	case DBMGR_TYPE:
		return _m_dbmgrs;
	case LOGINAPP_TYPE:
		return _m_loginapps;
	case BASEAPPMGR_TYPE:
		return _m_baseappmgrs;
	case CELLAPPMGR_TYPE:
		return _m_cellappmgrs;
	case CELLAPP_TYPE:
		return _m_cellapps;
	case BASEAPP_TYPE:
		return _m_baseapps;
	case MACHINE_TYPE:
		return _m_machines;
	case CENTER_TYPE:
		return _m_centers;		
	};

	return _m_baseapps;
}

//-------------------------------------------------------------------------------------		
Mercury::Channel* EngineComponentMgr::findComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID)
{
	COMPONENT_MAP& components = getComponents(componentType);
	COMPONENT_MAP::iterator iter = components.find(componentID);
	if(iter != components.end())
		return iter->second;

	return NULL;
}

//-------------------------------------------------------------------------------------		
}
