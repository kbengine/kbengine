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
	#undef KBE_BOTS_INTERFACE_HPP
#endif


#ifndef KBE_BOTS_INTERFACE_HPP
#define KBE_BOTS_INTERFACE_HPP

// common include	
#if defined(BOTS)
#include "bots.hpp"
#endif
#include "bots_interface_macros.hpp"
#include "network/interface_defs.hpp"
#include "client_lib/common.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Bots所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(BotsInterface)

	// 某app主动请求look。
	BOTS_MESSAGE_DECLARE_ARGS0(lookApp,									MERCURY_FIXED_MESSAGE)

	// 请求关闭服务器
	BOTS_MESSAGE_DECLARE_STREAM(reqCloseServer,							MERCURY_VARIABLE_MESSAGE)

	// console远程执行python语句。
	BOTS_MESSAGE_DECLARE_STREAM(onExecScriptCommand,					MERCURY_VARIABLE_MESSAGE)

	// 某个app向本app告知处于活动状态。
	BOTS_MESSAGE_DECLARE_ARGS2(onAppActiveTick,							MERCURY_FIXED_MESSAGE,
								COMPONENT_TYPE,							componentType, 
								COMPONENT_ID,							componentID)

	// 添加bots。
	BOTS_MESSAGE_DECLARE_STREAM(addBots,								MERCURY_VARIABLE_MESSAGE)

	// 请求查询watcher数据
	BOTS_MESSAGE_DECLARE_STREAM(queryWatcher,							MERCURY_VARIABLE_MESSAGE)

	// 开始profile
	BOTS_MESSAGE_DECLARE_STREAM(startProfile,							MERCURY_VARIABLE_MESSAGE)

	// 请求强制杀死当前app
	BOTS_MESSAGE_DECLARE_STREAM(reqKillServer,							MERCURY_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
