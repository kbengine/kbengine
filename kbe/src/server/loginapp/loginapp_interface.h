// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_LOGINAPP_INTERFACE_H
#endif


#ifndef KBE_LOGINAPP_INTERFACE_H
#define KBE_LOGINAPP_INTERFACE_H

// common include	
#if defined(LOGINAPP)
#include "loginapp.h"
#endif
#include "loginapp_interface_macros.h"
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
	LOGINAPP所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(LoginappInterface)
	// 客户端协议导出。
	LOGINAPP_MESSAGE_EXPOSED(importClientMessages)
	LOGINAPP_MESSAGE_DECLARE_ARGS0(importClientMessages,							NETWORK_FIXED_MESSAGE)

	// 错误码描述导出。
	LOGINAPP_MESSAGE_EXPOSED(importServerErrorsDescr)
	LOGINAPP_MESSAGE_DECLARE_ARGS0(importServerErrorsDescr,							NETWORK_FIXED_MESSAGE)

	// 客户端SDK导出。
	LOGINAPP_MESSAGE_EXPOSED(importClientSDK)
	LOGINAPP_MESSAGE_DECLARE_STREAM(importClientSDK,								NETWORK_VARIABLE_MESSAGE)

	// 某app主动请求断线。
	LOGINAPP_MESSAGE_DECLARE_ARGS0(reqClose,										NETWORK_FIXED_MESSAGE)

	// 某app主动请求look。
	LOGINAPP_MESSAGE_DECLARE_ARGS0(lookApp,											NETWORK_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	LOGINAPP_MESSAGE_DECLARE_ARGS0(queryLoad,										NETWORK_FIXED_MESSAGE)

	// hello握手。
	NETWORK_MESSAGE_EXPOSED(Loginapp, hello)
	LOGINAPP_MESSAGE_DECLARE_STREAM(hello,											NETWORK_VARIABLE_MESSAGE)

	// 某个app向本app告知处于活动状态。
	LOGINAPP_MESSAGE_EXPOSED(onClientActiveTick)
	LOGINAPP_MESSAGE_DECLARE_ARGS0(onClientActiveTick,								NETWORK_FIXED_MESSAGE)
	
	// 请求创建账号
	LOGINAPP_MESSAGE_EXPOSED(reqCreateAccount)
	LOGINAPP_MESSAGE_DECLARE_STREAM(reqCreateAccount,								NETWORK_VARIABLE_MESSAGE)

	LOGINAPP_MESSAGE_EXPOSED(reqCreateMailAccount)
	LOGINAPP_MESSAGE_DECLARE_STREAM(reqCreateMailAccount,							NETWORK_VARIABLE_MESSAGE)

	// 重置账号密码申请
	LOGINAPP_MESSAGE_EXPOSED(reqAccountResetPassword)
	LOGINAPP_MESSAGE_DECLARE_ARGS1(reqAccountResetPassword,							NETWORK_VARIABLE_MESSAGE,
									std::string,									accountName)

	// 重置账号密码申请的回调
	LOGINAPP_MESSAGE_DECLARE_ARGS4(onReqAccountResetPasswordCB,						NETWORK_VARIABLE_MESSAGE,
									std::string,									accountName,
									std::string,									email,
									SERVER_ERROR_CODE,								failedcode,
									std::string,									code)
	// 用户登录服务器 
	LOGINAPP_MESSAGE_EXPOSED(login)
	LOGINAPP_MESSAGE_DECLARE_STREAM(login,											NETWORK_VARIABLE_MESSAGE)

	// 某app请求获取一个entityID段的回调
	LOGINAPP_MESSAGE_DECLARE_ARGS3(onDbmgrInitCompleted,							NETWORK_VARIABLE_MESSAGE,
									COMPONENT_ORDER,								startGlobalOrder,
									COMPONENT_ORDER,								startGroupOrder,
									std::string,									digest)

	// 某个app向本app告知处于活动状态。
	LOGINAPP_MESSAGE_DECLARE_ARGS2(onAppActiveTick,									NETWORK_FIXED_MESSAGE,
									COMPONENT_TYPE,									componentType, 
									COMPONENT_ID,									componentID)

	// 从dbmgr查询到用户合法性结果
	LOGINAPP_MESSAGE_DECLARE_STREAM(onLoginAccountQueryResultFromDbmgr,				NETWORK_VARIABLE_MESSAGE)

	// baseappmgr返回的登录网关地址
	LOGINAPP_MESSAGE_DECLARE_ARGS5(onLoginAccountQueryBaseappAddrFromBaseappmgr,	NETWORK_VARIABLE_MESSAGE,
									std::string,									loginName, 
									std::string,									accountName,
									std::string,									addr,
									uint16,											tcp_port,
									uint16,											udp_port)

	// 向dbmgr请求创建账号返回结果
	LOGINAPP_MESSAGE_DECLARE_STREAM(onReqCreateAccountResult,						NETWORK_VARIABLE_MESSAGE)
	LOGINAPP_MESSAGE_DECLARE_STREAM(onReqCreateMailAccountResult,					NETWORK_VARIABLE_MESSAGE)

	// dbmgr账号激活返回
	LOGINAPP_MESSAGE_DECLARE_ARGS2(onAccountActivated,								NETWORK_VARIABLE_MESSAGE,
									std::string,									code, 
									bool,											success)
	
	// dbmgr账号绑定email返回
	LOGINAPP_MESSAGE_DECLARE_ARGS2(onAccountBindedEmail,							NETWORK_VARIABLE_MESSAGE,
									std::string,									code, 
									bool,											success)

	// dbmgr账号重设密码返回
	LOGINAPP_MESSAGE_DECLARE_ARGS2(onAccountResetPassword,							NETWORK_VARIABLE_MESSAGE,
									std::string,									code, 
									bool,											success)

	// baseapp请求绑定email（返回时需要找到loginapp的地址）
	LOGINAPP_MESSAGE_DECLARE_ARGS6(onReqAccountBindEmailAllocCallbackLoginapp,		NETWORK_VARIABLE_MESSAGE,
									COMPONENT_ID,									reqBaseappID,
									ENTITY_ID,										entityID,
									std::string,									accountName,
									std::string,									email,
									SERVER_ERROR_CODE,								failedcode,
									std::string,									code)

	// 请求关闭服务器
	LOGINAPP_MESSAGE_DECLARE_STREAM(reqCloseServer,									NETWORK_VARIABLE_MESSAGE)


	// 请求查询watcher数据
	LOGINAPP_MESSAGE_DECLARE_STREAM(queryWatcher,									NETWORK_VARIABLE_MESSAGE)

	// baseapp同步自己的初始化信息
	LOGINAPP_MESSAGE_DECLARE_ARGS1(onBaseappInitProgress,							NETWORK_FIXED_MESSAGE,
									float,											progress)

	// 开始profile
	LOGINAPP_MESSAGE_DECLARE_STREAM(startProfile,									NETWORK_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	LOGINAPP_MESSAGE_DECLARE_STREAM(reqKillServer,									NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
