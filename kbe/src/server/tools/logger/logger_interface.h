// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_LOGGER_INTERFACE_H
#endif


#ifndef KBE_LOGGER_INTERFACE_H
#define KBE_LOGGER_INTERFACE_H

// common include	
#if defined(LOGGER)
#include "logger.h"
#endif
#include "logger_interface_macros.h"
#include "network/interface_defs.h"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Logger所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(LoggerInterface)
	// 某app注册自己的接口地址到本app
	LOGGER_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,							NETWORK_VARIABLE_MESSAGE,
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
									std::string,							extAddrEx)

	// 某app主动请求look。
	LOGGER_MESSAGE_DECLARE_ARGS0(lookApp,									NETWORK_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	LOGGER_MESSAGE_DECLARE_ARGS0(queryLoad,									NETWORK_FIXED_MESSAGE)

	// 某个app向本app告知处于活动状态。
	LOGGER_MESSAGE_DECLARE_ARGS2(onAppActiveTick,							NETWORK_FIXED_MESSAGE,
									COMPONENT_TYPE,							componentType, 
									COMPONENT_ID,							componentID)

	// 远程写日志
	LOGGER_MESSAGE_DECLARE_STREAM(writeLog,									NETWORK_VARIABLE_MESSAGE)

	// 注册log监听者
	LOGGER_MESSAGE_DECLARE_STREAM(registerLogWatcher,						NETWORK_VARIABLE_MESSAGE)

	// 注销log监听者
	LOGGER_MESSAGE_DECLARE_STREAM(deregisterLogWatcher,						NETWORK_VARIABLE_MESSAGE)

	// log监听者更新自己的设置
	LOGGER_MESSAGE_DECLARE_STREAM(updateLogWatcherSetting,					NETWORK_VARIABLE_MESSAGE)

	// 请求关闭服务器
	LOGGER_MESSAGE_DECLARE_STREAM(reqCloseServer,							NETWORK_VARIABLE_MESSAGE)

	// 请求查询watcher数据
	LOGGER_MESSAGE_DECLARE_STREAM(queryWatcher,								NETWORK_VARIABLE_MESSAGE)

	// 开始profile
	LOGGER_MESSAGE_DECLARE_STREAM(startProfile,								NETWORK_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	LOGGER_MESSAGE_DECLARE_STREAM(reqKillServer,							NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
