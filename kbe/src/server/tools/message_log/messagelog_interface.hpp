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
	#undef KBE_MESSAGELOG_INTERFACE_HPP
#endif


#ifndef KBE_MESSAGELOG_INTERFACE_HPP
#define KBE_MESSAGELOG_INTERFACE_HPP

// common include	
#if defined(MESSAGELOG)
#include "messagelog.hpp"
#endif
#include "messagelog_interface_macros.hpp"
#include "network/interface_defs.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Messagelog所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(MessagelogInterface)
	// 某app注册自己的接口地址到本app
	MESSAGELOG_MESSAGE_DECLARE_ARGS11(onRegisterNewApp,						MERCURY_VARIABLE_MESSAGE,
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
									std::string,							extAddrEx)

	// 某app主动请求look。
	MESSAGELOG_MESSAGE_DECLARE_ARGS0(lookApp,								MERCURY_FIXED_MESSAGE)

	// 某个app请求查看该app负载状态。
	MESSAGELOG_MESSAGE_DECLARE_ARGS0(queryLoad,								MERCURY_FIXED_MESSAGE)

	// 某个app向本app告知处于活动状态。
	MESSAGELOG_MESSAGE_DECLARE_ARGS2(onAppActiveTick,						MERCURY_FIXED_MESSAGE,
									COMPONENT_TYPE,							componentType, 
									COMPONENT_ID,							componentID)

	// 远程写日志
	MESSAGELOG_MESSAGE_DECLARE_STREAM(writeLog,								MERCURY_VARIABLE_MESSAGE)

	// 注册log监听者
	MESSAGELOG_MESSAGE_DECLARE_STREAM(registerLogWatcher,					MERCURY_VARIABLE_MESSAGE)

	// 请求关闭服务器
	MESSAGELOG_MESSAGE_DECLARE_STREAM(reqCloseServer,						MERCURY_VARIABLE_MESSAGE)

	// 请求查询watcher数据
	MESSAGELOG_MESSAGE_DECLARE_STREAM(queryWatcher,							MERCURY_VARIABLE_MESSAGE)

	// 开始profile
	MESSAGELOG_MESSAGE_DECLARE_STREAM(startProfile,							MERCURY_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	MESSAGELOG_MESSAGE_DECLARE_STREAM(reqKillServer,						MERCURY_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
