// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_MACHINE_INTERFACE_H
#endif


#ifndef KBE_MACHINE_INTERFACE_H
#define KBE_MACHINE_INTERFACE_H

// common include	
#if defined(MACHINE)
#include "machine.h"
#endif
#include "machine_interface_macros.h"
#include "network/interface_defs.h"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	machine所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(MachineInterface)
	// 其他组件向app广播自己的接口地址
	MACHINE_MESSAGE_DECLARE_ARGS25(onBroadcastInterface,			NETWORK_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									COMPONENT_TYPE,					componentType, 
									COMPONENT_ID,					componentID, 
									COMPONENT_ID,					componentIDEx, 
									COMPONENT_ORDER,				globalorderid, 
									COMPONENT_ORDER,				grouporderid, 
									COMPONENT_GUS,					gus,
									uint32,							intaddr, 
									uint16,							intport,
									uint32,							extaddr, 
									uint16,							extport,
									std::string,					extaddrEx,
									uint32,							pid,
									float,							cpu, 
									float,							mem, 
									uint32,							usedmem,
									int8,							state,
									uint32,							machineID, 
									uint64,							extradata,
									uint64,							extradata1,
									uint64,							extradata2,
									uint64,							extradata3,
									uint32,							backRecvAddr,
									uint16,							backRecvPort)
	
	// 其他组件向app请求获取某个组件类别的地址
	MACHINE_MESSAGE_DECLARE_ARGS7(onFindInterfaceAddr,				NETWORK_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									COMPONENT_TYPE,					componentType, 
									COMPONENT_ID,					componentID, 
									COMPONENT_TYPE,					findComponentType,
									uint32,							addr, 
									uint16,							finderRecvPort)
						
	// 查询所有接口信息
	MACHINE_MESSAGE_DECLARE_ARGS3(onQueryAllInterfaceInfos,			NETWORK_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									uint16,							finderRecvPort)
		
	// 查询所有machine进程
	MACHINE_MESSAGE_DECLARE_ARGS3(onQueryMachines,					NETWORK_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									uint16,							finderRecvPort)

	MACHINE_MESSAGE_DECLARE_ARGS6(queryComponentID,					NETWORK_VARIABLE_MESSAGE,
									COMPONENT_TYPE,					componentType,
									COMPONENT_ID,					componentID,
									int32,							uid,
									uint16,							finderRecvPort,
									int,							macMD5,
									int32,							pid)
	// 某app主动请求look。
	MACHINE_MESSAGE_DECLARE_ARGS0(lookApp,							NETWORK_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	MACHINE_MESSAGE_DECLARE_ARGS0(queryLoad,						NETWORK_FIXED_MESSAGE)

	// 启动服务器
	MACHINE_MESSAGE_DECLARE_STREAM(startserver,						NETWORK_VARIABLE_MESSAGE)

	// 关闭服务器
	MACHINE_MESSAGE_DECLARE_STREAM(stopserver,						NETWORK_VARIABLE_MESSAGE)

	// 关闭服务器
	MACHINE_MESSAGE_DECLARE_STREAM(killserver,						NETWORK_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	MACHINE_MESSAGE_DECLARE_STREAM(reqKillServer,					NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
