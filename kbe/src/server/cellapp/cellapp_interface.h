// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_CELLAPP_INTERFACE_H
#endif


#ifndef KBE_CELLAPP_INTERFACE_H
#define KBE_CELLAPP_INTERFACE_H

// common include	
#if defined(CELLAPP)
#include "entity.h"
#include "cellapp.h"
#endif
#include "cellapp_interface_macros.h"
#include "entity_interface_macros.h"
#include "network/interface_defs.h"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	cellapp所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(CellappInterface)
	// 某app注册自己的接口地址到本app
	CELLAPP_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,								NETWORK_VARIABLE_MESSAGE,
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

	// 某app主动请求look。
	CELLAPP_MESSAGE_DECLARE_ARGS0(lookApp,											NETWORK_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	CELLAPP_MESSAGE_DECLARE_ARGS0(queryLoad,										NETWORK_FIXED_MESSAGE)

	// console远程执行python语句。
	CELLAPP_MESSAGE_DECLARE_STREAM(onExecScriptCommand,								NETWORK_VARIABLE_MESSAGE)

	// dbmgr告知已经启动的其他baseapp或者cellapp的地址
	// 当前app需要主动的去与他们建立连接
	CELLAPP_MESSAGE_DECLARE_ARGS11(onGetEntityAppFromDbmgr,							NETWORK_VARIABLE_MESSAGE,
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
	CELLAPP_MESSAGE_DECLARE_ARGS2(onReqAllocEntityID,								NETWORK_FIXED_MESSAGE,
									ENTITY_ID,										startID,
									ENTITY_ID,										endID)

	// 某app请求获取一个entityID段的回调
	CELLAPP_MESSAGE_DECLARE_ARGS6(onDbmgrInitCompleted,								NETWORK_VARIABLE_MESSAGE,
									GAME_TIME,										gametime, 
									ENTITY_ID,										startID,
									ENTITY_ID,										endID,
									COMPONENT_ORDER,								startGlobalOrder,
									COMPONENT_ORDER,								startGroupOrder,
									std::string,									digest)

	// global数据改变
	CELLAPP_MESSAGE_DECLARE_STREAM(onBroadcastGlobalDataChanged,					NETWORK_VARIABLE_MESSAGE)
	CELLAPP_MESSAGE_DECLARE_STREAM(onBroadcastCellAppDataChanged,					NETWORK_VARIABLE_MESSAGE)

	// baseEntity请求创建在一个新的space中。
	CELLAPP_MESSAGE_DECLARE_STREAM(onCreateCellEntityInNewSpaceFromBaseapp,			NETWORK_VARIABLE_MESSAGE)

	// baseEntity请求恢复在一个新的space中。
	CELLAPP_MESSAGE_DECLARE_STREAM(onRestoreSpaceInCellFromBaseapp,					NETWORK_VARIABLE_MESSAGE)

	// 其他APP请求在此灾难恢复。
	CELLAPP_MESSAGE_DECLARE_STREAM(requestRestore,									NETWORK_VARIABLE_MESSAGE)
	
	// baseapp请求在这个cellapp上创建一个entity。
	CELLAPP_MESSAGE_DECLARE_STREAM(onCreateCellEntityFromBaseapp,					NETWORK_VARIABLE_MESSAGE)

	// 销毁某个cellEntity。
	CELLAPP_MESSAGE_DECLARE_ARGS1(onDestroyCellEntityFromBaseapp,					NETWORK_FIXED_MESSAGE,
									ENTITY_ID,										eid)

	// 某个app向本app告知处于活动状态。
	CELLAPP_MESSAGE_DECLARE_ARGS2(onAppActiveTick,									NETWORK_FIXED_MESSAGE,
									COMPONENT_TYPE,									componentType, 
									COMPONENT_ID,									componentID)

	// entity收到远程call请求, 由某个app上的entitycall发起
	CELLAPP_MESSAGE_DECLARE_STREAM(onEntityCall,									NETWORK_VARIABLE_MESSAGE)

	// client访问entity的cell方法
	CELLAPP_MESSAGE_DECLARE_STREAM(onRemoteCallMethodFromClient,					NETWORK_VARIABLE_MESSAGE)

	// client更新数据
	CELLAPP_MESSAGE_DECLARE_STREAM(onUpdateDataFromClient,							NETWORK_VARIABLE_MESSAGE)
	CELLAPP_MESSAGE_DECLARE_STREAM(onUpdateDataFromClientForControlledEntity,		NETWORK_VARIABLE_MESSAGE)

	// executeRawDatabaseCommand从dbmgr的回调
	CELLAPP_MESSAGE_DECLARE_STREAM(onExecuteRawDatabaseCommandCB,					NETWORK_VARIABLE_MESSAGE)

	// base请求获取celldata
	CELLAPP_MESSAGE_DECLARE_STREAM(reqBackupEntityCellData,							NETWORK_VARIABLE_MESSAGE)

	// base请求获取WriteToDB
	CELLAPP_MESSAGE_DECLARE_STREAM(reqWriteToDBFromBaseapp,							NETWORK_VARIABLE_MESSAGE)

	// 客户端直接发送消息给cell实体
	CELLAPP_MESSAGE_DECLARE_STREAM(forwardEntityMessageToCellappFromClient,			NETWORK_VARIABLE_MESSAGE)

	// 请求关闭服务器
	CELLAPP_MESSAGE_DECLARE_STREAM(reqCloseServer,									NETWORK_VARIABLE_MESSAGE)

	// 请求查询watcher数据
	CELLAPP_MESSAGE_DECLARE_STREAM(queryWatcher,									NETWORK_VARIABLE_MESSAGE)

	// 开始profile
	CELLAPP_MESSAGE_DECLARE_STREAM(startProfile,									NETWORK_VARIABLE_MESSAGE)

	// 请求teleport到当前cellapp上
	CELLAPP_MESSAGE_DECLARE_STREAM(reqTeleportToCellApp,							NETWORK_VARIABLE_MESSAGE)

	// entity传送到目的cellapp上的space之后， 返回给之前cellapp的回调
	CELLAPP_MESSAGE_DECLARE_STREAM(reqTeleportToCellAppCB,							NETWORK_VARIABLE_MESSAGE)

	// 当跨cellapp传送后需要baseapp设置完成状态再清除cellapp记录的标记，此后cellapp才可以继续teleport
	CELLAPP_MESSAGE_DECLARE_STREAM(reqTeleportToCellAppOver,						NETWORK_VARIABLE_MESSAGE)
		
	// real请求更新属性到ghost
	CELLAPP_MESSAGE_DECLARE_STREAM(onUpdateGhostPropertys,							NETWORK_VARIABLE_MESSAGE)
	
	// ghost请求调用def方法real
	CELLAPP_MESSAGE_DECLARE_STREAM(onRemoteRealMethodCall,							NETWORK_VARIABLE_MESSAGE)

	// real请求更新易变数据到ghost
	CELLAPP_MESSAGE_DECLARE_STREAM(onUpdateGhostVolatileData,						NETWORK_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	CELLAPP_MESSAGE_DECLARE_STREAM(reqKillServer,									NETWORK_VARIABLE_MESSAGE)

	// 工具请求改变space查看器（含添加和删除功能）
	CELLAPP_MESSAGE_DECLARE_STREAM(setSpaceViewer,									NETWORK_VARIABLE_MESSAGE)

	//--------------------------------------------Entity----------------------------------------------------------
	//远程呼叫entity方法
	ENTITY_MESSAGE_DECLARE_STREAM(onRemoteMethodCall,								NETWORK_VARIABLE_MESSAGE)

	//客户端设置新位置
	ENTITY_MESSAGE_DECLARE_ARGS2(setPosition_XZ_int,								NETWORK_FIXED_MESSAGE,
									int32,											x, 
									int32,											z)
	
	//客户端设置新位置
	ENTITY_MESSAGE_DECLARE_ARGS3(setPosition_XYZ_int,								NETWORK_FIXED_MESSAGE,
									int32,											x, 
									int32,											y, 
									int32,											z)

	//客户端设置新位置
	ENTITY_MESSAGE_DECLARE_ARGS2(setPosition_XZ_float,								NETWORK_FIXED_MESSAGE,
									float,											x, 
									float,											z)

	//客户端设置新位置
	ENTITY_MESSAGE_DECLARE_ARGS3(setPosition_XYZ_float,								NETWORK_FIXED_MESSAGE,
									float,											x, 
									float,											y, 
									float,											z)

	//entity绑定了一个观察者(客户端)
	ENTITY_MESSAGE_DECLARE_ARGS0(onGetWitnessFromBase,								NETWORK_FIXED_MESSAGE)

	//entity丢失了一个观察者(客户端)
	ENTITY_MESSAGE_DECLARE_ARGS0(onLoseWitness,										NETWORK_FIXED_MESSAGE)
NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
