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
#include "proxy_interface_macros.hpp"
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
	// 某app主动请求断线。
	BASEAPP_MESSAGE_DECLARE_ARGS0(reqClose,									MERCURY_FIXED_MESSAGE)

	// 某app主动请求look。
	BASEAPP_MESSAGE_DECLARE_ARGS0(lookApp,									MERCURY_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	BASEAPP_MESSAGE_DECLARE_ARGS0(queryLoad,								MERCURY_FIXED_MESSAGE)

	// console远程执行python语句。
	BASEAPP_MESSAGE_DECLARE_STREAM(onExecScriptCommand,						MERCURY_VARIABLE_MESSAGE)

	// 某app注册自己的接口地址到本app
	BASEAPP_MESSAGE_DECLARE_ARGS8(onRegisterNewApp,							MERCURY_VARIABLE_MESSAGE,
									int32,									uid, 
									std::string,							username,
									int8,									componentType, 
									uint64,									componentID, 
									uint32,									intaddr, 
									uint16,									intport,
									uint32,									extaddr, 
									uint16,									extport)

	// dbmgr告知已经启动的其他baseapp或者cellapp的地址
	// 当前app需要主动的去与他们建立连接
	BASEAPP_MESSAGE_DECLARE_ARGS8(onGetEntityAppFromDbmgr,					MERCURY_VARIABLE_MESSAGE,
									int32,									uid, 
									std::string,							username,
									int8,									componentType, 
									uint64,									componentID, 
									uint32,									intaddr, 
									uint16,									intport,
									uint32,									extaddr, 
									uint16,									extport)

	// 某app请求获取一个entityID段的回调
	BASEAPP_MESSAGE_DECLARE_ARGS2(onReqAllocEntityID,						MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								startID,
									ENTITY_ID,								endID)


	// 某app请求获取一个entityID段的回调
	BASEAPP_MESSAGE_DECLARE_ARGS5(onDbmgrInitCompleted,						MERCURY_FIXED_MESSAGE,
									GAME_TIME,								gametime, 
									ENTITY_ID,								startID,
									ENTITY_ID,								endID,
									int32,									startGlobalOrder,
									int32,									startGroupOrder)

	// hello握手。
	BASEAPP_MESSAGE_DECLARE_STREAM(hello,									MERCURY_VARIABLE_MESSAGE)

	// global数据改变
	BASEAPP_MESSAGE_DECLARE_STREAM(onBroadcastGlobalDataChange,				MERCURY_VARIABLE_MESSAGE)
	BASEAPP_MESSAGE_DECLARE_STREAM(onBroadcastGlobalBasesChange,			MERCURY_VARIABLE_MESSAGE)

	// 某个app向本app告知处于活动状态。
	BASEAPP_MESSAGE_DECLARE_ARGS2(onAppActiveTick,							MERCURY_FIXED_MESSAGE,
									COMPONENT_TYPE,							componentType, 
									COMPONENT_ID,							componentID)

	// 某个app向本app告知处于活动状态。
	BASEAPP_MESSAGE_DECLARE_ARGS0(onClientActiveTick,						MERCURY_FIXED_MESSAGE)

	// 收到baseappmgr决定将某个baseapp要求createBaseAnywhere的请求在本baseapp上执行 
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateBaseAnywhere,					MERCURY_VARIABLE_MESSAGE)

	// createBaseAnywhere成功之后回调消息到发起层createBaseAnywhere的baseapp的entity。
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateBaseAnywhereCallback,			MERCURY_FIXED_MESSAGE)

	// createCellEntity的cell实体创建成功回调。
	BASEAPP_MESSAGE_DECLARE_ARGS3(onEntityGetCell,							MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								id,
									COMPONENT_ID,							componentID,
									SPACE_ID,								spaceID)

	// createCellEntity的cell实体创建成功回调。
	BASEAPP_MESSAGE_DECLARE_ARGS1(onCreateCellFailure,						MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								entityID)

	// loginapp向自己注册一个将要登录的账号, 由baseappmgr转发。
	BASEAPP_MESSAGE_DECLARE_ARGS5(registerPendingLogin,						MERCURY_VARIABLE_MESSAGE,
									std::string,							loginName, 
									std::string,							accountName,
									std::string,							password,
									ENTITY_ID,								entityID,
									DBID,									entityDBID)

	// 前端请求登录到网关上。
	BASEAPP_MESSAGE_DECLARE_ARGS2(loginGateway,								MERCURY_VARIABLE_MESSAGE,
									std::string,							accountName,
									std::string,							password)

	// 前端请求重新登录到网关上。
	BASEAPP_MESSAGE_DECLARE_ARGS4(reLoginGateway,							MERCURY_FIXED_MESSAGE,
									std::string,							accountName,
									std::string,							password,
									uint64,									key,
									ENTITY_ID,								entityID)

	// 从dbmgr获取到账号Entity信息
	BASEAPP_MESSAGE_DECLARE_STREAM(onQueryAccountCBFromDbmgr,				MERCURY_VARIABLE_MESSAGE)

	// cellapp上的entity已经进入游戏世界了。
	BASEAPP_MESSAGE_DECLARE_ARGS1(onEntityEnterWorldFromCellapp,			MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid)

	// cellapp上的entity已经离开游戏世界了。
	BASEAPP_MESSAGE_DECLARE_ARGS1(onEntityLeaveWorldFromCellapp,			MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid)

	// cellapp上的entity已经进入一个space了。
	BASEAPP_MESSAGE_DECLARE_ARGS2(onEntityEnterSpaceFromCellapp,			MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid,
									SPACE_ID,								spaceID)

	// cellapp上的entity已经离开一个space了。
	BASEAPP_MESSAGE_DECLARE_ARGS2(onEntityLeaveSpaceFromCellapp,			MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid,
									SPACE_ID,								spaceID)

	// entity收到一封mail, 由某个app上的mailbox发起
	BASEAPP_MESSAGE_DECLARE_STREAM(onEntityMail,							MERCURY_VARIABLE_MESSAGE)
	
	// client访问entity的cell方法
	BASEAPP_MESSAGE_DECLARE_STREAM(onRemoteCallCellMethodFromClient,		MERCURY_VARIABLE_MESSAGE)

	// executeRawDatabaseCommand从dbmgr的回调
	BASEAPP_MESSAGE_DECLARE_STREAM(onExecuteRawDatabaseCommandCB,			MERCURY_VARIABLE_MESSAGE)

	// cellapp备份entity的cell数据
	BASEAPP_MESSAGE_DECLARE_STREAM(onBackupEntityCellData,					MERCURY_VARIABLE_MESSAGE)

	// cellapp writeToDB完成
	BASEAPP_MESSAGE_DECLARE_STREAM(onCellWriteToDBCompleted,				MERCURY_VARIABLE_MESSAGE)

	// cellapp转发entity消息给client
	BASEAPP_MESSAGE_DECLARE_STREAM(forwardMessageToClientFromCellapp,		MERCURY_VARIABLE_MESSAGE)

	// 请求关闭服务器
	BASEAPP_MESSAGE_DECLARE_STREAM(reqCloseServer,							MERCURY_VARIABLE_MESSAGE)

	// 写entity到db回调。
	BASEAPP_MESSAGE_DECLARE_ARGS4(onWriteToDBCallback,						MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid,
									DBID,									entityDBID,
									CALLBACK_ID,							callbackID,
									bool,									success)

	// createBaseFromDBID的回调
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateBaseFromDBIDCallback,			MERCURY_FIXED_MESSAGE)

	// 请求查询watcher数据
	BASEAPP_MESSAGE_DECLARE_STREAM(queryWatcher,							MERCURY_VARIABLE_MESSAGE)

	// 充值回调
	BASEAPP_MESSAGE_DECLARE_STREAM(onChargeCB,								MERCURY_VARIABLE_MESSAGE)

	// 开始profile
	BASEAPP_MESSAGE_DECLARE_STREAM(startProfile,							MERCURY_VARIABLE_MESSAGE)

	//--------------------------------------------Base----------------------------------------------------------
	// 远程呼叫entity方法
	BASE_MESSAGE_DECLARE_STREAM(onRemoteMethodCall,							MERCURY_VARIABLE_MESSAGE)

	// cellapp通报该entity的cell部分销毁或者丢失
	BASE_MESSAGE_DECLARE_STREAM(onLoseCell,									MERCURY_VARIABLE_MESSAGE)

	// 客户端直接发送消息给cell实体
	BASE_MESSAGE_DECLARE_STREAM(forwardEntityMessageToCellappFromClient,	MERCURY_VARIABLE_MESSAGE)
	
	// 某个entity请求teleport到本entity的space上
	BASE_MESSAGE_DECLARE_ARGS3(reqTeleportOther,							MERCURY_FIXED_MESSAGE,
								ENTITY_ID,									reqTeleportEntityID,
								COMPONENT_ID,								reqTeleportEntityAppID,
								COMPONENT_ID,								reqTeleportEntityBaseAppID)

	// 某个entity请求teleport后的回调结果
	BASE_MESSAGE_DECLARE_ARGS1(onTeleportCB,								MERCURY_FIXED_MESSAGE,
								SPACE_ID,									spaceID)

	// 某个entity请求teleport后的回调结果
	BASE_MESSAGE_DECLARE_ARGS1(onGetDBID,									MERCURY_FIXED_MESSAGE,
								DBID,										dbid)

	//--------------------------------------------Proxy---------------------------------------------------------
	/**
		远程呼叫entity方法
	*/
	PROXY_MESSAGE_DECLARE_ARGS0(onClientGetCell,							MERCURY_FIXED_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
