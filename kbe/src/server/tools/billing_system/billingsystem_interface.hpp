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
	#undef KBE_BILLINGSYSTEM_INTERFACE_HPP
#endif


#ifndef KBE_BILLINGSYSTEM_INTERFACE_HPP
#define KBE_BILLINGSYSTEM_INTERFACE_HPP

// common include	
#if defined(BILLINGSYSTEM)
#include "billingsystem.hpp"
#endif
#include "billingsystem_interface_macros.hpp"
#include "network/interface_defs.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	BillingSystem消息宏，  参数为流， 需要自己解开
*/

/**
	BillingSystem所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(BillingSystemInterface)
	// 某app注册自己的接口地址到本app
	BILLINGSYSTEM_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,					MERCURY_VARIABLE_MESSAGE,
									int32,									uid, 
									std::string,							username,
									int8,									componentType, 
									uint64,									componentID, 
									int8,									globalorderID,
									int8,									grouporderID,
									uint32,									intaddr, 
									uint16,									intport,
									uint32,									extaddr, 
									uint16,									extport,
									std::string,							extaddrEx)

	// 请求创建账号。
	BILLINGSYSTEM_MESSAGE_DECLARE_STREAM(reqCreateAccount,					MERCURY_VARIABLE_MESSAGE)

	// 登陆账号。
	BILLINGSYSTEM_MESSAGE_DECLARE_STREAM(onAccountLogin,					MERCURY_VARIABLE_MESSAGE)

	// 充值请求
	BILLINGSYSTEM_MESSAGE_DECLARE_STREAM(charge,							MERCURY_VARIABLE_MESSAGE)

	// 某app主动请求look。
	BILLINGSYSTEM_MESSAGE_DECLARE_ARGS0(lookApp,							MERCURY_FIXED_MESSAGE)

	// 某个app向本app告知处于活动状态。
	BILLINGSYSTEM_MESSAGE_DECLARE_ARGS2(onAppActiveTick,					MERCURY_FIXED_MESSAGE,
										COMPONENT_TYPE,						componentType, 
										COMPONENT_ID,						componentID)

	// 请求关闭服务器
	BILLINGSYSTEM_MESSAGE_DECLARE_STREAM(reqCloseServer,					MERCURY_VARIABLE_MESSAGE)

	// 请求查询watcher数据
	BILLINGSYSTEM_MESSAGE_DECLARE_STREAM(queryWatcher,						MERCURY_VARIABLE_MESSAGE)

	// 请求擦除客户端请求任务。
	BILLINGSYSTEM_MESSAGE_DECLARE_ARGS1(eraseClientReq,						MERCURY_VARIABLE_MESSAGE,
										std::string,						logkey)

	// 开始profile
	BILLINGSYSTEM_MESSAGE_DECLARE_STREAM(startProfile,						MERCURY_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	BILLINGSYSTEM_MESSAGE_DECLARE_STREAM(reqKillServer,						MERCURY_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}

#endif // KBE_BILLINGSYSTEM_INTERFACE_HPP
