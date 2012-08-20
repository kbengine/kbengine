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

/*
		ServerConfig::getSingleton().loadConfig("../../res/server/KBEngine.xml");
		ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();													
*/
#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__
#define __LIB_DLLAPI__	
// common include
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <stdarg.h> 
#include "cstdkbe/singleton.hpp"
#include "thread/threadmutex.hpp"
#include "thread/threadguard.hpp"
#include "xmlplus/xmlplus.hpp"	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{
namespace Mercury
{
class Address;
}

// 引擎组件信息结构体
typedef struct EngineComponentInfo
{
	uint32 port;											// 组件的运行后监听的端口
	char ip[MAX_BUF];										// 组件的运行期ip地址

	char entryScriptFile[MAX_NAME];							// 组件的入口脚本文件
	char dbAccountEntityScriptType[MAX_NAME];				// 数据库帐号脚本类别
	float defaultAoIRadius;									// 配置在cellapp节点中的player的aoi半径大小
	float defaultAoIHysteresisArea;							// 配置在cellapp节点中的player的aoi的滞后范围
	const Mercury::Address* externalAddr;					// 外部地址
	const Mercury::Address* internalAddr;					// 内部地址
	COMPONENT_ID componentID;

	char internalInterface[MAX_NAME];						// 内部网卡接口名称
	char externalInterface[MAX_NAME];						// 外部网卡接口名称
	int32 externalPorts_min;								// 对外socket端口使用指定范围
	int32 externalPorts_max;

	char db_type[MAX_BUF];									// 数据库的类别
	uint32 db_port;											// 数据库的端口
	char db_ip[MAX_BUF];									// 数据库的ip地址
	char db_username[MAX_BUF];								// 数据库的用户名
	char db_password[MAX_BUF];								// 数据库的密码
	char db_name[MAX_BUF];									// 数据库名
	uint16 db_numConnections;								// 数据库最大连接
}ENGINE_COMPONENT_INFO;

class ServerConfig : public Singleton<ServerConfig>
{
public:
	ServerConfig();
	~ServerConfig();
	
	bool loadConfig(std::string fileName);
	
	ENGINE_COMPONENT_INFO& getCellApp(void);
	ENGINE_COMPONENT_INFO& getBaseApp(void);
	ENGINE_COMPONENT_INFO& getDBMgr(void);
	ENGINE_COMPONENT_INFO& getLoginApp(void);
	ENGINE_COMPONENT_INFO& getCellAppMgr(void);
	ENGINE_COMPONENT_INFO& getBaseAppMgr(void);
	ENGINE_COMPONENT_INFO& getKBMachine(void);
	ENGINE_COMPONENT_INFO& getKBCenter(void);
	ENGINE_COMPONENT_INFO& getBots(void);

	inline ENGINE_COMPONENT_INFO& getComponent(COMPONENT_TYPE ComponentType);
 	
 	void updateInfos(bool isPrint, COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
 				const Mercury::Address& internalAddr, const Mercury::Address& externalAddr);
 	
	inline int16 gameUpdateHertz(void)const { return gameUpdateHertz_;}
private:
	ENGINE_COMPONENT_INFO _cellAppInfo;
	ENGINE_COMPONENT_INFO _baseAppInfo;
	ENGINE_COMPONENT_INFO _dbmgrInfo;
	ENGINE_COMPONENT_INFO _loginAppInfo;
	ENGINE_COMPONENT_INFO _cellAppMgrInfo;
	ENGINE_COMPONENT_INFO _baseAppMgrInfo;
	ENGINE_COMPONENT_INFO _kbMachineInfo;
	ENGINE_COMPONENT_INFO _kbCenterInfo;
	ENGINE_COMPONENT_INFO _botsInfo;
public:
	int16 gameUpdateHertz_;
};

#define g_kbeSrvConfig ServerConfig::getSingleton()
}
#endif
