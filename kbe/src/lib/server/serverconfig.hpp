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

struct Profiles_Config
{
	Profiles_Config():
		open_pyprofile(false),
		open_cprofile(false),
		open_eventprofile(false),
		open_mercuryprofile(false)
	{
	}

	bool open_pyprofile;
	bool open_cprofile;
	bool open_eventprofile;
	bool open_mercuryprofile;
};

struct ChannelCommon
{
	float channelInternalTimeout;
	float channelExternalTimeout;
	uint32 extReadBufferSize;
	uint32 extWriteBufferSize;
	uint32 intReadBufferSize;
	uint32 intWriteBufferSize;
};

struct EmailServerInfo
{
	std::string smtp_server;
	uint32 smtp_port;
	std::string username;
	std::string password;
	uint8 smtp_auth;
};

struct EmailSendInfo
{
	std::string subject;
	std::string message;
	std::string backlink_success_message, backlink_fail_message, backlink_hello_message;

	uint32 deadline;
};

// 引擎组件信息结构体
typedef struct EngineComponentInfo
{
	EngineComponentInfo()
	{
		tcp_SOMAXCONN = 5;
		notFoundAccountAutoCreate = false;
		use_coordinate_system = true;
		account_type = 3;
	}

	~EngineComponentInfo()
	{
	}

	uint32 port;											// 组件的运行后监听的端口
	char ip[MAX_BUF];										// 组件的运行期ip地址

	char entryScriptFile[MAX_NAME];							// 组件的入口脚本文件
	char dbAccountEntityScriptType[MAX_NAME];				// 数据库帐号脚本类别
	float defaultAoIRadius;									// 配置在cellapp节点中的player的aoi半径大小
	float defaultAoIHysteresisArea;							// 配置在cellapp节点中的player的aoi的滞后范围
	uint16 witness_timeout;									// 观察者默认超时时间(秒)
	const Mercury::Address* externalAddr;					// 外部地址
	const Mercury::Address* internalAddr;					// 内部地址
	COMPONENT_ID componentID;

	float ghostDistance;									// ghost区域距离
	uint16 ghostingMaxPerCheck;								// 每秒检查ghost次数
	uint16 ghostUpdateHertz;								// ghost更新hz
	
	bool use_coordinate_system;								// 是否使用坐标系统 如果为false， aoi,trap, move等功能将不再维护
	bool rangelist_hasY;									// 范围管理器是管理Y轴， 注：有y轴则aoi、trap等功能有了高度， 但y轴的管理会带来一定的消耗

	bool aliasEntityID;										// 优化EntityID，aoi范围内小于255个EntityID, 传输到client时使用1字节伪ID 

	char internalInterface[MAX_NAME];						// 内部网卡接口名称
	char externalInterface[MAX_NAME];						// 外部网卡接口名称
	int32 externalPorts_min;								// 对外socket端口使用指定范围
	int32 externalPorts_max;

	char db_type[MAX_BUF];									// 数据库的类别
	uint32 db_port;											// 数据库的端口
	char db_ip[MAX_BUF];									// 数据库的ip地址
	char db_username[MAX_NAME];								// 数据库的用户名
	char db_password[MAX_BUF * 10];							// 数据库的密码
	char db_name[MAX_NAME];									// 数据库名
	uint16 db_numConnections;								// 数据库最大连接
	std::string db_unicodeString_characterSet;				// 设置数据库字符集
	std::string db_unicodeString_collation;
	bool notFoundAccountAutoCreate;							// 登录合法时游戏数据库找不到游戏账号则自动创建
	bool db_passwordEncrypt;								// db密码是否是加密的
	bool allowEmptyDigest;

	float archivePeriod;									// entity存储数据库周期
	float backupPeriod;										// entity备份周期
	bool backUpUndefinedProperties;							// entity是否备份未定义属性
	uint16 entityRestoreSize;								// entity restore每tick数量 

	float loadSmoothingBias;								// baseapp负载滤平衡调整值， 
	uint32 login_port;										// 服务器登录端口 目前bots在用
	char login_ip[MAX_BUF];									// 服务器登录ip地址

	ENTITY_ID criticallyLowSize;							// id剩余这么多个时向dbmgr申请新的id资源

	uint32 downloadBitsPerSecondTotal;						// 所有客户端每秒下载带宽总上限
	uint32 downloadBitsPerSecondPerClient;					// 每个客户端每秒的下载带宽

	Profiles_Config profiles;

	uint32 defaultAddBots_totalCount;						// 默认启动进程后自动添加这么多个bots 添加总数量
	float defaultAddBots_tickTime;							// 默认启动进程后自动添加这么多个bots 每次添加所用时间(s)
	uint32 defaultAddBots_tickCount;						// 默认启动进程后自动添加这么多个bots 每次添加数量

	uint32 tcp_SOMAXCONN;									// listen监听队列最大值

	int8 encrypt_login;										// 加密登录信息

	uint32 telnet_port;
	std::string telnet_passwd;
	std::string telnet_deflayer;

	uint32 perSecsDestroyEntitySize;						// 每秒销毁base|entity数量

	uint64 respool_timeout;
	uint32 respool_buffersize;

	uint8 account_type;										// 1: 普通账号, 2: email账号(需要激活), 3: 智能账号(自动识别email， 普通号码等) 
	uint32 accountDefaultFlags;								// 新账号默认标记(ACCOUNT_FLAGS可叠加， 填写时按十进制格式) 
	uint64 accountDefaultDeadline;							// 新账号默认过期时间(秒, 引擎会加上当前时间)
	
	std::string http_cbhost;
	uint16 http_cbport;										// 用户http回调接口，处理认证、密码重置等
}ENGINE_COMPONENT_INFO;

class ServerConfig : public Singleton<ServerConfig>
{
public:
	ServerConfig();
	~ServerConfig();
	
	bool loadConfig(std::string fileName);
	
	INLINE ENGINE_COMPONENT_INFO& getCellApp(void);
	INLINE ENGINE_COMPONENT_INFO& getBaseApp(void);
	INLINE ENGINE_COMPONENT_INFO& getDBMgr(void);
	INLINE ENGINE_COMPONENT_INFO& getLoginApp(void);
	INLINE ENGINE_COMPONENT_INFO& getCellAppMgr(void);
	INLINE ENGINE_COMPONENT_INFO& getBaseAppMgr(void);
	INLINE ENGINE_COMPONENT_INFO& getKBMachine(void);
	INLINE ENGINE_COMPONENT_INFO& getKBCenter(void);
	INLINE ENGINE_COMPONENT_INFO& getBots(void);
	INLINE ENGINE_COMPONENT_INFO& getResourcemgr(void);
	INLINE ENGINE_COMPONENT_INFO& getMessagelog(void);
	INLINE ENGINE_COMPONENT_INFO& getBilling(void);

	INLINE ENGINE_COMPONENT_INFO& getComponent(COMPONENT_TYPE componentType);
 	
 	void updateInfos(bool isPrint, COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
 				const Mercury::Address& internalAddr, const Mercury::Address& externalAddr);
 	
	INLINE int16 gameUpdateHertz(void)const;
	INLINE Mercury::Address billingSystemAddr(void)const;
	
	INLINE const char* billingSystemAccountType()const;
	INLINE const char* billingSystemChargeType()const;

	INLINE const char* billingSystemThirdpartyAccountServiceAddr()const;
	INLINE uint16 billingSystemThirdpartyAccountServicePort()const;

	INLINE const char* billingSystemThirdpartyChargeServiceAddr()const;
	INLINE uint16 billingSystemThirdpartyChargeServicePort()const;

	INLINE uint16 billingSystemThirdpartyServiceCBPort()const;

	const ChannelCommon& channelCommon(){ return channelCommon_; }

	uint32 tcp_SOMAXCONN(COMPONENT_TYPE componentType);

	float shutdowntime(){ return shutdown_time_; }
	float shutdownWaitTickTime(){ return shutdown_waitTickTime_; }

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
	ENGINE_COMPONENT_INFO _resourcemgrInfo;
	ENGINE_COMPONENT_INFO _messagelogInfo;
	ENGINE_COMPONENT_INFO _billingInfo;
public:
	int16 gameUpdateHertz_;

	ChannelCommon channelCommon_;

	// 每个客户端每秒占用的最大带宽
	uint32 bitsPerSecondToClient_;		

	Mercury::Address billingSystemAddr_;
	std::string billingSystem_accountType_;							// 计费系统类别
	std::string billingSystem_chargeType_;							// 计费系统类别
	std::string billingSystem_thirdpartyAccountServiceAddr_;		// 第三方运营账号服务地址(当type是thirdparty时有效)
	uint16	billingSystem_thirdpartyAccountServicePort_;			
	std::string billingSystem_thirdpartyChargeServiceAddr_;			// 第三方运营充值服务地址(当type是thirdparty时有效)
	uint16 billingSystem_thirdpartyChargeServicePort_;	
	uint16 billingSystem_thirdpartyServiceCBPort_;	
	uint32 billingSystem_orders_timeout_;

	float shutdown_time_;
	float shutdown_waitTickTime_;

	float callback_timeout_;										// callback默认超时时间(秒)
	float thread_timeout_;											// 默认超时时间(秒)

	uint32 thread_init_create_, thread_pre_create_, thread_max_create_;
	
	EmailServerInfo	emailServerInfo_;
	EmailSendInfo emailAtivationInfo_;
	EmailSendInfo emailResetPasswordInfo_;
	EmailSendInfo emailBindInfo_;

};

#define g_kbeSrvConfig ServerConfig::getSingleton()
}


#ifdef CODE_INLINE
#include "serverconfig.ipp"
#endif
#endif
