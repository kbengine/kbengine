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
	#undef __CELLAPP_INTERFACE_H__
#endif


#ifndef __CELLAPP_INTERFACE_H__
#define __CELLAPP_INTERFACE_H__

// common include	
#if defined(CELLAPP)
#include "entity.hpp"
#include "cellapp.hpp"
#endif
#include "cellapp_interface_macros.hpp"
#include "entity_interface_macros.hpp"
#include "network/interface_defs.hpp"
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
	CELLAPP_MESSAGE_DECLARE_ARGS6(onRegisterNewApp,	MERCURY_VARIABLE_MESSAGE,
									int32,				uid, 
									std::string,		username,
									int8,				componentType, 
									uint64,				componentID, 
									uint32,				addr, 
									uint16,				port)

	ENTITY_MESSAGE_DECLARE_ARGS1(test, MERCURY_VARIABLE_MESSAGE,
								std::string, name
	)
	
	/**
		远程呼叫entity方法
	*/
	ENTITY_MESSAGE_DECLARE_STREAM(onRemoteMethodCall, MERCURY_FIXED_MESSAGE)
	
NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
