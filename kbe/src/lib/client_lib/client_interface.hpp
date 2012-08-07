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
	#undef __CLIENT_INTERFACE_H__
#endif


#ifndef __CLIENT_INTERFACE_H__
#define __CLIENT_INTERFACE_H__

// common include	
#if defined(CLIENT)
#include "clientapp.hpp"
#endif
#include "client_interface_macros.hpp"
#include "network/interface_defs.hpp"
#include "server/mercury_errors.hpp"
#include "common.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	CLIENT所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(ClientInterface)
	// 创建账号失败。
	CLIENT_MESSAGE_DECLARE_ARGS1(onCreateAccountResult,			MERCURY_FIXED_MESSAGE,
									MERCURY_ERROR_CODE,			failedcode)

	// 登录成功。
	CLIENT_MESSAGE_DECLARE_STREAM(onLoginSuccessfully,			MERCURY_VARIABLE_MESSAGE)

	// 登录失败。
	CLIENT_MESSAGE_DECLARE_ARGS1(onLoginFailed,					MERCURY_FIXED_MESSAGE,
									MERCURY_ERROR_CODE,			failedcode)

	// 登录网关成功。
	CLIENT_MESSAGE_DECLARE_ARGS2(onLoginGatewaySuccessfully,	MERCURY_FIXED_MESSAGE,
									uint64,						rndUUID,
									ENTITY_ID,					eid)

	// 登录网关失败。
	CLIENT_MESSAGE_DECLARE_ARGS1(onLoginGatewayFailed,			MERCURY_FIXED_MESSAGE,
									MERCURY_ERROR_CODE,			failedcode)

	// 服务器上的entity已经进入游戏世界了。
	CLIENT_MESSAGE_DECLARE_ARGS1(onEntityEnterWorld,			MERCURY_FIXED_MESSAGE,
									ENTITY_ID,					eid)

	// 远程呼叫entity方法
	CLIENT_MESSAGE_DECLARE_STREAM(onRemoteMethodCall,			MERCURY_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
