/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __INTERFACE_DEFS_H__
#define __INTERFACE_DEFS_H__
	
// common include
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "network/common.hpp"
#include "network/message_handler.hpp"

//#define NDEBUG
#include <assert.h>
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif

#endif
// __INTERFACE_DEFS_H__
namespace KBEngine{
namespace Mercury
{
#ifdef NETWORK_INTERFACE_DECLARE_BEGIN
	#undef NETWORK_INTERFACE_DECLARE_BEGIN
	#undef NETWORK_MESSAGE_DECLARE_ARGS1
	#undef NETWORK_MESSAGE_HANDLER
	#undef NETWORK_INTERFACE_DECLARE_END
	#undef MESSAGE_ARGS1
	#undef SET_MESSAGE_FUNC
#endif

#ifdef DEFINE_IN_INTERFACE
	//#undef DEFINE_IN_INTERFACE
	
	#define NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, HANDLER_TYPE)												\
		HANDLER_TYPE* p##NAME = static_cast<HANDLER_TYPE*>(messageHandlers.add(#DOMAIN"::"#NAME,			\
						 new NAME##Args, new HANDLER_TYPE));												\
		const HANDLER_TYPE& NAME = *p##NAME;																\
			
		
#else
	#define NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, HANDLER_TYPE)			\
		extern const HANDLER_TYPE& NAME;								\
	
#endif

// 定义接口域名称
#ifndef DEFINE_IN_INTERFACE
#define NETWORK_INTERFACE_DECLARE_BEGIN(INAME) 						\
	namespace INAME													\
{																	\
		extern Mercury::MessageHandlers messageHandlers;			\
			
#else
#define NETWORK_INTERFACE_DECLARE_BEGIN(INAME) 						\
	namespace INAME													\
{																	\
		Mercury::MessageHandlers messageHandlers;					\
			
#endif

#define NETWORK_INTERFACE_DECLARE_END() }

/**---------------------------------------------------------------------
/		只有一个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS1(NAME, ARG_TYPE1, ARG_NAME)
#else
#define MESSAGE_ARGS1(NAME, ARG_TYPE1, ARG_NAME)					\
	class NAME##Args : public Mercury::MessageArgs					\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME;											\
	public:															\
		NAME##Args():Mercury::MessageArgs(){}						\
		~NAME##Args(){}												\
																	\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME;											\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS1(DOMAIN, NAME, MSGHANDLER, ARG_TYPE1, ARG_NAME)	\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER)									\
	MESSAGE_ARGS1(NAME, ARG_TYPE1, ARG_NAME)											\
	
}

}

