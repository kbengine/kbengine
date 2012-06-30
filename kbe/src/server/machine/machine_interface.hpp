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
	#undef __MACHINE_INTERFACE_H__
#endif


#ifndef __MACHINE_INTERFACE_H__
#define __MACHINE_INTERFACE_H__

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
	MACHINE_MESSAGE_DECLARE_ARGS6(onBroadcastInterface,	MERCURY_VARIABLE_MESSAGE,
									int32,				uid, 
									std::string,		username,
									int8,				componentType, 
									int32,				componentID, 
									uint32,				addr, 
									uint16,				port)
	
	// 其他组件向app请求获取某个组件类别的地址
	MACHINE_MESSAGE_DECLARE_ARGS6(onFindInterfaceAddr,	MERCURY_VARIABLE_MESSAGE,
									int32,				uid, 
									std::string,		username,
									int8,				componentType, 
									int8,				findComponentType,
									uint32,				addr, 
									uint16,				finderRecvPort)
									
NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
