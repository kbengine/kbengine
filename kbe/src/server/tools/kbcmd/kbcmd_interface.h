// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_KBCMD_TOOL_INTERFACE_H
#endif


#ifndef KBE_KBCMD_TOOL_INTERFACE_H
#define KBE_KBCMD_TOOL_INTERFACE_H

// common include	
#if defined(INTERFACES)
#include "kbcmd.h"
#endif
#include "kbcmd_interface_macros.h"
#include "network/interface_defs.h"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	KBCMD消息宏，  参数为流， 需要自己解开
*/

/**
	KBCMD所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(KBCMDInterface)

	// 某app主动请求look。
	KBCMD_MESSAGE_DECLARE_ARGS0(lookApp, NETWORK_FIXED_MESSAGE)

	// 某个app向本app告知处于活动状态。
	KBCMD_MESSAGE_DECLARE_ARGS2(onAppActiveTick, NETWORK_FIXED_MESSAGE,
		COMPONENT_TYPE, componentType,
		COMPONENT_ID, componentID)

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}

#endif // KBE_KBCMD_TOOL_INTERFACE_H
