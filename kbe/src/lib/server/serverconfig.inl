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


namespace KBEngine { 

//-------------------------------------------------------------------------------------
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getCellApp(void)
{
	return _cellAppInfo;
}

//-------------------------------------------------------------------------------------
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getBaseApp(void)
{
	return _baseAppInfo;
}

//-------------------------------------------------------------------------------------
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getDBMgr(void)
{
	return _dbmgrInfo;
}

//-------------------------------------------------------------------------------------
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getLoginApp(void)
{
	return _loginAppInfo;
}

//-------------------------------------------------------------------------------------
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getCellAppMgr(void)
{
	return _cellAppMgrInfo;
}

//-------------------------------------------------------------------------------------
ENGINE_COMPONENT_INFO& ServerConfig::getBaseAppMgr(void)
{
	return _baseAppMgrInfo;
}

//-------------------------------------------------------------------------------------		
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getKBMachine(void)
{
	return _kbMachineInfo;
}

//-------------------------------------------------------------------------------------		
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getBots(void)
{
	return _botsInfo;
}

//-------------------------------------------------------------------------------------		
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getLogger(void)
{
	return _loggerInfo;
}

//-------------------------------------------------------------------------------------	
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getConfig()
{
	return getComponent(g_componentType);
}

//-------------------------------------------------------------------------------------	
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getComponent(COMPONENT_TYPE componentType)
{
	switch(componentType)
	{
	case DBMGR_TYPE:
		return getDBMgr();
	case LOGINAPP_TYPE:
		return getLoginApp();
	case BASEAPPMGR_TYPE:
		return getBaseAppMgr();
	case CELLAPPMGR_TYPE:
		return getCellAppMgr();
	case CELLAPP_TYPE:
		return getCellApp();
	case BASEAPP_TYPE:
		return getBaseApp();
	case MACHINE_TYPE:
		return getKBMachine();
	case LOGGER_TYPE:
		return getLogger();
	default:
		return getCellApp();
	};

	return getBaseApp();	
}

//-------------------------------------------------------------------------------------	
INLINE int16 ServerConfig::gameUpdateHertz(void)const { return gameUpdateHertz_;}

//-------------------------------------------------------------------------------------	
INLINE Network::Address ServerConfig::interfacesAddr(void)const { return interfacesAddr_;}

INLINE const char* ServerConfig::interfacesAccountType()const { return interfaces_accountType_.c_str(); }
INLINE const char* ServerConfig::interfacesChargeType()const { return interfaces_chargeType_.c_str(); }

INLINE const char* ServerConfig::interfacesThirdpartyAccountServiceAddr()const { return interfaces_thirdpartyAccountServiceAddr_.c_str(); }
INLINE uint16 ServerConfig::interfacesThirdpartyAccountServicePort()const { return interfaces_thirdpartyAccountServicePort_; }

INLINE const char* ServerConfig::interfacesThirdpartyChargeServiceAddr()const { return interfaces_thirdpartyChargeServiceAddr_.c_str(); }
INLINE uint16 ServerConfig::interfacesThirdpartyChargeServicePort()const { return interfaces_thirdpartyChargeServicePort_; }

INLINE uint16 ServerConfig::interfacesThirdpartyServiceCBPort()const { return interfaces_thirdpartyServiceCBPort_; }

//-------------------------------------------------------------------------------------	

}
