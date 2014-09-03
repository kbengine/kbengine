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
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getMessagelog(void)
{
	return _messagelogInfo;
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
	case MESSAGELOG_TYPE:
		return getMessagelog();
	default:
		return getCellApp();
	};

	return getBaseApp();	
}

//-------------------------------------------------------------------------------------	
INLINE int16 ServerConfig::gameUpdateHertz(void)const { return gameUpdateHertz_;}

//-------------------------------------------------------------------------------------	
INLINE Mercury::Address ServerConfig::billingSystemAddr(void)const { return billingSystemAddr_;}

INLINE const char* ServerConfig::billingSystemAccountType()const { return billingSystem_accountType_.c_str(); }
INLINE const char* ServerConfig::billingSystemChargeType()const { return billingSystem_chargeType_.c_str(); }

INLINE const char* ServerConfig::billingSystemThirdpartyAccountServiceAddr()const { return billingSystem_thirdpartyAccountServiceAddr_.c_str(); }
INLINE uint16 ServerConfig::billingSystemThirdpartyAccountServicePort()const { return billingSystem_thirdpartyAccountServicePort_; }

INLINE const char* ServerConfig::billingSystemThirdpartyChargeServiceAddr()const { return billingSystem_thirdpartyChargeServiceAddr_.c_str(); }
INLINE uint16 ServerConfig::billingSystemThirdpartyChargeServicePort()const { return billingSystem_thirdpartyChargeServicePort_; }

INLINE uint16 ServerConfig::billingSystemThirdpartyServiceCBPort()const { return billingSystem_thirdpartyServiceCBPort_; }

//-------------------------------------------------------------------------------------	

}
