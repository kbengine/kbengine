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

#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_LOGINAPP_INTERFACE_HPP
#endif


#ifndef KBE_LOGINAPP_INTERFACE_HPP
#define KBE_LOGINAPP_INTERFACE_HPP

// common include	
#if defined(LOGINAPP)
#include "loginapp.hpp"
#endif
#include "loginapp_interface_macros.hpp"
#include "network/interface_defs.hpp"
#include "server/server_errors.hpp"
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
	LOGINAPP_MESSAGE_DECLARE_ARGS0(importClientMessages,							MERCURY_FIXED_MESSAGE)

	// 错误码描述导出。
	LOGINAPP_MESSAGE_EXPOSED(importServerErrorsDescr)
	LOGINAPP_MESSAGE_DECLARE_ARGS0(importServerErrorsDescr,							MERCURY_FIXED_MESSAGE)

	// 某app主动请求断线。
	LOGINAPP_MESSAGE_DECLARE_ARGS0(reqClose,										MERCURY_FIXED_MESSAGE)

	// 某app主动请求look。
	LOGINAPP_MESSAGE_DECLARE_ARGS0(lookApp,											MERCURY_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	LOGINAPP_MESSAGE_DECLARE_ARGS0(queryLoad,										MERCURY_FIXED_MESSAGE)

	// hello握手。
	NETWORK_MESSAGE_EXPOSED(Loginapp, hello)
	LOGINAPP_MESSAGE_DECLARE_STREAM(hello,											MERCURY_VARIABLE_MESSAGE)

	// 某个app向本app告知处于活动状态。
	LOGINAPP_MESSAGE_EXPOSED(onClientActiveTick)
	LOGINAPP_MESSAGE_DECLARE_ARGS0(onClientActiveTick,								MERCURY_FIXED_MESSAGE)
	
	// 请求创建账号
	LOGINAPP_MESSAGE_EXPOSED(reqCreateAccount)
	LOGINAPP_MESSAGE_DECLARE_STREAM(reqCreateAccount,								MERCURY_VARIABLE_MESSAGE)

	LOGINAPP_MESSAGE_EXPOSED(reqCreateMailAccount)
	LOGINAPP_MESSAGE_DECLARE_STREAM(reqCreateMailAccount,							MERCURY_VARIABLE_MESSAGE)

	// 重置账号密码申请
	LOGINAPP_MESSAGE_EXPOSED(reqAccountResetPassword)
	LOGINAPP_MESSAGE_DECLARE_ARGS1(reqAccountResetPassword,							MERCURY_VARIABLE_MESSAGE,
									std::string,									accountName)

	// 重置账号密码申请的回调
	LOGINAPP_MESSAGE_DECLARE_ARGS4(onReqAccountResetPasswordCB,						MERCURY_VARIABLE_MESSAGE,
									std::string,									accountName,
									std::string,									email,
									SERVER_ERROR_CODE,								failedcode,
									std::string,									code)
	// 用户登录服务器 
	LOGINAPP_MESSAGE_EXPOSED(login)
	LOGINAPP_MESSAGE_DECLARE_STREAM(login,											MERCURY_VARIABLE_MESSAGE)

	// 某app请求获取一个entityID段的回调
	LOGINAPP_MESSAGE_DECLARE_ARGS3(onDbmgrInitCompleted,							MERCURY_VARIABLE_MESSAGE,
									int32,											startGlobalOrder,
									int32,											startGroupOrder,
									std::string,									digest)

	// 某个app向本app告知处于活动状态。
	LOGINAPP_MESSAGE_DECLARE_ARGS2(onAppActiveTick,									MERCURY_FIXED_MESSAGE,
									COMPONENT_TYPE,									componentType, 
									COMPONENT_ID,									componentID)

	// 从dbmgr查询到用户合法性结果
	LOGINAPP_MESSAGE_DECLARE_STREAM(onLoginAccountQueryResultFromDbmgr,				MERCURY_VARIABLE_MESSAGE)

	// baseappmgr返回的登录网关地址
	LOGINAPP_MESSAGE_DECLARE_ARGS4(onLoginAccountQueryBaseappAddrFromBaseappmgr,	MERCURY_VARIABLE_MESSAGE,
									std::string,									loginName, 
									std::string,									accountName,
									uint32,											addr,
									uint16,											port)

	// 向dbmgr请求创建账号返回结果
	LOGINAPP_MESSAGE_DECLARE_STREAM(onReqCreateAccountResult,						MERCURY_VARIABLE_MESSAGE)
	LOGINAPP_MESSAGE_DECLARE_STREAM(onReqCreateMailAccountResult,					MERCURY_VARIABLE_MESSAGE)

	// dbmgr账号激活返回
	LOGINAPP_MESSAGE_DECLARE_ARGS2(onAccountActivated,								MERCURY_VARIABLE_MESSAGE,
									std::string,									code, 
									bool,											success)
	
	// dbmgr账号绑定email返回
	LOGINAPP_MESSAGE_DECLARE_ARGS2(onAccountBindedEmail,							MERCURY_VARIABLE_MESSAGE,
									std::string,									code, 
									bool,											success)

	// dbmgr账号重设密码返回
	LOGINAPP_MESSAGE_DECLARE_ARGS2(onAccountResetPassword,							MERCURY_VARIABLE_MESSAGE,
									std::string,									code, 
									bool,											success)

	// 请求关闭服务器
	LOGINAPP_MESSAGE_DECLARE_STREAM(reqCloseServer,									MERCURY_VARIABLE_MESSAGE)


	// 请求查询watcher数据
	LOGINAPP_MESSAGE_DECLARE_STREAM(queryWatcher,									MERCURY_VARIABLE_MESSAGE)

	// baseapp同步自己的初始化信息
	LOGINAPP_MESSAGE_DECLARE_ARGS1(onBaseappInitProgress,							MERCURY_FIXED_MESSAGE,
									float,											progress)

	// 开始profile
	LOGINAPP_MESSAGE_DECLARE_STREAM(startProfile,									MERCURY_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	LOGINAPP_MESSAGE_DECLARE_STREAM(reqKillServer,									MERCURY_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
