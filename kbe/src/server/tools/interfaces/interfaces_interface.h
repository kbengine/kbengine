// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_INTERFACES_TOOL_INTERFACE_H
#endif


#ifndef KBE_INTERFACES_TOOL_INTERFACE_H
#define KBE_INTERFACES_TOOL_INTERFACE_H

// common include	
#if defined(INTERFACES)
#include "interfaces.h"
#endif
#include "interfaces_interface_macros.h"
#include "network/interface_defs.h"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Interfaces消息宏，  参数为流， 需要自己解开
*/

/**
	Interfaces所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(InterfacesInterface)
	// 某app注册自己的接口地址到本app
	INTERFACES_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,						NETWORK_VARIABLE_MESSAGE,
									int32,									uid, 
									std::string,							username,
									COMPONENT_TYPE,							componentType, 
									COMPONENT_ID,							componentID, 
									COMPONENT_ORDER,						globalorderID,
									COMPONENT_ORDER,						grouporderID,
									uint32,									intaddr, 
									uint16,									intport,
									uint32,									extaddr, 
									uint16,									extport,
									std::string,							extaddrEx)

	// 请求创建账号。
	INTERFACES_MESSAGE_DECLARE_STREAM(reqCreateAccount,						NETWORK_VARIABLE_MESSAGE)

	// 登陆账号。
	INTERFACES_MESSAGE_DECLARE_STREAM(onAccountLogin,						NETWORK_VARIABLE_MESSAGE)

	// 充值请求
	INTERFACES_MESSAGE_DECLARE_STREAM(charge,								NETWORK_VARIABLE_MESSAGE)

	// 某app主动请求look。
	INTERFACES_MESSAGE_DECLARE_ARGS0(lookApp,								NETWORK_FIXED_MESSAGE)

	// 某个app向本app告知处于活动状态。
	INTERFACES_MESSAGE_DECLARE_ARGS2(onAppActiveTick,						NETWORK_FIXED_MESSAGE,
										COMPONENT_TYPE,						componentType, 
										COMPONENT_ID,						componentID)

	// 请求关闭服务器
	INTERFACES_MESSAGE_DECLARE_STREAM(reqCloseServer,						NETWORK_VARIABLE_MESSAGE)

	// 请求查询watcher数据
	INTERFACES_MESSAGE_DECLARE_STREAM(queryWatcher,							NETWORK_VARIABLE_MESSAGE)

	// 请求擦除客户端请求任务。
	INTERFACES_MESSAGE_DECLARE_ARGS1(eraseClientReq,						NETWORK_VARIABLE_MESSAGE,
										std::string,						logkey)

	// 开始profile
	INTERFACES_MESSAGE_DECLARE_STREAM(startProfile,							NETWORK_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	INTERFACES_MESSAGE_DECLARE_STREAM(reqKillServer,						NETWORK_VARIABLE_MESSAGE)

	// executeRawDatabaseCommand从dbmgr的回调
	INTERFACES_MESSAGE_DECLARE_STREAM(onExecuteRawDatabaseCommandCB,		NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}

#endif // KBE_INTERFACES_TOOL_INTERFACE_H
