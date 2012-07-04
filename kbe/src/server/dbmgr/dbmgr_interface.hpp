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
	#undef __DBMGR_INTERFACE_H__
#endif


#ifndef __DBMGR_INTERFACE_H__
#define __DBMGR_INTERFACE_H__

// common include	
#if defined(DBMGR)
#include "dbmgr.hpp"
#endif
#include "dbmgr_interface_macros.hpp"
#include "network/interface_defs.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Dbmgr消息宏，  参数为流， 需要自己解开
*/

/**
	DBMGR所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(DbmgrInterface)
	// 某app注册自己的接口地址到本app
	DBMGR_MESSAGE_DECLARE_ARGS6(onRegisterNewApp,	MERCURY_VARIABLE_MESSAGE,
									int32,				uid, 
									std::string,		username,
									int8,				componentType, 
									uint64,				componentID, 
									uint32,				addr, 
									uint16,				port)
NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
