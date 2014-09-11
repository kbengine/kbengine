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
	#undef KBE_MACHINE_INTERFACE_HPP
#endif


#ifndef KBE_MACHINE_INTERFACE_HPP
#define KBE_MACHINE_INTERFACE_HPP

// common include	
#if defined(MACHINE)
#include "machine.hpp"
#endif
#include "machine_interface_macros.hpp"
#include "network/interface_defs.hpp"
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
	MACHINE_MESSAGE_DECLARE_ARGS22(onBroadcastInterface,			MERCURY_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									int8,							componentType, 
									uint64,							componentID, 
									uint64,							componentIDEx, 
									int8,							globalorderid, 
									int8,							grouporderid, 
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
									uint64,							extradata3)
	
	// 其他组件向app请求获取某个组件类别的地址
	MACHINE_MESSAGE_DECLARE_ARGS7(onFindInterfaceAddr,				MERCURY_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									int8,							componentType, 
									uint64,							componentID, 
									int8,							findComponentType,
									uint32,							addr, 
									uint16,							finderRecvPort)
						
	// 查询所有接口信息
	MACHINE_MESSAGE_DECLARE_ARGS3(onQueryAllInterfaceInfos,			MERCURY_VARIABLE_MESSAGE,
									int32,							uid, 
									std::string,					username,
									uint16,							finderRecvPort)
						
	// 某app主动请求look。
	MACHINE_MESSAGE_DECLARE_ARGS0(lookApp,							MERCURY_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	MACHINE_MESSAGE_DECLARE_ARGS0(queryLoad,						MERCURY_FIXED_MESSAGE)

	// 启动服务器
	MACHINE_MESSAGE_DECLARE_STREAM(startserver,						MERCURY_VARIABLE_MESSAGE)

	// 关闭服务器
	MACHINE_MESSAGE_DECLARE_STREAM(stopserver,						MERCURY_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	MACHINE_MESSAGE_DECLARE_STREAM(reqKillServer,					MERCURY_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
