// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_DBMGR_INTERFACE_H
#endif


#ifndef KBE_DBMGR_INTERFACE_H
#define KBE_DBMGR_INTERFACE_H

// common include	
#if defined(DBMGR)
#include "dbmgr.h"
#endif
#include "dbmgr_interface_macros.h"
#include "network/interface_defs.h"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Dbmgr消息宏，  参数为流， 需要自己解开
*/

/**
	DBMGR所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(DbmgrInterface)
	// 某app注册自己的接口地址到本app
	DBMGR_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,					NETWORK_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									COMPONENT_TYPE,					componentType, 
									COMPONENT_ID,					componentID, 
									COMPONENT_ORDER,				globalorderID,
									COMPONENT_ORDER,				grouporderID,
									uint32,							intaddr, 
									uint16,							intport,
									uint32,							extaddr, 
									uint16,							extport,
									std::string,					extaddrEx)

	// 某app主动请求look。
	DBMGR_MESSAGE_DECLARE_ARGS0(lookApp,							NETWORK_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	DBMGR_MESSAGE_DECLARE_ARGS0(queryLoad,							NETWORK_FIXED_MESSAGE)

	// 某app请求获取一个entityID段 
	DBMGR_MESSAGE_DECLARE_ARGS2(onReqAllocEntityID,					NETWORK_FIXED_MESSAGE,
								COMPONENT_TYPE,						componentType,
								COMPONENT_ID,						componentID)

	// global数据改变
	DBMGR_MESSAGE_DECLARE_STREAM(onBroadcastGlobalDataChanged,		NETWORK_VARIABLE_MESSAGE)

	// 某个app向本app告知处于活动状态。
	DBMGR_MESSAGE_DECLARE_ARGS2(onAppActiveTick,					NETWORK_FIXED_MESSAGE,
									COMPONENT_TYPE,					componentType, 
									COMPONENT_ID,					componentID)

	// loginapp请求创建账号。
	DBMGR_MESSAGE_DECLARE_STREAM(reqCreateAccount,					NETWORK_VARIABLE_MESSAGE)
	DBMGR_MESSAGE_DECLARE_STREAM(onCreateAccountCBFromInterfaces,	NETWORK_VARIABLE_MESSAGE)

	// 登陆账号。
	DBMGR_MESSAGE_DECLARE_STREAM(onAccountLogin,					NETWORK_VARIABLE_MESSAGE)
	DBMGR_MESSAGE_DECLARE_STREAM(onLoginAccountCBBFromInterfaces,	NETWORK_VARIABLE_MESSAGE)

	// baseapp查询账号信息。
	DBMGR_MESSAGE_DECLARE_ARGS8(queryAccount,						NETWORK_VARIABLE_MESSAGE,
									std::string,					accountName,
									std::string,					password,
									bool,							needCheckPassword,
									COMPONENT_ID,					componentID,
									ENTITY_ID,						entityID,
									DBID,							entityDBID,
									uint32,							ip,
									uint16,							port)

	// baseapp上账号上线。
	DBMGR_MESSAGE_DECLARE_ARGS3(onAccountOnline,					NETWORK_VARIABLE_MESSAGE,
									std::string,					accountName,
									COMPONENT_ID,					componentID,
									ENTITY_ID,						entityID)
		
	// baseapp上entity下线。
	DBMGR_MESSAGE_DECLARE_ARGS3(onEntityOffline,					NETWORK_FIXED_MESSAGE,
									DBID,							dbid,
									uint16,							sid,
									uint16,							dbInterfaceIndex)

	// 请求擦除客户端请求任务。
	DBMGR_MESSAGE_DECLARE_ARGS1(eraseClientReq,						NETWORK_VARIABLE_MESSAGE,
									std::string,					logkey)

	// 数据库查询
	DBMGR_MESSAGE_DECLARE_STREAM(executeRawDatabaseCommand,			NETWORK_VARIABLE_MESSAGE)

	// 某个entity存档
	DBMGR_MESSAGE_DECLARE_STREAM(writeEntity,						NETWORK_VARIABLE_MESSAGE)

	// 删除某个entity的存档
	DBMGR_MESSAGE_DECLARE_STREAM(removeEntity,						NETWORK_VARIABLE_MESSAGE)

	// 请求从数据库删除实体
	DBMGR_MESSAGE_DECLARE_STREAM(deleteEntityByDBID,				NETWORK_VARIABLE_MESSAGE)

	// 通过dbid查询一个实体是否从数据库检出
	DBMGR_MESSAGE_DECLARE_STREAM(lookUpEntityByDBID,				NETWORK_VARIABLE_MESSAGE)

	// 请求关闭服务器
	DBMGR_MESSAGE_DECLARE_STREAM(reqCloseServer,					NETWORK_VARIABLE_MESSAGE)

	// 某个app向本app告知处于活动状态。
	DBMGR_MESSAGE_DECLARE_ARGS7(queryEntity,						NETWORK_VARIABLE_MESSAGE, 
									uint16,							dbInterfaceIndex,
									COMPONENT_ID,					componentID,
									int8,							queryMode,
									DBID,							dbid, 
									std::string,					entityType,
									CALLBACK_ID,					callbackID,
									ENTITY_ID,						entityID)

	// 实体自动加载功能
	DBMGR_MESSAGE_DECLARE_STREAM(entityAutoLoad,					NETWORK_VARIABLE_MESSAGE)

	// 同步entity流模板
	DBMGR_MESSAGE_DECLARE_STREAM(syncEntityStreamTemplate,			NETWORK_VARIABLE_MESSAGE)

	// 请求查询watcher数据
	DBMGR_MESSAGE_DECLARE_STREAM(queryWatcher,						NETWORK_VARIABLE_MESSAGE)

	// 充值请求
	DBMGR_MESSAGE_DECLARE_STREAM(charge,							NETWORK_VARIABLE_MESSAGE)

	// 充值回调
	DBMGR_MESSAGE_DECLARE_STREAM(onChargeCB,						NETWORK_VARIABLE_MESSAGE)

	// 激活回调。
	DBMGR_MESSAGE_DECLARE_ARGS1(accountActivate,					NETWORK_VARIABLE_MESSAGE,
									std::string,					scode)

	// 账号请求重置密码。
	DBMGR_MESSAGE_DECLARE_ARGS1(accountReqResetPassword,			NETWORK_VARIABLE_MESSAGE,
									std::string,					accountName)

	// 账号完成重置密码。
	DBMGR_MESSAGE_DECLARE_ARGS3(accountResetPassword,				NETWORK_VARIABLE_MESSAGE,
									std::string,					accountName,
									std::string,					newpassword,
									std::string,					code)

	// 账号请求绑定邮箱。
	DBMGR_MESSAGE_DECLARE_ARGS4(accountReqBindMail,					NETWORK_VARIABLE_MESSAGE,
									ENTITY_ID,						entityID,
									std::string,					accountName,
									std::string,					password,
									std::string,					email)

	// 账号完成绑定邮箱。
	DBMGR_MESSAGE_DECLARE_ARGS2(accountBindMail,					NETWORK_VARIABLE_MESSAGE,
									std::string,					username,
									std::string,					code)

	// 账号修改密码。
	DBMGR_MESSAGE_DECLARE_ARGS4(accountNewPassword,					NETWORK_VARIABLE_MESSAGE,
									ENTITY_ID,						entityID,
									std::string,					accountName,
									std::string,					password,
									std::string,					newpassword)

	// 开始profile
	DBMGR_MESSAGE_DECLARE_STREAM(startProfile,						NETWORK_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	DBMGR_MESSAGE_DECLARE_STREAM(reqKillServer,						NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
