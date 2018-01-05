/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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
INLINE ENGINE_COMPONENT_INFO& ServerConfig::getInterfaces(void)
{
	return _interfacesInfo;
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
INLINE int16 ServerConfig::gameUpdateHertz(void) const { return gameUpdateHertz_;}

//-------------------------------------------------------------------------------------	
INLINE Network::Address ServerConfig::interfacesAddr(void) const { return interfacesAddr_;}

//-------------------------------------------------------------------------------------	
INLINE DBInterfaceInfo* ServerConfig::dbInterface(const std::string& name)
{
	std::vector<DBInterfaceInfo>::iterator dbinfo_iter = _dbmgrInfo.dbInterfaceInfos.begin();
	for (; dbinfo_iter != _dbmgrInfo.dbInterfaceInfos.end(); ++dbinfo_iter)
	{
		if (name == (*dbinfo_iter).name)
		{
			return &(*dbinfo_iter);
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------	
INLINE bool ServerConfig::IsPureDBInterfaceName(const std::string& dbInterfaceName)
{
	for (size_t i = 0; i < _dbmgrInfo.dbInterfaceInfos.size(); ++i)
	{
		if (_dbmgrInfo.dbInterfaceInfos[i].name == dbInterfaceName)
		{
			return _dbmgrInfo.dbInterfaceInfos[i].isPure;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------	
INLINE int ServerConfig::dbInterfaceName2dbInterfaceIndex(const std::string& dbInterfaceName)
{
	for (size_t i = 0; i < _dbmgrInfo.dbInterfaceInfos.size(); ++i)
	{
		if (_dbmgrInfo.dbInterfaceInfos[i].name == dbInterfaceName)
		{
			return (int)i;
		}
	}

	return -1;
}

//-------------------------------------------------------------------------------------	
INLINE const char* ServerConfig::dbInterfaceIndex2dbInterfaceName(size_t dbInterfaceIndex)
{
	if (_dbmgrInfo.dbInterfaceInfos.size() > dbInterfaceIndex)
	{
		return _dbmgrInfo.dbInterfaceInfos[dbInterfaceIndex].name;
	}

	return "";
}

//-------------------------------------------------------------------------------------	

}
