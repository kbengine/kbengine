// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

/*
		ServerConfig::getSingleton().loadConfig("../../res/server/KBEngine.xml");
		ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();													
*/
#ifndef KBE_SERVER_CONFIG_H
#define KBE_SERVER_CONFIG_H

#define __LIB_DLLAPI__	

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <stdarg.h> 
#include "common/singleton.h"
#include "thread/threadmutex.h"
#include "thread/threadguard.h"
#include "xml/xml.h"	

	
namespace KBEngine{
namespace Network
{
class Address;
}

struct Profiles_Config
{
	Profiles_Config():
		open_pyprofile(false),
		open_cprofile(false),
		open_eventprofile(false),
		open_networkprofile(false)
	{
	}

	bool open_pyprofile;
	bool open_cprofile;
	bool open_eventprofile;
	bool open_networkprofile;
};

struct ChannelCommon
{
	float channelInternalTimeout;
	float channelExternalTimeout;
	uint32 extReadBufferSize;
	uint32 extWriteBufferSize;
	uint32 intReadBufferSize;
	uint32 intWriteBufferSize;
	uint32 intReSendInterval;
	uint32 intReSendRetries;
	uint32 extReSendInterval;
	uint32 extReSendRetries;
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

struct DBInterfaceInfo
{
	DBInterfaceInfo()
	{
		index = 0;
		isPure = false;
		db_numConnections = 5;
		db_passwordEncrypt = true;

		memset(name, 0, sizeof(name));
		memset(db_type, 0, sizeof(db_type));
		memset(db_ip, 0, sizeof(db_ip));
		memset(db_username, 0, sizeof(db_username));
		memset(db_password, 0, sizeof(db_password));
		memset(db_name, 0, sizeof(db_name));
	}

	int index;
	bool isPure;											// 是否为纯净库（没有引擎创建的实体表）
	char name[MAX_BUF];										// 数据库的接口名称
	char db_type[MAX_BUF];									// 数据库的类别
	uint32 db_port;											// 数据库的端口
	char db_ip[MAX_BUF];									// 数据库的ip地址
	char db_username[MAX_NAME];								// 数据库的用户名
	char db_password[MAX_BUF * 10];							// 数据库的密码
	bool db_passwordEncrypt;								// db密码是否是加密的
	char db_name[MAX_NAME];									// 数据库名
	uint16 db_numConnections;								// 数据库最大连接
	std::string db_unicodeString_characterSet;				// 设置数据库字符集
	std::string db_unicodeString_collation;
};

// 引擎组件信息结构体
typedef struct EngineComponentInfo
{
	EngineComponentInfo()
	{
		tcp_SOMAXCONN = 5;
		notFoundAccountAutoCreate = false;
		account_registration_enable = false;
		account_reset_password_enable = false;
		use_coordinate_system = true;
		account_type = 3;
		debugDBMgr = false;

		externalAddress[0] = '\0';

		isOnInitCallPropertysSetMethods = true;
		forceInternalLogin = false;
	}

	~EngineComponentInfo()
	{
	}

	uint32 port;											// 组件的运行后监听的端口
	char ip[MAX_BUF];										// 组件的运行期ip地址

	std::vector< std::string > machine_addresses;			// 配置中给出的所有的machine的地址
	
	char entryScriptFile[MAX_NAME];							// 组件的入口脚本文件
	char dbAccountEntityScriptType[MAX_NAME];				// 数据库帐号脚本类别
	float defaultViewRadius;								// 配置在cellapp节点中的player的view半径大小
	float defaultViewHysteresisArea;						// 配置在cellapp节点中的player的view的滞后范围
	uint16 witness_timeout;									// 观察者默认超时时间(秒)
	const Network::Address* externalTcpAddr;				// 外部地址
	const Network::Address* externalUdpAddr;				// 外部地址
	const Network::Address* internalTcpAddr;				// 内部地址
	COMPONENT_ID componentID;

	float ghostDistance;									// ghost区域距离
	uint16 ghostingMaxPerCheck;								// 每秒检查ghost次数
	uint16 ghostUpdateHertz;								// ghost更新hz
	
	bool use_coordinate_system;								// 是否使用坐标系统 如果为false, view, trap, move等功能将不再维护
	bool coordinateSystem_hasY;								// 范围管理器是管理Y轴， 注：有y轴则view、trap等功能有了高度， 但y轴的管理会带来一定的消耗
	uint16 entity_posdir_additional_updates;				// 实体位置停止发生改变后，引擎继续向客户端更新tick次的位置信息，为0则总是更新。
	uint16 entity_posdir_updates_type;						// 实体位置更新方式，0：非优化高精度同步, 1:优化同步, 2:智能选择模式
	uint16 entity_posdir_updates_smart_threshold;			// 实体位置更新智能模式下的同屏人数阈值

	bool aliasEntityID;										// 优化EntityID，view范围内小于255个EntityID, 传输到client时使用1字节伪ID 
	bool entitydefAliasID;									// 优化entity属性和方法广播时占用的带宽，entity客户端属性或者客户端不超过255个时， 方法uid和属性uid传输到client时使用1字节别名ID

	char internalInterface[MAX_NAME];						// 内部网卡接口名称
	char externalInterface[MAX_NAME];						// 外部网卡接口名称
	char externalAddress[MAX_NAME];							// 外部IP地址

	int32 externalTcpPorts_min;								// 对外socket TCP端口使用指定范围
	int32 externalTcpPorts_max;

	int32 externalUdpPorts_min;								// 对外socket UDP端口使用指定范围
	int32 externalUdpPorts_max;

	std::vector<DBInterfaceInfo> dbInterfaceInfos;			// 数据库接口
	bool notFoundAccountAutoCreate;							// 登录合法时游戏数据库找不到游戏账号则自动创建
	bool allowEmptyDigest;									// 是否检查defs-MD5
	bool account_registration_enable;						// 是否开放注册
	bool account_reset_password_enable;						// 是否开放重设密码功能
	bool isShareDB;											// 是否共享数据库

	float archivePeriod;									// entity存储数据库周期
	float backupPeriod;										// entity备份周期
	bool backUpUndefinedProperties;							// entity是否备份未定义属性
	uint16 entityRestoreSize;								// entity restore每tick数量 

	float loadSmoothingBias;								// baseapp负载滤平衡调整值， 
	uint32 login_port;										// 服务器登录端口 目前bots在用
	uint32 login_port_min;									// 服务器登录端口使用指定范围 目前bots在用
	uint32 login_port_max;
	char login_ip[MAX_BUF];									// 服务器登录ip地址

	ENTITY_ID ids_criticallyLowSize;						// id剩余这么多个时向dbmgr申请新的id资源
	ENTITY_ID ids_increasing_range;							// 申请ID时id每次递增范围

	uint32 downloadBitsPerSecondTotal;						// 所有客户端每秒下载带宽总上限
	uint32 downloadBitsPerSecondPerClient;					// 每个客户端每秒的下载带宽

	Profiles_Config profiles;

	uint32 defaultAddBots_totalCount;						// 默认启动进程后自动添加这么多个bots 添加总数量
	float defaultAddBots_tickTime;							// 默认启动进程后自动添加这么多个bots 每次添加所用时间(s)
	uint32 defaultAddBots_tickCount;						// 默认启动进程后自动添加这么多个bots 每次添加数量

	bool forceInternalLogin;								// 对应baseapp的externalAddress的解决方案，当externalAddress强制下发公网IP提供登陆时，
															// 如果局域网内部使用机器人测试也走公网IP和流量可能会不合适，此时可以设置为true，登陆时强制直接使用内网环境

	std::string bots_account_name_prefix;					// 机器人账号名称的前缀
	uint32 bots_account_name_suffix_inc;					// 机器人账号名称的后缀递增, 0使用随机数递增， 否则按照baseNum填写的数递增
	std::string bots_account_passwd;						// 机器人账号的密码

	uint32 tcp_SOMAXCONN;									// listen监听队列最大值

	int8 encrypt_login;										// 加密登录信息

	uint32 telnet_port;
	std::string telnet_passwd;
	std::string telnet_deflayer;

	uint32 perSecsDestroyEntitySize;						// 每秒销毁entity数量

	uint64 respool_timeout;
	uint32 respool_buffersize;

	uint8 account_type;										// 1: 普通账号, 2: email账号(需要激活), 3: 智能账号(自动识别email， 普通号码等) 
	uint32 accountDefaultFlags;								// 新账号默认标记(ACCOUNT_FLAGS可叠加， 填写时按十进制格式) 
	uint64 accountDefaultDeadline;							// 新账号默认过期时间(秒, 引擎会加上当前时间)
	
	std::string http_cbhost;
	uint16 http_cbport;										// 用户http回调接口，处理认证、密码重置等

	bool debugDBMgr;										// debug模式下可输出读写操作信息

	bool isOnInitCallPropertysSetMethods;					// 机器人(bots)专用：在Entity初始化时是否触发属性的set_*事件
} ENGINE_COMPONENT_INFO;

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
	INLINE ENGINE_COMPONENT_INFO& getBots(void);
	INLINE ENGINE_COMPONENT_INFO& getLogger(void);
	INLINE ENGINE_COMPONENT_INFO& getInterfaces(void);

	INLINE ENGINE_COMPONENT_INFO& getComponent(COMPONENT_TYPE componentType);
 	
	INLINE ENGINE_COMPONENT_INFO& getConfig();

 	void updateInfos(bool isPrint, COMPONENT_TYPE componentType, COMPONENT_ID componentID, 
 				const Network::Address& internalTcpAddr, const Network::Address& externalTcpAddr, const Network::Address& externalUdpAddr);
 	
	void updateExternalAddress(char* buf);

	INLINE int16 gameUpdateHertz(void) const;

	std::string interfacesAddress(void) const;
	int32 interfacesPortMin(void) const;
	int32 interfacesPortMax(void) const;
	INLINE std::vector< Network::Address > interfacesAddrs(void) const;

	const ChannelCommon& channelCommon(){ return channelCommon_; }

	uint32 tcp_SOMAXCONN(COMPONENT_TYPE componentType);

	float shutdowntime(){ return shutdown_time_; }
	float shutdownWaitTickTime(){ return shutdown_waitTickTime_; }

	uint32 tickMaxBufferedLogs() const { return tick_max_buffered_logs_; }
	uint32 tickMaxSyncLogs() const { return tick_max_sync_logs_; }

	INLINE float channelExternalTimeout(void) const;
	INLINE bool isPureDBInterfaceName(const std::string& dbInterfaceName);
	INLINE DBInterfaceInfo* dbInterface(const std::string& name);
	INLINE int dbInterfaceName2dbInterfaceIndex(const std::string& dbInterfaceName);
	INLINE const char* dbInterfaceIndex2dbInterfaceName(size_t dbInterfaceIndex);

private:
	void _updateEmailInfos();

private:
	ENGINE_COMPONENT_INFO _cellAppInfo;
	ENGINE_COMPONENT_INFO _baseAppInfo;
	ENGINE_COMPONENT_INFO _dbmgrInfo;
	ENGINE_COMPONENT_INFO _loginAppInfo;
	ENGINE_COMPONENT_INFO _cellAppMgrInfo;
	ENGINE_COMPONENT_INFO _baseAppMgrInfo;
	ENGINE_COMPONENT_INFO _kbMachineInfo;
	ENGINE_COMPONENT_INFO _botsInfo;
	ENGINE_COMPONENT_INFO _loggerInfo;
	ENGINE_COMPONENT_INFO _interfacesInfo;

public:
	int16 gameUpdateHertz_;
	uint32 tick_max_buffered_logs_;
	uint32 tick_max_sync_logs_;

	ChannelCommon channelCommon_;

	// 每个客户端每秒占用的最大带宽
	uint32 bitsPerSecondToClient_;		

	std::string interfacesAddress_;
	int32 interfacesPort_min_;
	int32 interfacesPort_max_;
	std::vector< Network::Address > interfacesAddrs_;
	uint32 interfaces_orders_timeout_;

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
#include "serverconfig.inl"
#endif
#endif // KBE_SERVER_CONFIG_H
