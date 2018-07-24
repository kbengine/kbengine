// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_CELLAPPMGR_INTERFACE_H
#endif


#ifndef KBE_CELLAPPMGR_INTERFACE_H
#define KBE_CELLAPPMGR_INTERFACE_H

// common include	
#if defined(CELLAPPMGR)
#include "cellappmgr.h"
#endif
#include "cellappmgr_interface_macros.h"
#include "network/interface_defs.h"
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
NETWORK_INTERFACE_DECLARE_BEGIN(CellappmgrInterface)
	// 某app注册自己的接口地址到本app
	CELLAPPMGR_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,						NETWORK_VARIABLE_MESSAGE,
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
									std::string,							extaddrEx)

	// 某app主动请求look。
	CELLAPPMGR_MESSAGE_DECLARE_ARGS0(lookApp,								NETWORK_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	CELLAPPMGR_MESSAGE_DECLARE_ARGS0(queryLoad,								NETWORK_FIXED_MESSAGE)

	// 某个app向本app告知处于活动状态。
	CELLAPPMGR_MESSAGE_DECLARE_ARGS2(onAppActiveTick,						NETWORK_FIXED_MESSAGE,
									COMPONENT_TYPE,							componentType, 
									COMPONENT_ID,							componentID)

	// baseEntity请求创建在一个新的space中。
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(reqCreateCellEntityInNewSpace,		NETWORK_VARIABLE_MESSAGE)

	// baseEntity请求恢复在一个新的space中。
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(reqRestoreSpaceInCell,				NETWORK_VARIABLE_MESSAGE)

	// 消息转发， 由某个app想通过本app将消息转发给某个app。
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(forwardMessage,						NETWORK_VARIABLE_MESSAGE)

	// 请求关闭服务器
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(reqCloseServer,						NETWORK_VARIABLE_MESSAGE)

	// 请求查询watcher数据
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(queryWatcher,							NETWORK_VARIABLE_MESSAGE)

	// 更新cellapp信息。
	CELLAPPMGR_MESSAGE_DECLARE_ARGS4(updateCellapp,							NETWORK_FIXED_MESSAGE,
									COMPONENT_ID,							componentID,
									ENTITY_ID,								numEntities,
									float,									load,
									uint32,									flags)

	// 开始profile
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(startProfile,							NETWORK_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(reqKillServer,						NETWORK_VARIABLE_MESSAGE)

	// cellapp同步自己的初始化信息
	CELLAPPMGR_MESSAGE_DECLARE_ARGS4(onCellappInitProgress,					NETWORK_FIXED_MESSAGE,
									COMPONENT_ID,							cid,
									float,									progress,
									COMPONENT_ORDER,						componentGlobalOrder,
									COMPONENT_ORDER,						componentGroupOrder)

	// 查询所有相关进程负载信息
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(queryAppsLoads,						NETWORK_VARIABLE_MESSAGE)

	// 查询所有相关进程space信息
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(querySpaces,							NETWORK_VARIABLE_MESSAGE)

	// 更新相关进程space信息，注意：此spaceData并非API文档中描述的spaceData
	// 是指space的一些信息
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(updateSpaceData,						NETWORK_VARIABLE_MESSAGE)

	// 工具请求改变space查看器（含添加和删除功能）
	CELLAPPMGR_MESSAGE_DECLARE_STREAM(setSpaceViewer,						NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
