// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_BASEAPPMGR_INTERFACE_H
#endif


#ifndef KBE_BASEAPPMGR_INTERFACE_H
#define KBE_BASEAPPMGR_INTERFACE_H

// common include	
#if defined(BASEAPPMGR)
#include "baseappmgr.h"
#endif
#include "baseappmgr_interface_macros.h"
#include "network/interface_defs.h"
#include "server/server_errors.h"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	BASEAPPMGR所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(BaseappmgrInterface)
	// 某app注册自己的接口地址到本app
	BASEAPPMGR_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,									NETWORK_VARIABLE_MESSAGE,
									int32,												uid, 
									std::string,										username,
									COMPONENT_TYPE,										componentType, 
									COMPONENT_ID,										componentID, 
									COMPONENT_ORDER,									globalorderID,
									COMPONENT_ORDER,									grouporderID,
									uint32,												intaddr, 
									uint16,												intport,
									uint32,												extaddr, 
									uint16,												extport,
									std::string,										extaddrEx)

	// 某app主动请求look。
	BASEAPPMGR_MESSAGE_DECLARE_ARGS0(lookApp,											NETWORK_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	BASEAPPMGR_MESSAGE_DECLARE_ARGS0(queryLoad,											NETWORK_FIXED_MESSAGE)

	// 某个app向本app告知处于活动状态。
	BASEAPPMGR_MESSAGE_DECLARE_ARGS2(onAppActiveTick,									NETWORK_FIXED_MESSAGE,
									COMPONENT_TYPE,										componentType, 
									COMPONENT_ID,										componentID)

	// baseEntity请求创建在一个新的space中。
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(reqCreateEntityAnywhere,							NETWORK_VARIABLE_MESSAGE)

	// baseEntity请求创建在一个新的space中。
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(reqCreateEntityRemotely,							NETWORK_VARIABLE_MESSAGE)

	// baseEntity请求创建在一个新的space中，查询当前最好的组件ID
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(reqCreateEntityAnywhereFromDBIDQueryBestBaseappID,NETWORK_VARIABLE_MESSAGE)

	// baseEntity请求创建在一个新的space中。
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(reqCreateEntityAnywhereFromDBID,					NETWORK_VARIABLE_MESSAGE)

	// baseEntity请求创建在一个新的space中。
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(reqCreateEntityRemotelyFromDBID,					NETWORK_VARIABLE_MESSAGE)
	
	// 消息转发， 由某个app想通过本app将消息转发给某个app。	
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(forwardMessage,									NETWORK_VARIABLE_MESSAGE)

	// 某个app向本app告知处于活动状态。
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(registerPendingAccountToBaseapp,					NETWORK_VARIABLE_MESSAGE)

	// 获取到baseapp的地址。
	BASEAPPMGR_MESSAGE_DECLARE_ARGS5(onPendingAccountGetBaseappAddr,					NETWORK_VARIABLE_MESSAGE,
									std::string,										loginName, 
									std::string,										accountName,
									std::string,										addr,
									uint16,												tcp_port,
									uint16,												udp_port)
									
	// 一个新登录的账号获得合法登入baseapp的权利， 现在需要将账号注册给指定的baseapp
	// 使其允许在此baseapp上登录。
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(registerPendingAccountToBaseappAddr,				NETWORK_VARIABLE_MESSAGE)

	// 请求关闭服务器
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(reqCloseServer,									NETWORK_VARIABLE_MESSAGE)

	// 更新baseapp信息。
	BASEAPPMGR_MESSAGE_DECLARE_ARGS5(updateBaseapp,										NETWORK_FIXED_MESSAGE,
									COMPONENT_ID,										componentID,
									ENTITY_ID,											numBases,
									ENTITY_ID,											numProxices,
									float,												load,
									uint32,												flags)

	// 请求查询watcher数据
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(queryWatcher,										NETWORK_VARIABLE_MESSAGE)

	// baseapp同步自己的初始化信息
	BASEAPPMGR_MESSAGE_DECLARE_ARGS2(onBaseappInitProgress,								NETWORK_FIXED_MESSAGE,
									COMPONENT_ID,										cid,
									float,												progress)

	// 开始profile
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(startProfile,										NETWORK_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(reqKillServer,									NETWORK_VARIABLE_MESSAGE)

	// 查询所有相关进程负载信息
	BASEAPPMGR_MESSAGE_DECLARE_STREAM(queryAppsLoads,									NETWORK_VARIABLE_MESSAGE)

	// baseapp请求绑定email（返回时需要找到loginapp的地址）
	BASEAPPMGR_MESSAGE_DECLARE_ARGS6(reqAccountBindEmailAllocCallbackLoginapp,			NETWORK_VARIABLE_MESSAGE,
									COMPONENT_ID,										reqBaseappID,
									ENTITY_ID,											entityID,
									std::string,										accountName,
									std::string,										email,
									SERVER_ERROR_CODE,									failedcode,
									std::string,										code)

	// baseapp请求绑定email（返回时需要找到loginapp的地址）
	BASEAPPMGR_MESSAGE_DECLARE_ARGS8(onReqAccountBindEmailCBFromLoginapp,				NETWORK_VARIABLE_MESSAGE,
									COMPONENT_ID,										reqBaseappID,
									ENTITY_ID,											entityID,
									std::string,										accountName,
									std::string,										email,
									SERVER_ERROR_CODE,									failedcode,
									std::string,										code,
									std::string,										loginappCBHost, 
									uint16,												loginappCBPort)

	NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
