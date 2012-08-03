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
	#undef __BASEAPP_INTERFACE_H__
#endif


#ifndef __BASEAPP_INTERFACE_H__
#define __BASEAPP_INTERFACE_H__

// common include	
#if defined(BASEAPP)
#include "baseapp.hpp"
#endif
#include "baseapp_interface_macros.hpp"
#include "base_interface_macros.hpp"
#include "network/interface_defs.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	BASEAPP所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(BaseappInterface)
	// 某app注册自己的接口地址到本app
	BASEAPP_MESSAGE_DECLARE_ARGS8(onRegisterNewApp,					MERCURY_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									int8,							componentType, 
									uint64,							componentID, 
									uint32,							intaddr, 
									uint16,							intport,
									uint32,							extaddr, 
									uint16,							extport)

	// dbmgr告知已经启动的其他baseapp或者cellapp的地址
	// 当前app需要主动的去与他们建立连接
	BASEAPP_MESSAGE_DECLARE_ARGS8(onGetEntityAppFromDbmgr,			MERCURY_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									int8,							componentType, 
									uint64,							componentID, 
									uint32,							intaddr, 
									uint16,							intport,
									uint32,							extaddr, 
									uint16,							extport)

	// 某app请求获取一个entityID段的回调
	BASEAPP_MESSAGE_DECLARE_ARGS2(onReqAllocEntityID,				MERCURY_FIXED_MESSAGE,
									ENTITY_ID,						startID,
									ENTITY_ID,						endID)


	// 某app请求获取一个entityID段的回调
	BASEAPP_MESSAGE_DECLARE_ARGS4(onDbmgrInitCompleted,				MERCURY_FIXED_MESSAGE,
									ENTITY_ID,						startID,
									ENTITY_ID,						endID,
									int32,							startGlobalOrder,
									int32,							startGroupOrder)

	// global数据改变
	BASEAPP_MESSAGE_DECLARE_STREAM(onBroadcastGlobalDataChange,		MERCURY_VARIABLE_MESSAGE)
	BASEAPP_MESSAGE_DECLARE_STREAM(onBroadcastGlobalBasesChange,	MERCURY_VARIABLE_MESSAGE)

	// 某个app向本app告知处于活动状态。
	BASEAPP_MESSAGE_DECLARE_ARGS2(onAppActiveTick,					MERCURY_FIXED_MESSAGE,
									COMPONENT_TYPE,					componentType, 
									COMPONENT_ID,					componentID)

	// 收到baseappmgr决定将某个baseapp要求createBaseAnywhere的请求在本baseapp上执行 
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateBaseAnywhere,			MERCURY_VARIABLE_MESSAGE)

	// createBaseAnywhere成功之后回调消息到发起层createBaseAnywhere的baseapp的entity。
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateBaseAnywhereCallback,	MERCURY_FIXED_MESSAGE)

	// createCellEntity的cell实体创建成功回调。
	BASEAPP_MESSAGE_DECLARE_ARGS2(onEntityGetCell,					MERCURY_FIXED_MESSAGE,
									ENTITY_ID,						id,
									COMPONENT_ID,					componentID)

	// loginapp向自己注册一个将要登录的账号。
	BASEAPP_MESSAGE_DECLARE_ARGS2(registerPendingLogin,				MERCURY_VARIABLE_MESSAGE,
									std::string,					accountName,
									std::string,					password)

	// 前端请求登录到网关上。
	BASEAPP_MESSAGE_DECLARE_ARGS2(loginGateway,						MERCURY_VARIABLE_MESSAGE,
									std::string,					accountName,
									std::string,					password)

	// 前端请求重新登录到网关上。
	BASEAPP_MESSAGE_DECLARE_ARGS2(reLoginGateway,					MERCURY_FIXED_MESSAGE,
									uint64,							key,
									ENTITY_ID,						entityID)

	/**
		远程呼叫entity方法
	*/
	BASE_MESSAGE_DECLARE_STREAM(onRemoteMethodCall,					MERCURY_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
