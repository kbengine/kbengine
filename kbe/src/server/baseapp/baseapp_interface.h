// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_BASEAPP_INTERFACE_H
#endif


#ifndef KBE_BASEAPP_INTERFACE_H
#define KBE_BASEAPP_INTERFACE_H

// common include	
#if defined(BASEAPP)
#include "baseapp.h"
#endif
#include "baseapp_interface_macros.h"
#include "entity_interface_macros.h"
#include "proxy_interface_macros.h"
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
	BASEAPP所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(BaseappInterface)
	// 客户端请求导入协议。
	BASEAPP_MESSAGE_EXPOSED(importClientMessages)
	BASEAPP_MESSAGE_DECLARE_ARGS0(importClientMessages,								NETWORK_FIXED_MESSAGE)

	// 客户端entitydef导出。
	BASEAPP_MESSAGE_EXPOSED(importClientEntityDef)
	BASEAPP_MESSAGE_DECLARE_ARGS0(importClientEntityDef,							NETWORK_FIXED_MESSAGE)

	// 某app主动请求断线。
	BASEAPP_MESSAGE_DECLARE_ARGS0(reqClose,											NETWORK_FIXED_MESSAGE)

	// 某app主动请求look。
	BASEAPP_MESSAGE_DECLARE_ARGS0(lookApp,											NETWORK_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	BASEAPP_MESSAGE_DECLARE_ARGS0(queryLoad,										NETWORK_FIXED_MESSAGE)

	// console远程执行python语句。
	BASEAPP_MESSAGE_DECLARE_STREAM(onExecScriptCommand,								NETWORK_VARIABLE_MESSAGE)

	// 某app注册自己的接口地址到本app
	BASEAPP_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,								NETWORK_VARIABLE_MESSAGE,
									int32,											uid, 
									std::string,									username,
									COMPONENT_TYPE,									componentType, 
									COMPONENT_ID,									componentID, 
									COMPONENT_ORDER,								globalorderID,
									COMPONENT_ORDER,								grouporderID,
									uint32,											intaddr, 
									uint16,											intport,
									uint32,											extaddr, 
									uint16,											extport,
									std::string,									extaddrEx)

	// dbmgr告知已经启动的其他baseapp或者cellapp的地址
	// 当前app需要主动的去与他们建立连接
	BASEAPP_MESSAGE_DECLARE_ARGS11(onGetEntityAppFromDbmgr,							NETWORK_VARIABLE_MESSAGE,
									int32,											uid, 
									std::string,									username,
									COMPONENT_TYPE,									componentType, 
									COMPONENT_ID,									componentID, 
									COMPONENT_ORDER,								globalorderID,
									COMPONENT_ORDER,								grouporderID,
									uint32,											intaddr, 
									uint16,											intport,
									uint32,											extaddr, 
									uint16,											extport,
									std::string,									extaddrEx)

	// 某app请求获取一个entityID段的回调
	BASEAPP_MESSAGE_DECLARE_ARGS2(onReqAllocEntityID,								NETWORK_FIXED_MESSAGE,
									ENTITY_ID,										startID,
									ENTITY_ID,										endID)


	// 某app请求获取一个entityID段的回调
	BASEAPP_MESSAGE_DECLARE_ARGS6(onDbmgrInitCompleted,								NETWORK_VARIABLE_MESSAGE,
									GAME_TIME,										gametime, 
									ENTITY_ID,										startID,
									ENTITY_ID,										endID,
									COMPONENT_ORDER,								startGlobalOrder,
									COMPONENT_ORDER,								startGroupOrder,
									std::string,									digest)

	// hello握手。
	BASEAPP_MESSAGE_EXPOSED(hello)
	BASEAPP_MESSAGE_DECLARE_STREAM(hello,											NETWORK_VARIABLE_MESSAGE)

	// global数据改变
	BASEAPP_MESSAGE_DECLARE_STREAM(onBroadcastGlobalDataChanged,					NETWORK_VARIABLE_MESSAGE)
	BASEAPP_MESSAGE_DECLARE_STREAM(onBroadcastBaseAppDataChanged,					NETWORK_VARIABLE_MESSAGE)

	// 某个app向本app告知处于活动状态。
	BASEAPP_MESSAGE_DECLARE_ARGS2(onAppActiveTick,									NETWORK_FIXED_MESSAGE,
									COMPONENT_TYPE,									componentType, 
									COMPONENT_ID,									componentID)

	// 某个app向本app告知处于活动状态。
	BASEAPP_MESSAGE_EXPOSED(onClientActiveTick)
	BASEAPP_MESSAGE_DECLARE_ARGS0(onClientActiveTick,								NETWORK_FIXED_MESSAGE)

	// 收到baseappmgr决定将某个baseapp要求createEntityAnywhere的请求在本baseapp上执行 
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateEntityAnywhere,							NETWORK_VARIABLE_MESSAGE)

	// createEntityAnywhere成功之后回调消息到发起层createEntityAnywhere的baseapp的entity。
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateEntityAnywhereCallback,					NETWORK_FIXED_MESSAGE)

	// createEntityRemotely成功之后回调消息到发起层createEntityRemotely的baseapp的entity。
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateEntityRemotely,							NETWORK_FIXED_MESSAGE)

	// createEntityRemotely成功之后回调消息到发起层createEntityRemotely的baseapp的entity。
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateEntityRemotelyCallback,					 NETWORK_FIXED_MESSAGE)

	// createCellEntity的cell实体创建成功回调。
	BASEAPP_MESSAGE_DECLARE_ARGS3(onEntityGetCell,									NETWORK_FIXED_MESSAGE,
									ENTITY_ID,										id,
									COMPONENT_ID,									componentID,
									SPACE_ID,										spaceID)

	// createCellEntity的cell实体创建成功回调。
	BASEAPP_MESSAGE_DECLARE_ARGS1(onCreateCellFailure,								NETWORK_FIXED_MESSAGE,
									ENTITY_ID,										entityID)

	// loginapp向自己注册一个将要登录的账号, 由baseappmgr转发。
	BASEAPP_MESSAGE_DECLARE_STREAM(registerPendingLogin,							NETWORK_VARIABLE_MESSAGE)

	// 数据库中查询的自动entity加载信息返回 
	BASEAPP_MESSAGE_DECLARE_STREAM(onEntityAutoLoadCBFromDBMgr,						NETWORK_VARIABLE_MESSAGE)

	// 前端请求登录到网关上。
	BASEAPP_MESSAGE_EXPOSED(loginBaseapp)
	BASEAPP_MESSAGE_DECLARE_ARGS2(loginBaseapp,										NETWORK_VARIABLE_MESSAGE,
									std::string,									accountName,
									std::string,									password)

	// 前端请求从网关登出。
	BASEAPP_MESSAGE_EXPOSED(logoutBaseapp)
	BASEAPP_MESSAGE_DECLARE_ARGS2(logoutBaseapp,									NETWORK_FIXED_MESSAGE,
									uint64,											key,
									ENTITY_ID,										entityID)

	// 前端请求重新登录到网关上。
	BASEAPP_MESSAGE_EXPOSED(reloginBaseapp)
	BASEAPP_MESSAGE_DECLARE_ARGS4(reloginBaseapp,									NETWORK_VARIABLE_MESSAGE,
									std::string,									accountName,
									std::string,									password,
									uint64,											key,
									ENTITY_ID,										entityID)

	// 从dbmgr获取到账号Entity信息
	BASEAPP_MESSAGE_DECLARE_STREAM(onQueryAccountCBFromDbmgr,						NETWORK_VARIABLE_MESSAGE)

	// entity收到远程call请求, 由某个app上的entitycall发起
	BASEAPP_MESSAGE_DECLARE_STREAM(onEntityCall,									NETWORK_VARIABLE_MESSAGE)
	
	// client访问entity的cell方法
	BASEAPP_MESSAGE_EXPOSED(onRemoteCallCellMethodFromClient)
	BASEAPP_MESSAGE_DECLARE_STREAM(onRemoteCallCellMethodFromClient,				NETWORK_VARIABLE_MESSAGE)

	// client更新数据
	BASEAPP_MESSAGE_EXPOSED(onUpdateDataFromClient)
	BASEAPP_MESSAGE_DECLARE_STREAM(onUpdateDataFromClient,							NETWORK_VARIABLE_MESSAGE)
	BASEAPP_MESSAGE_EXPOSED(onUpdateDataFromClientForControlledEntity)
	BASEAPP_MESSAGE_DECLARE_STREAM(onUpdateDataFromClientForControlledEntity,		NETWORK_VARIABLE_MESSAGE)

	// executeRawDatabaseCommand从dbmgr的回调
	BASEAPP_MESSAGE_DECLARE_STREAM(onExecuteRawDatabaseCommandCB,					NETWORK_VARIABLE_MESSAGE)

	// cellapp备份entity的cell数据
	BASEAPP_MESSAGE_DECLARE_STREAM(onBackupEntityCellData,							NETWORK_VARIABLE_MESSAGE)

	// cellapp writeToDB完成
	BASEAPP_MESSAGE_DECLARE_STREAM(onCellWriteToDBCompleted,						NETWORK_VARIABLE_MESSAGE)

	// cellapp转发entity消息给client
	BASEAPP_MESSAGE_DECLARE_STREAM(forwardMessageToClientFromCellapp,				NETWORK_VARIABLE_MESSAGE)

	// cellapp转发entity消息给某个baseEntity的cellEntity
	BASEAPP_MESSAGE_DECLARE_STREAM(forwardMessageToCellappFromCellapp,				NETWORK_VARIABLE_MESSAGE)

	// 请求关闭服务器
	BASEAPP_MESSAGE_DECLARE_STREAM(reqCloseServer,									NETWORK_VARIABLE_MESSAGE)

	// 写entity到db回调。
	BASEAPP_MESSAGE_DECLARE_ARGS5(onWriteToDBCallback,								NETWORK_FIXED_MESSAGE,
									ENTITY_ID,										eid,
									DBID,											entityDBID,
									uint16,											dbInterfaceIndex,
									CALLBACK_ID,									callbackID,
									bool,											success)

	// createEntityFromDBID的回调
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateEntityFromDBIDCallback,					NETWORK_FIXED_MESSAGE)

	// createEntityAnywhereFromDBID的回调
	BASEAPP_MESSAGE_DECLARE_STREAM(onGetCreateEntityAnywhereFromDBIDBestBaseappID,	NETWORK_FIXED_MESSAGE)

	// createEntityAnywhereFromDBID的回调
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateEntityAnywhereFromDBIDCallback,			NETWORK_FIXED_MESSAGE)

	// createEntityAnywhereFromDBID的回调
	BASEAPP_MESSAGE_DECLARE_STREAM(createEntityAnywhereFromDBIDOtherBaseapp,		NETWORK_FIXED_MESSAGE)

	// createEntityAnywhereFromDBID的回调
	BASEAPP_MESSAGE_DECLARE_ARGS5(onCreateEntityAnywhereFromDBIDOtherBaseappCallback,NETWORK_VARIABLE_MESSAGE,
									COMPONENT_ID,									createByBaseappID,
									std::string,									entityType,
									ENTITY_ID,										createdEntityID,
									CALLBACK_ID,									callbackID,
									DBID,											dbid)

	// createEntityRemotelyFromDBID的回调
	BASEAPP_MESSAGE_DECLARE_STREAM(onCreateEntityRemotelyFromDBIDCallback,			NETWORK_FIXED_MESSAGE)

	// createEntityRemotelyFromDBID的回调
	BASEAPP_MESSAGE_DECLARE_STREAM(createEntityRemotelyFromDBIDOtherBaseapp,		NETWORK_FIXED_MESSAGE)

	// createEntityRemotelyFromDBID的回调
	BASEAPP_MESSAGE_DECLARE_ARGS5(onCreateEntityRemotelyFromDBIDOtherBaseappCallback,NETWORK_VARIABLE_MESSAGE,
									COMPONENT_ID,									createByBaseappID,
									std::string,									entityType,
									ENTITY_ID,										createdEntityID,
									CALLBACK_ID,									callbackID,
									DBID,											dbid)

	// 请求查询watcher数据
	BASEAPP_MESSAGE_DECLARE_STREAM(queryWatcher,									NETWORK_VARIABLE_MESSAGE)

	// 充值回调
	BASEAPP_MESSAGE_DECLARE_STREAM(onChargeCB,										NETWORK_VARIABLE_MESSAGE)

	// 开始profile
	BASEAPP_MESSAGE_DECLARE_STREAM(startProfile,									NETWORK_VARIABLE_MESSAGE)

	// 请求从数据库删除实体
	BASEAPP_MESSAGE_DECLARE_STREAM(deleteEntityByDBIDCB,							NETWORK_VARIABLE_MESSAGE)
	
	// lookUpEntityByDBID的回调
	BASEAPP_MESSAGE_DECLARE_STREAM(lookUpEntityByDBIDCB,							NETWORK_VARIABLE_MESSAGE)

	// 某个baseapp上的space恢复了cell， 判断当前baseapp是否有相关entity需要恢复cell
	BASEAPP_MESSAGE_DECLARE_STREAM(onRestoreSpaceCellFromOtherBaseapp,				NETWORK_VARIABLE_MESSAGE)

	// 请求在其他APP灾难恢复返回结果。
	BASEAPP_MESSAGE_DECLARE_STREAM(onRequestRestoreCB,								NETWORK_VARIABLE_MESSAGE)

	// 请求绑定email
	BASEAPP_MESSAGE_EXPOSED(reqAccountBindEmail)
	BASEAPP_MESSAGE_DECLARE_ARGS3(reqAccountBindEmail,								NETWORK_VARIABLE_MESSAGE,
									ENTITY_ID,										entityID,
									std::string,									password,
									std::string,									email)

	// 请求绑定email申请的回调
	BASEAPP_MESSAGE_DECLARE_ARGS5(onReqAccountBindEmailCBFromDBMgr,					NETWORK_VARIABLE_MESSAGE,
									ENTITY_ID,										entityID,
									std::string,									accountName,
									std::string,									email,
									SERVER_ERROR_CODE,								failedcode,
									std::string,									code)

	// baseapp请求绑定email（返回时需要找到loginapp的地址）
	BASEAPP_MESSAGE_DECLARE_ARGS7(onReqAccountBindEmailCBFromBaseappmgr,			NETWORK_VARIABLE_MESSAGE,
									ENTITY_ID,										entityID,
									std::string,									accountName,
									std::string,									email,
									SERVER_ERROR_CODE,								failedcode,
									std::string,									code,
									std::string,									loginappCBHost,
									uint16,											loginappCBPort)

	// 请求修改密码
	BASEAPP_MESSAGE_EXPOSED(reqAccountNewPassword)
	BASEAPP_MESSAGE_DECLARE_ARGS3(reqAccountNewPassword,							NETWORK_VARIABLE_MESSAGE,
									ENTITY_ID,										entityID,
									std::string,									oldpassword,
									std::string,									newpassword)

	// 请求修改密码的回调
	BASEAPP_MESSAGE_DECLARE_ARGS3(onReqAccountNewPasswordCB,						NETWORK_VARIABLE_MESSAGE,
									ENTITY_ID,										entityID,
									std::string,									accountName,
									SERVER_ERROR_CODE,								failedcode)

	// 请求强制杀死当前app
	BASEAPP_MESSAGE_DECLARE_STREAM(reqKillServer,									NETWORK_VARIABLE_MESSAGE)

	//--------------------------------------------Entity----------------------------------------------------------
	// 远程呼叫entity方法
	ENTITY_MESSAGE_EXPOSED(onRemoteMethodCall)
	ENTITY_MESSAGE_DECLARE_STREAM(onRemoteMethodCall,								NETWORK_VARIABLE_MESSAGE)

	// cellapp通报该entity的cell部分销毁或者丢失
	ENTITY_MESSAGE_DECLARE_STREAM(onLoseCell,										NETWORK_VARIABLE_MESSAGE)

	// 客户端直接发送消息给cell实体
	ENTITY_MESSAGE_EXPOSED(forwardEntityMessageToCellappFromClient)
	ENTITY_MESSAGE_DECLARE_STREAM(forwardEntityMessageToCellappFromClient,			NETWORK_VARIABLE_MESSAGE)

	// 某个entity请求teleport后的回调结果
	ENTITY_MESSAGE_DECLARE_ARGS2(onTeleportCB,										NETWORK_FIXED_MESSAGE,
								SPACE_ID,											spaceID,
								bool,												fromCellTeleport)

	// 某个entity请求teleport后的回调结果
	ENTITY_MESSAGE_DECLARE_ARGS1(onGetDBID,											NETWORK_FIXED_MESSAGE,
								DBID,												dbid)

	// entity请求迁移到另一个cellapp上的space过程开始
	ENTITY_MESSAGE_DECLARE_ARGS2(onMigrationCellappStart,							NETWORK_FIXED_MESSAGE,
								COMPONENT_ID,										sourceCellAppID,
								COMPONENT_ID,										targetCellAppID)
		
	// entity请求迁移到另一个cellapp上的space过程结束
	ENTITY_MESSAGE_DECLARE_ARGS2(onMigrationCellappEnd,								NETWORK_FIXED_MESSAGE,
								COMPONENT_ID,										sourceCellAppID,
								COMPONENT_ID,										targetCellAppID)

	//--------------------------------------------Proxy---------------------------------------------------------
	/**
		远程呼叫entity方法
	*/
	//PROXY_MESSAGE_EXPOSED(onClientGetCell)
	//PROXY_MESSAGE_DECLARE_ARGS0(onClientGetCell,									NETWORK_FIXED_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
