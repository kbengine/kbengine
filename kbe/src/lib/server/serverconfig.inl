// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
INLINE Network::Address ServerConfig::interfacesAddr(void) const { return interfacesAddr_; }

//-------------------------------------------------------------------------------------	
INLINE std::vector< Network::Address > ServerConfig::interfacesAddrs(void) const { return interfacesAddrs_;}

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
INLINE float ServerConfig::channelExternalTimeout(void) const { return Network::g_channelExternalTimeout; }

//-------------------------------------------------------------------------------------	
INLINE bool ServerConfig::isPureDBInterfaceName(const std::string& dbInterfaceName)
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
