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
	#undef NETWORK_MESSAGE_DECLARE_STREAM
	#undef NETWORK_MESSAGE_DECLARE_ARGS1
	#undef NETWORK_MESSAGE_DECLARE_ARGS2
	#undef NETWORK_MESSAGE_DECLARE_ARGS3
	#undef NETWORK_MESSAGE_DECLARE_ARGS4
	#undef NETWORK_MESSAGE_DECLARE_ARGS5
	#undef NETWORK_MESSAGE_HANDLER
	#undef NETWORK_INTERFACE_DECLARE_END
	#undef MESSAGE_STREAM
	#undef MESSAGE_ARGS1
	#undef MESSAGE_ARGS2
	#undef MESSAGE_ARGS3
	#undef MESSAGE_ARGS4
	#undef MESSAGE_ARGS5
	#undef SET_MESSAGE_FUNC
#endif



#ifdef DEFINE_IN_INTERFACE
	//#undef DEFINE_IN_INTERFACE
	
	#define NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, HANDLER_TYPE, MSG_LENGTH, ARG_N)						\
		HANDLER_TYPE* p##NAME = static_cast<HANDLER_TYPE*>(messageHandlers.add(#DOMAIN"::"#NAME,		\
						 new NAME##Args##ARG_N, MSG_LENGTH, new HANDLER_TYPE));							\
		const HANDLER_TYPE& NAME = *p##NAME;															\
			
		
#else
	#define NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, HANDLER_TYPE, MSG_LENGTH, ARG_N)			\
		extern const HANDLER_TYPE& NAME;													\
	
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
/		流消息 接收消息的接口参数为memorystream, 自己处理
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_STREAM(NAME)
#else
#define MESSAGE_STREAM(NAME)										\
	class NAME##Args_stream : public Mercury::MessageArgs			\
	{																\
	public:															\
		NAME##Args_stream():Mercury::MessageArgs(){}				\
		~NAME##Args_stream(){}										\
																	\
		virtual void writeToStream(MemoryStream& s)					\
		{															\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_STREAM(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH)				\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 	\
														_stream)	\
	MESSAGE_STREAM(NAME)											\

/**---------------------------------------------------------------------
/		一个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)
#else
#define MESSAGE_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	class NAME##Args1 : public Mercury::MessageArgs					\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
	public:															\
		NAME##Args1():Mercury::MessageArgs(){}						\
		NAME##Args1(ARG_TYPE1 init_##ARG_NAME1):					\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1)									\
		{}															\
		~NAME##Args1(){}											\
																	\
		virtual void writeToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS1(DOMAIN, NAME, MSGHANDLER,		\
											MSG_LENGTH,				\
											ARG_TYPE1, ARG_NAME1)	\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 1)\
	MESSAGE_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)						\

/**---------------------------------------------------------------------
/		二个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2)					\
	
#else
#define MESSAGE_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2)					\
	class NAME##Args2 : public Mercury::MessageArgs					\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
	public:															\
		NAME##Args2():Mercury::MessageArgs(){}						\
		NAME##Args2(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2)									\
		{}															\
		~NAME##Args2(){}											\
																	\
		virtual void writeToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS2(DOMAIN, NAME, MSGHANDLER,		\
											MSG_LENGTH,				\
											ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2)	\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 2)\
	MESSAGE_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 						\
						ARG_TYPE2, ARG_NAME2)						\

/**---------------------------------------------------------------------
/		三个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS3(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3)					\
	
#else
#define MESSAGE_ARGS3(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3)					\
	class NAME##Args3 : public Mercury::MessageArgs					\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
	public:															\
		NAME##Args3():Mercury::MessageArgs(){}						\
		NAME##Args3(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3)									\
		{}															\
		~NAME##Args3(){}											\
																	\
		virtual void writeToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS3(DOMAIN, NAME, MSGHANDLER,		\
											MSG_LENGTH,				\
						ARG_TYPE1, ARG_NAME1,						\
						ARG_TYPE2, ARG_NAME2,						\
						ARG_TYPE3, ARG_NAME3)						\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 3)\
	MESSAGE_ARGS3(NAME, ARG_TYPE1, ARG_NAME1, 						\
						ARG_TYPE2, ARG_NAME2,						\
						ARG_TYPE3, ARG_NAME3)						\

	
/**---------------------------------------------------------------------
/		四个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4)					\
	
#else
#define MESSAGE_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4)					\
	class NAME##Args4 : public Mercury::MessageArgs					\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
	public:															\
		NAME##Args4():Mercury::MessageArgs(){}						\
		NAME##Args4(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4)									\
		{}															\
		~NAME##Args4(){}											\
																	\
		virtual void writeToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS4(DOMAIN, NAME, MSGHANDLER,		\
											MSG_LENGTH,				\
						ARG_TYPE1, ARG_NAME1,						\
						ARG_TYPE2, ARG_NAME2,						\
						ARG_TYPE3, ARG_NAME3,						\
						ARG_TYPE4, ARG_NAME4)						\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 4)\
	MESSAGE_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 						\
						ARG_TYPE2, ARG_NAME2,						\
						ARG_TYPE3, ARG_NAME3,						\
						ARG_TYPE4, ARG_NAME4)						\


/**---------------------------------------------------------------------
/		伍个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5)					\
	
#else
#define MESSAGE_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5)					\
	class NAME##Args5 : public Mercury::MessageArgs					\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
	public:															\
		NAME##Args5():Mercury::MessageArgs(){}						\
		NAME##Args5(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE4 init_##ARG_NAME4):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5)									\
		{}															\
		~NAME##Args5(){}											\
																	\
		virtual void writeToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS5(DOMAIN, NAME, MSGHANDLER,		\
											MSG_LENGTH,				\
						ARG_TYPE1, ARG_NAME1,						\
						ARG_TYPE2, ARG_NAME2,						\
						ARG_TYPE3, ARG_NAME3,						\
						ARG_TYPE4, ARG_NAME4,						\
						ARG_TYPE5, ARG_NAME5)						\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 5)\
	MESSAGE_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 						\
						ARG_TYPE2, ARG_NAME2,						\
						ARG_TYPE3, ARG_NAME3,						\
						ARG_TYPE4, ARG_NAME4,						\
						ARG_TYPE5, ARG_NAME5)						\
	
}
}

