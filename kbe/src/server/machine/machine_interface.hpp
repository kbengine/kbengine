/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
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

NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}
#endif
