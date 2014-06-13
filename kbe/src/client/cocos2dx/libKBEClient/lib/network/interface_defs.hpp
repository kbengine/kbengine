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

#ifndef __INTERFACE_DEFS_H__
#define __INTERFACE_DEFS_H__
	
// common include
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "network/common.hpp"
#include "network/message_handler.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
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
	#undef NETWORK_MESSAGE_DECLARE_ARGS0
	#undef NETWORK_MESSAGE_DECLARE_ARGS1
	#undef NETWORK_MESSAGE_DECLARE_ARGS2
	#undef NETWORK_MESSAGE_DECLARE_ARGS3
	#undef NETWORK_MESSAGE_DECLARE_ARGS4
	#undef NETWORK_MESSAGE_DECLARE_ARGS5
	#undef NETWORK_MESSAGE_DECLARE_ARGS6
	#undef NETWORK_MESSAGE_DECLARE_ARGS7
	#undef NETWORK_MESSAGE_DECLARE_ARGS8
	#undef NETWORK_MESSAGE_DECLARE_ARGS9
	#undef NETWORK_MESSAGE_DECLARE_ARGS10
	#undef NETWORK_MESSAGE_DECLARE_ARGS11
	#undef NETWORK_MESSAGE_DECLARE_ARGS12
	#undef NETWORK_MESSAGE_DECLARE_ARGS13
	#undef NETWORK_MESSAGE_DECLARE_ARGS14
	#undef NETWORK_MESSAGE_DECLARE_ARGS15
	#undef NETWORK_MESSAGE_DECLARE_ARGS16
	#undef NETWORK_MESSAGE_DECLARE_ARGS17
	#undef NETWORK_MESSAGE_DECLARE_ARGS18
	#undef NETWORK_MESSAGE_DECLARE_ARGS19
	#undef NETWORK_MESSAGE_DECLARE_ARGS20
	#undef NETWORK_MESSAGE_DECLARE_ARGS21

	#undef NETWORK_MESSAGE_HANDLER
	#undef NETWORK_MESSAGE_EXPOSED
	#undef NETWORK_INTERFACE_DECLARE_END
	
	#undef MESSAGE_STREAM
	#undef MESSAGE_ARGS0
	#undef MESSAGE_ARGS1
	#undef MESSAGE_ARGS2
	#undef MESSAGE_ARGS3
	#undef MESSAGE_ARGS4
	#undef MESSAGE_ARGS5
	#undef MESSAGE_ARGS6
	#undef MESSAGE_ARGS7
	#undef MESSAGE_ARGS8
	#undef MESSAGE_ARGS9
	#undef MESSAGE_ARGS10
	#undef MESSAGE_ARGS11
	#undef MESSAGE_ARGS12
	#undef MESSAGE_ARGS13
	#undef MESSAGE_ARGS14
	#undef MESSAGE_ARGS15
	#undef MESSAGE_ARGS16
	#undef MESSAGE_ARGS17
	#undef MESSAGE_ARGS18
	#undef MESSAGE_ARGS19
	#undef MESSAGE_ARGS20
	#undef MESSAGE_ARGS21
	#undef SET_MESSAGE_FUNC
#endif

#ifdef DEFINE_IN_INTERFACE
	//#undef DEFINE_IN_INTERFACE
	
	#define NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, HANDLER_TYPE, MSG_LENGTH, ARG_N)						\
		HANDLER_TYPE* p##NAME = static_cast<HANDLER_TYPE*>(messageHandlers.add(#DOMAIN"::"#NAME,		\
						 new NAME##Args##ARG_N, MSG_LENGTH, new HANDLER_TYPE));							\
		const HANDLER_TYPE& NAME = *p##NAME;															\
			
	#define NETWORK_MESSAGE_EXPOSED(DOMAIN, NAME);														\
		bool p##DOMAIN##NAME##_exposed = messageHandlers.pushExposedMessage(#DOMAIN"::"#NAME);			\

#else
	#define NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, HANDLER_TYPE, MSG_LENGTH, ARG_N)						\
		extern const HANDLER_TYPE& NAME;																\

	#define NETWORK_MESSAGE_EXPOSED(DOMAIN, NAME)														\
	
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
		virtual int32 dataSize(void)								\
		{															\
			return MERCURY_VARIABLE_MESSAGE;						\
		}															\
		virtual MessageArgs::MESSAGE_ARGS_TYPE type(void)			\
		{															\
			return MESSAGE_ARGS_TYPE_VARIABLE;						\
		}															\
		virtual void addToStream(MemoryStream& s)					\
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
/		零个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS0(NAME)
#else
#define MESSAGE_ARGS0(NAME)											\
	class NAME##Args0 : public Mercury::MessageArgs					\
	{																\
	public:															\
		NAME##Args0():Mercury::MessageArgs(){}						\
		~NAME##Args0(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s)			\
		{															\
		}															\
		static void staticAddToStream(MemoryStream& s)				\
		{															\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return 0;												\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS0(DOMAIN, NAME, MSGHANDLER,		\
											MSG_LENGTH)				\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 0)\
	MESSAGE_ARGS0(NAME)												\

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
		NAME##Args1():Mercury::MessageArgs(){						\
		strArgsTypes.push_back(#ARG_TYPE1);}						\
		NAME##Args1(ARG_TYPE1 init_##ARG_NAME1):					\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1)									\
		{strArgsTypes.push_back(#ARG_TYPE1);}						\
		~NAME##Args1(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1)			\
		{															\
			s << init_##ARG_NAME1;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1)			\
		{															\
			s << init_##ARG_NAME1;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return sizeof(ARG_TYPE1);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
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
		NAME##Args2():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
		}															\
		NAME##Args2(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2)									\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
		}															\
		~NAME##Args2(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
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
		NAME##Args3():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
		}															\
		NAME##Args3(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3)									\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
		}															\
		~NAME##Args3(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
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
		NAME##Args4():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
		}															\
		NAME##Args4(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4)									\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
		}															\
		~NAME##Args4(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
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
		NAME##Args5():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
		}															\
		NAME##Args5(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5)									\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
		}															\
		~NAME##Args5(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
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

/**---------------------------------------------------------------------
/		六个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6)					\
	
#else
#define MESSAGE_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6)					\
	class NAME##Args6 : public Mercury::MessageArgs					\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
	public:															\
		NAME##Args6():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
		}															\
		NAME##Args6(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6)									\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
		}															\
		~NAME##Args6(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS6(DOMAIN, NAME, MSGHANDLER,		\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 6)\
	MESSAGE_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 						\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6)					\

/**---------------------------------------------------------------------
/		七个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS7(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7)					\
	
#else
#define MESSAGE_ARGS7(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7)					\
	class NAME##Args7 : public Mercury::MessageArgs					\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
	public:															\
		NAME##Args7():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
		}															\
		NAME##Args7(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7)									\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
		}															\
		~NAME##Args7(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS7(DOMAIN, NAME, MSGHANDLER,		\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 7)\
	MESSAGE_ARGS7(NAME, ARG_TYPE1, ARG_NAME1, 						\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7)					\
	
/**---------------------------------------------------------------------
/		八个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS8(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8)					\
	
#else
#define MESSAGE_ARGS8(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8)					\
	class NAME##Args8 : public Mercury::MessageArgs					\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
	public:															\
		NAME##Args8():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
		}															\
		NAME##Args8(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8)									\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
		}															\
		~NAME##Args8(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS8(DOMAIN, NAME, MSGHANDLER,		\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 8)\
	MESSAGE_ARGS8(NAME, 					 						\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8)					\
	
/**---------------------------------------------------------------------
/		九个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS9(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9)					\
	
#else
#define MESSAGE_ARGS9(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9)					\
	class NAME##Args9 : public Mercury::MessageArgs					\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
	public:															\
		NAME##Args9():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
		}															\
		NAME##Args9(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9)									\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
		}															\
		~NAME##Args9(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9)			\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS9(DOMAIN, NAME, MSGHANDLER,		\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME, MSGHANDLER, MSG_LENGTH, 9)\
	MESSAGE_ARGS9(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9)					\
	
/**---------------------------------------------------------------------
/		十个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS10(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10)					\
	
#else
#define MESSAGE_ARGS10(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10)					\
	class NAME##Args10 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
	public:															\
		NAME##Args10():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
		}															\
		NAME##Args10(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
		}															\
		~NAME##Args10(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS10(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 10)\
	MESSAGE_ARGS10(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10)					\
	

	
/**---------------------------------------------------------------------
/		十一个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS11(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11)					\
	
#else
#define MESSAGE_ARGS11(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11)					\
	class NAME##Args11 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
	public:															\
		NAME##Args11():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
		}															\
		NAME##Args11(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
		}															\
		~NAME##Args11(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS11(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 11)\
	MESSAGE_ARGS11(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11)					\



/**---------------------------------------------------------------------
/		十二个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS12(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12)					\
	
#else
#define MESSAGE_ARGS12(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12)					\
	class NAME##Args12 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
		ARG_TYPE12 ARG_NAME12;										\
	public:															\
		NAME##Args12():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
		}															\
		NAME##Args12(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11,								\
		ARG_TYPE12 init_##ARG_NAME12):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11),								\
		ARG_NAME12(init_##ARG_NAME12)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
		}															\
		~NAME##Args12(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11) +							\
					sizeof(ARG_TYPE12);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
			s << ARG_NAME12;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
			s >> ARG_NAME12;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS12(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 12)\
	MESSAGE_ARGS12(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12)					\


/**---------------------------------------------------------------------
/		十三个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS13(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13)					\
	
#else
#define MESSAGE_ARGS13(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15)					\
	class NAME##Args13 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
		ARG_TYPE12 ARG_NAME12;										\
		ARG_TYPE13 ARG_NAME13;										\
	public:															\
		NAME##Args13():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
		}															\
		NAME##Args13(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11,								\
		ARG_TYPE12 init_##ARG_NAME12,								\
		ARG_TYPE13 init_##ARG_NAME13):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11),								\
		ARG_NAME12(init_##ARG_NAME12),								\
		ARG_NAME13(init_##ARG_NAME13)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
		}															\
		~NAME##Args13(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11) +							\
					sizeof(ARG_TYPE12) +							\
					sizeof(ARG_TYPE13);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
			s << ARG_NAME12;										\
			s << ARG_NAME13;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
			s >> ARG_NAME12;										\
			s >> ARG_NAME13;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS13(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 13)\
	MESSAGE_ARGS13(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13)					\


/**---------------------------------------------------------------------
/		十四个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS14(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14)					\
	
#else
#define MESSAGE_ARGS14(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14)					\
	class NAME##Args14 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
		ARG_TYPE12 ARG_NAME12;										\
		ARG_TYPE13 ARG_NAME13;										\
		ARG_TYPE14 ARG_NAME14;										\
	public:															\
		NAME##Args14():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
		}															\
		NAME##Args14(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11,								\
		ARG_TYPE12 init_##ARG_NAME12,								\
		ARG_TYPE13 init_##ARG_NAME13,								\
		ARG_TYPE14 init_##ARG_NAME14):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11),								\
		ARG_NAME12(init_##ARG_NAME12),								\
		ARG_NAME13(init_##ARG_NAME13),								\
		ARG_NAME14(init_##ARG_NAME14)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
		}															\
		~NAME##Args14(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11) +							\
					sizeof(ARG_TYPE12) +							\
					sizeof(ARG_TYPE13) +							\
					sizeof(ARG_TYPE14);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
			s << ARG_NAME12;										\
			s << ARG_NAME13;										\
			s << ARG_NAME14;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
			s >> ARG_NAME12;										\
			s >> ARG_NAME13;										\
			s >> ARG_NAME14;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS14(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 14)\
	MESSAGE_ARGS14(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14)					\

/**---------------------------------------------------------------------
/		十五个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS15(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15)					\
	
#else
#define MESSAGE_ARGS15(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15)					\
	class NAME##Args15 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
		ARG_TYPE12 ARG_NAME12;										\
		ARG_TYPE13 ARG_NAME13;										\
		ARG_TYPE14 ARG_NAME14;										\
		ARG_TYPE15 ARG_NAME15;										\
	public:															\
		NAME##Args15():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
		}															\
		NAME##Args15(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11,								\
		ARG_TYPE12 init_##ARG_NAME12,								\
		ARG_TYPE13 init_##ARG_NAME13,								\
		ARG_TYPE14 init_##ARG_NAME14,								\
		ARG_TYPE15 init_##ARG_NAME15):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11),								\
		ARG_NAME12(init_##ARG_NAME12),								\
		ARG_NAME13(init_##ARG_NAME13),								\
		ARG_NAME14(init_##ARG_NAME14),								\
		ARG_NAME15(init_##ARG_NAME15)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
		}															\
		~NAME##Args15(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11) +							\
					sizeof(ARG_TYPE12) +							\
					sizeof(ARG_TYPE13) +							\
					sizeof(ARG_TYPE14) +							\
					sizeof(ARG_TYPE15);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
			s << ARG_NAME12;										\
			s << ARG_NAME13;										\
			s << ARG_NAME14;										\
			s << ARG_NAME15;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
			s >> ARG_NAME12;										\
			s >> ARG_NAME13;										\
			s >> ARG_NAME14;										\
			s >> ARG_NAME15;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS15(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 15)\
	MESSAGE_ARGS15(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15)					\


/**---------------------------------------------------------------------
/		十六个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS16(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16)					\
	
#else
#define MESSAGE_ARGS16(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16)					\
	class NAME##Args16 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
		ARG_TYPE12 ARG_NAME12;										\
		ARG_TYPE13 ARG_NAME13;										\
		ARG_TYPE14 ARG_NAME14;										\
		ARG_TYPE15 ARG_NAME15;										\
		ARG_TYPE16 ARG_NAME16;										\
	public:															\
		NAME##Args16():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
		}															\
		NAME##Args16(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11,								\
		ARG_TYPE12 init_##ARG_NAME12,								\
		ARG_TYPE13 init_##ARG_NAME13,								\
		ARG_TYPE14 init_##ARG_NAME14,								\
		ARG_TYPE15 init_##ARG_NAME15,								\
		ARG_TYPE16 init_##ARG_NAME16):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11),								\
		ARG_NAME12(init_##ARG_NAME12),								\
		ARG_NAME13(init_##ARG_NAME13),								\
		ARG_NAME14(init_##ARG_NAME14),								\
		ARG_NAME15(init_##ARG_NAME15),								\
		ARG_NAME16(init_##ARG_NAME16)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
		}															\
		~NAME##Args16(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11) +							\
					sizeof(ARG_TYPE12) +							\
					sizeof(ARG_TYPE13) +							\
					sizeof(ARG_TYPE14) +							\
					sizeof(ARG_TYPE15) +							\
					sizeof(ARG_TYPE16);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
			s << ARG_NAME12;										\
			s << ARG_NAME13;										\
			s << ARG_NAME14;										\
			s << ARG_NAME15;										\
			s << ARG_NAME16;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
			s >> ARG_NAME12;										\
			s >> ARG_NAME13;										\
			s >> ARG_NAME14;										\
			s >> ARG_NAME15;										\
			s >> ARG_NAME16;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS16(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 16)\
	MESSAGE_ARGS16(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16)					\


/**---------------------------------------------------------------------
/		十七个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS17(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17)					\
	
#else
#define MESSAGE_ARGS17(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17)					\
	class NAME##Args17 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
		ARG_TYPE12 ARG_NAME12;										\
		ARG_TYPE13 ARG_NAME13;										\
		ARG_TYPE14 ARG_NAME14;										\
		ARG_TYPE15 ARG_NAME15;										\
		ARG_TYPE16 ARG_NAME16;										\
		ARG_TYPE17 ARG_NAME17;										\
	public:															\
		NAME##Args17():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
			strArgsTypes.push_back(#ARG_TYPE17);					\
		}															\
		NAME##Args17(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11,								\
		ARG_TYPE12 init_##ARG_NAME12,								\
		ARG_TYPE13 init_##ARG_NAME13,								\
		ARG_TYPE14 init_##ARG_NAME14,								\
		ARG_TYPE15 init_##ARG_NAME15,								\
		ARG_TYPE16 init_##ARG_NAME16,								\
		ARG_TYPE17 init_##ARG_NAME17):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11),								\
		ARG_NAME12(init_##ARG_NAME12),								\
		ARG_NAME13(init_##ARG_NAME13),								\
		ARG_NAME14(init_##ARG_NAME14),								\
		ARG_NAME15(init_##ARG_NAME15),								\
		ARG_NAME16(init_##ARG_NAME16),								\
		ARG_NAME17(init_##ARG_NAME17)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
			strArgsTypes.push_back(#ARG_TYPE17);					\
		}															\
		~NAME##Args17(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16,		\
								ARG_TYPE17 init_##ARG_NAME17)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
			s << init_##ARG_NAME17;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16,		\
								ARG_TYPE17 init_##ARG_NAME17)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
			s << init_##ARG_NAME17;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11) +							\
					sizeof(ARG_TYPE12) +							\
					sizeof(ARG_TYPE13) +							\
					sizeof(ARG_TYPE14) +							\
					sizeof(ARG_TYPE15) +							\
					sizeof(ARG_TYPE16) +							\
					sizeof(ARG_TYPE17);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
			s << ARG_NAME12;										\
			s << ARG_NAME13;										\
			s << ARG_NAME14;										\
			s << ARG_NAME15;										\
			s << ARG_NAME16;										\
			s << ARG_NAME17;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
			s >> ARG_NAME12;										\
			s >> ARG_NAME13;										\
			s >> ARG_NAME14;										\
			s >> ARG_NAME15;										\
			s >> ARG_NAME16;										\
			s >> ARG_NAME17;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS17(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 17)\
	MESSAGE_ARGS17(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17)					\


/**---------------------------------------------------------------------
/		十八个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS18(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18)					\
	
#else
#define MESSAGE_ARGS18(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18)					\
	class NAME##Args18 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
		ARG_TYPE12 ARG_NAME12;										\
		ARG_TYPE13 ARG_NAME13;										\
		ARG_TYPE14 ARG_NAME14;										\
		ARG_TYPE15 ARG_NAME15;										\
		ARG_TYPE16 ARG_NAME16;										\
		ARG_TYPE17 ARG_NAME17;										\
		ARG_TYPE18 ARG_NAME18;										\
	public:															\
		NAME##Args18():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
			strArgsTypes.push_back(#ARG_TYPE17);					\
			strArgsTypes.push_back(#ARG_TYPE18);					\
		}															\
		NAME##Args18(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11,								\
		ARG_TYPE12 init_##ARG_NAME12,								\
		ARG_TYPE13 init_##ARG_NAME13,								\
		ARG_TYPE14 init_##ARG_NAME14,								\
		ARG_TYPE15 init_##ARG_NAME15,								\
		ARG_TYPE16 init_##ARG_NAME16,								\
		ARG_TYPE17 init_##ARG_NAME17,								\
		ARG_TYPE18 init_##ARG_NAME18):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11),								\
		ARG_NAME12(init_##ARG_NAME12),								\
		ARG_NAME13(init_##ARG_NAME13),								\
		ARG_NAME14(init_##ARG_NAME14),								\
		ARG_NAME15(init_##ARG_NAME15),								\
		ARG_NAME16(init_##ARG_NAME16),								\
		ARG_NAME17(init_##ARG_NAME17),								\
		ARG_NAME18(init_##ARG_NAME18)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
			strArgsTypes.push_back(#ARG_TYPE17);					\
			strArgsTypes.push_back(#ARG_TYPE18);					\
		}															\
		~NAME##Args18(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16,		\
								ARG_TYPE17 init_##ARG_NAME17,		\
								ARG_TYPE18 init_##ARG_NAME18)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
			s << init_##ARG_NAME17;									\
			s << init_##ARG_NAME18;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16,		\
								ARG_TYPE17 init_##ARG_NAME17,		\
								ARG_TYPE18 init_##ARG_NAME18)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
			s << init_##ARG_NAME17;									\
			s << init_##ARG_NAME18;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11) +							\
					sizeof(ARG_TYPE12) +							\
					sizeof(ARG_TYPE13) +							\
					sizeof(ARG_TYPE14) +							\
					sizeof(ARG_TYPE15) +							\
					sizeof(ARG_TYPE16) +							\
					sizeof(ARG_TYPE17) +							\
					sizeof(ARG_TYPE18);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
			s << ARG_NAME12;										\
			s << ARG_NAME13;										\
			s << ARG_NAME14;										\
			s << ARG_NAME15;										\
			s << ARG_NAME16;										\
			s << ARG_NAME17;										\
			s << ARG_NAME18;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
			s >> ARG_NAME12;										\
			s >> ARG_NAME13;										\
			s >> ARG_NAME14;										\
			s >> ARG_NAME15;										\
			s >> ARG_NAME16;										\
			s >> ARG_NAME17;										\
			s >> ARG_NAME18;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS18(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 18)\
	MESSAGE_ARGS18(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18)					\


/**---------------------------------------------------------------------
/		十九个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS19(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19)					\
	
#else
#define MESSAGE_ARGS19(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19)					\
	class NAME##Args19 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
		ARG_TYPE12 ARG_NAME12;										\
		ARG_TYPE13 ARG_NAME13;										\
		ARG_TYPE14 ARG_NAME14;										\
		ARG_TYPE15 ARG_NAME15;										\
		ARG_TYPE16 ARG_NAME16;										\
		ARG_TYPE17 ARG_NAME17;										\
		ARG_TYPE18 ARG_NAME18;										\
		ARG_TYPE19 ARG_NAME19;										\
	public:															\
		NAME##Args19():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
			strArgsTypes.push_back(#ARG_TYPE17);					\
			strArgsTypes.push_back(#ARG_TYPE18);					\
			strArgsTypes.push_back(#ARG_TYPE19);					\
		}															\
		NAME##Args19(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11,								\
		ARG_TYPE12 init_##ARG_NAME12,								\
		ARG_TYPE13 init_##ARG_NAME13,								\
		ARG_TYPE14 init_##ARG_NAME14,								\
		ARG_TYPE15 init_##ARG_NAME15,								\
		ARG_TYPE16 init_##ARG_NAME16,								\
		ARG_TYPE17 init_##ARG_NAME17,								\
		ARG_TYPE18 init_##ARG_NAME18,								\
		ARG_TYPE19 init_##ARG_NAME19):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11),								\
		ARG_NAME12(init_##ARG_NAME12),								\
		ARG_NAME13(init_##ARG_NAME13),								\
		ARG_NAME14(init_##ARG_NAME14),								\
		ARG_NAME15(init_##ARG_NAME15),								\
		ARG_NAME16(init_##ARG_NAME16),								\
		ARG_NAME17(init_##ARG_NAME17),								\
		ARG_NAME18(init_##ARG_NAME18),								\
		ARG_NAME19(init_##ARG_NAME19)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
			strArgsTypes.push_back(#ARG_TYPE17);					\
			strArgsTypes.push_back(#ARG_TYPE18);					\
			strArgsTypes.push_back(#ARG_TYPE19);					\
		}															\
		~NAME##Args19(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16,		\
								ARG_TYPE17 init_##ARG_NAME17,		\
								ARG_TYPE18 init_##ARG_NAME18,		\
								ARG_TYPE19 init_##ARG_NAME19)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
			s << init_##ARG_NAME17;									\
			s << init_##ARG_NAME18;									\
			s << init_##ARG_NAME19;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16,		\
								ARG_TYPE17 init_##ARG_NAME17,		\
								ARG_TYPE18 init_##ARG_NAME18,		\
								ARG_TYPE19 init_##ARG_NAME19)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
			s << init_##ARG_NAME17;									\
			s << init_##ARG_NAME18;									\
			s << init_##ARG_NAME19;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11) +							\
					sizeof(ARG_TYPE12) +							\
					sizeof(ARG_TYPE13) +							\
					sizeof(ARG_TYPE14) +							\
					sizeof(ARG_TYPE15) +							\
					sizeof(ARG_TYPE16) +							\
					sizeof(ARG_TYPE17) +							\
					sizeof(ARG_TYPE18) +							\
					sizeof(ARG_TYPE19);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
			s << ARG_NAME12;										\
			s << ARG_NAME13;										\
			s << ARG_NAME14;										\
			s << ARG_NAME15;										\
			s << ARG_NAME16;										\
			s << ARG_NAME17;										\
			s << ARG_NAME18;										\
			s << ARG_NAME19;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
			s >> ARG_NAME12;										\
			s >> ARG_NAME13;										\
			s >> ARG_NAME14;										\
			s >> ARG_NAME15;										\
			s >> ARG_NAME16;										\
			s >> ARG_NAME17;										\
			s >> ARG_NAME18;										\
			s >> ARG_NAME19;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS19(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 19)\
	MESSAGE_ARGS19(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19)					\


/**---------------------------------------------------------------------
/		二十个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS20(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19,					\
							ARG_TYPE20, ARG_NAME20)					\
	
#else
#define MESSAGE_ARGS20(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19,					\
							ARG_TYPE20, ARG_NAME20)					\
	class NAME##Args20 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
		ARG_TYPE12 ARG_NAME12;										\
		ARG_TYPE13 ARG_NAME13;										\
		ARG_TYPE14 ARG_NAME14;										\
		ARG_TYPE15 ARG_NAME15;										\
		ARG_TYPE16 ARG_NAME16;										\
		ARG_TYPE17 ARG_NAME17;										\
		ARG_TYPE18 ARG_NAME18;										\
		ARG_TYPE19 ARG_NAME19;										\
		ARG_TYPE20 ARG_NAME20;										\
	public:															\
		NAME##Args20():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
			strArgsTypes.push_back(#ARG_TYPE17);					\
			strArgsTypes.push_back(#ARG_TYPE18);					\
			strArgsTypes.push_back(#ARG_TYPE19);					\
			strArgsTypes.push_back(#ARG_TYPE20);					\
		}															\
		NAME##Args20(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11,								\
		ARG_TYPE12 init_##ARG_NAME12,								\
		ARG_TYPE13 init_##ARG_NAME13,								\
		ARG_TYPE14 init_##ARG_NAME14,								\
		ARG_TYPE15 init_##ARG_NAME15,								\
		ARG_TYPE16 init_##ARG_NAME16,								\
		ARG_TYPE17 init_##ARG_NAME17,								\
		ARG_TYPE18 init_##ARG_NAME18,								\
		ARG_TYPE19 init_##ARG_NAME19,								\
		ARG_TYPE20 init_##ARG_NAME20):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11),								\
		ARG_NAME12(init_##ARG_NAME12),								\
		ARG_NAME13(init_##ARG_NAME13),								\
		ARG_NAME14(init_##ARG_NAME14),								\
		ARG_NAME15(init_##ARG_NAME15),								\
		ARG_NAME16(init_##ARG_NAME16),								\
		ARG_NAME17(init_##ARG_NAME17),								\
		ARG_NAME18(init_##ARG_NAME18),								\
		ARG_NAME19(init_##ARG_NAME19),								\
		ARG_NAME20(init_##ARG_NAME20)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
			strArgsTypes.push_back(#ARG_TYPE17);					\
			strArgsTypes.push_back(#ARG_TYPE18);					\
			strArgsTypes.push_back(#ARG_TYPE19);					\
			strArgsTypes.push_back(#ARG_TYPE20);					\
		}															\
		~NAME##Args20(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16,		\
								ARG_TYPE17 init_##ARG_NAME17,		\
								ARG_TYPE18 init_##ARG_NAME18,		\
								ARG_TYPE19 init_##ARG_NAME19,		\
								ARG_TYPE20 init_##ARG_NAME20)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
			s << init_##ARG_NAME17;									\
			s << init_##ARG_NAME18;									\
			s << init_##ARG_NAME19;									\
			s << init_##ARG_NAME20;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16,		\
								ARG_TYPE17 init_##ARG_NAME17,		\
								ARG_TYPE18 init_##ARG_NAME18,		\
								ARG_TYPE19 init_##ARG_NAME19,		\
								ARG_TYPE20 init_##ARG_NAME20)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
			s << init_##ARG_NAME17;									\
			s << init_##ARG_NAME18;									\
			s << init_##ARG_NAME19;									\
			s << init_##ARG_NAME20;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11) +							\
					sizeof(ARG_TYPE12) +							\
					sizeof(ARG_TYPE13) +							\
					sizeof(ARG_TYPE14) +							\
					sizeof(ARG_TYPE15) +							\
					sizeof(ARG_TYPE16) +							\
					sizeof(ARG_TYPE17) +							\
					sizeof(ARG_TYPE18) +							\
					sizeof(ARG_TYPE19) +							\
					sizeof(ARG_TYPE20);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
			s << ARG_NAME12;										\
			s << ARG_NAME13;										\
			s << ARG_NAME14;										\
			s << ARG_NAME15;										\
			s << ARG_NAME16;										\
			s << ARG_NAME17;										\
			s << ARG_NAME18;										\
			s << ARG_NAME19;										\
			s << ARG_NAME20;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
			s >> ARG_NAME12;										\
			s >> ARG_NAME13;										\
			s >> ARG_NAME14;										\
			s >> ARG_NAME15;										\
			s >> ARG_NAME16;										\
			s >> ARG_NAME17;										\
			s >> ARG_NAME18;										\
			s >> ARG_NAME19;										\
			s >> ARG_NAME20;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS20(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19,					\
							ARG_TYPE20, ARG_NAME20)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 20)\
	MESSAGE_ARGS20(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19,					\
							ARG_TYPE20, ARG_NAME20)					\



/**---------------------------------------------------------------------
/		二十一个参数的消息
-----------------------------------------------------------------------*/
#ifdef DEFINE_IN_INTERFACE
#define MESSAGE_ARGS21(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19,					\
							ARG_TYPE20, ARG_NAME20,					\
							ARG_TYPE21, ARG_NAME21)					\
	
#else
#define MESSAGE_ARGS21(NAME, ARG_TYPE1, ARG_NAME1, 					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19,					\
							ARG_TYPE20, ARG_NAME20,					\
							ARG_TYPE21, ARG_NAME21)					\
	class NAME##Args21 : public Mercury::MessageArgs				\
	{																\
	public:															\
		ARG_TYPE1 ARG_NAME1;										\
		ARG_TYPE2 ARG_NAME2;										\
		ARG_TYPE3 ARG_NAME3;										\
		ARG_TYPE4 ARG_NAME4;										\
		ARG_TYPE5 ARG_NAME5;										\
		ARG_TYPE6 ARG_NAME6;										\
		ARG_TYPE7 ARG_NAME7;										\
		ARG_TYPE8 ARG_NAME8;										\
		ARG_TYPE9 ARG_NAME9;										\
		ARG_TYPE10 ARG_NAME10;										\
		ARG_TYPE11 ARG_NAME11;										\
		ARG_TYPE12 ARG_NAME12;										\
		ARG_TYPE13 ARG_NAME13;										\
		ARG_TYPE14 ARG_NAME14;										\
		ARG_TYPE15 ARG_NAME15;										\
		ARG_TYPE16 ARG_NAME16;										\
		ARG_TYPE17 ARG_NAME17;										\
		ARG_TYPE18 ARG_NAME18;										\
		ARG_TYPE19 ARG_NAME19;										\
		ARG_TYPE20 ARG_NAME20;										\
		ARG_TYPE21 ARG_NAME21;										\
	public:															\
		NAME##Args21():Mercury::MessageArgs()						\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
			strArgsTypes.push_back(#ARG_TYPE17);					\
			strArgsTypes.push_back(#ARG_TYPE18);					\
			strArgsTypes.push_back(#ARG_TYPE19);					\
			strArgsTypes.push_back(#ARG_TYPE20);					\
			strArgsTypes.push_back(#ARG_TYPE21);					\
		}															\
		NAME##Args21(ARG_TYPE1 init_##ARG_NAME1, 					\
		ARG_TYPE2 init_##ARG_NAME2,									\
		ARG_TYPE3 init_##ARG_NAME3,									\
		ARG_TYPE4 init_##ARG_NAME4,									\
		ARG_TYPE5 init_##ARG_NAME5,									\
		ARG_TYPE6 init_##ARG_NAME6,									\
		ARG_TYPE7 init_##ARG_NAME7,									\
		ARG_TYPE8 init_##ARG_NAME8,									\
		ARG_TYPE9 init_##ARG_NAME9,									\
		ARG_TYPE10 init_##ARG_NAME10,								\
		ARG_TYPE11 init_##ARG_NAME11,								\
		ARG_TYPE12 init_##ARG_NAME12,								\
		ARG_TYPE13 init_##ARG_NAME13,								\
		ARG_TYPE14 init_##ARG_NAME14,								\
		ARG_TYPE15 init_##ARG_NAME15,								\
		ARG_TYPE16 init_##ARG_NAME16,								\
		ARG_TYPE17 init_##ARG_NAME17,								\
		ARG_TYPE18 init_##ARG_NAME18,								\
		ARG_TYPE19 init_##ARG_NAME19,								\
		ARG_TYPE20 init_##ARG_NAME20,								\
		ARG_TYPE21 init_##ARG_NAME21):								\
		Mercury::MessageArgs(),										\
		ARG_NAME1(init_##ARG_NAME1),								\
		ARG_NAME2(init_##ARG_NAME2),								\
		ARG_NAME3(init_##ARG_NAME3),								\
		ARG_NAME4(init_##ARG_NAME4),								\
		ARG_NAME5(init_##ARG_NAME5),								\
		ARG_NAME6(init_##ARG_NAME6),								\
		ARG_NAME7(init_##ARG_NAME7),								\
		ARG_NAME8(init_##ARG_NAME8),								\
		ARG_NAME9(init_##ARG_NAME9),								\
		ARG_NAME10(init_##ARG_NAME10),								\
		ARG_NAME11(init_##ARG_NAME11),								\
		ARG_NAME12(init_##ARG_NAME12),								\
		ARG_NAME13(init_##ARG_NAME13),								\
		ARG_NAME14(init_##ARG_NAME14),								\
		ARG_NAME15(init_##ARG_NAME15),								\
		ARG_NAME16(init_##ARG_NAME16),								\
		ARG_NAME17(init_##ARG_NAME17),								\
		ARG_NAME18(init_##ARG_NAME18),								\
		ARG_NAME19(init_##ARG_NAME19),								\
		ARG_NAME20(init_##ARG_NAME20),								\
		ARG_NAME21(init_##ARG_NAME21)								\
		{															\
			strArgsTypes.push_back(#ARG_TYPE1);						\
			strArgsTypes.push_back(#ARG_TYPE2);						\
			strArgsTypes.push_back(#ARG_TYPE3);						\
			strArgsTypes.push_back(#ARG_TYPE4);						\
			strArgsTypes.push_back(#ARG_TYPE5);						\
			strArgsTypes.push_back(#ARG_TYPE6);						\
			strArgsTypes.push_back(#ARG_TYPE7);						\
			strArgsTypes.push_back(#ARG_TYPE8);						\
			strArgsTypes.push_back(#ARG_TYPE9);						\
			strArgsTypes.push_back(#ARG_TYPE10);					\
			strArgsTypes.push_back(#ARG_TYPE11);					\
			strArgsTypes.push_back(#ARG_TYPE12);					\
			strArgsTypes.push_back(#ARG_TYPE13);					\
			strArgsTypes.push_back(#ARG_TYPE14);					\
			strArgsTypes.push_back(#ARG_TYPE15);					\
			strArgsTypes.push_back(#ARG_TYPE16);					\
			strArgsTypes.push_back(#ARG_TYPE17);					\
			strArgsTypes.push_back(#ARG_TYPE18);					\
			strArgsTypes.push_back(#ARG_TYPE19);					\
			strArgsTypes.push_back(#ARG_TYPE20);					\
			strArgsTypes.push_back(#ARG_TYPE21);					\
		}															\
		~NAME##Args21(){}											\
																	\
		static void staticAddToBundle(Mercury::Bundle& s,			\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16,		\
								ARG_TYPE17 init_##ARG_NAME17,		\
								ARG_TYPE18 init_##ARG_NAME18,		\
								ARG_TYPE19 init_##ARG_NAME19,		\
								ARG_TYPE20 init_##ARG_NAME20,		\
								ARG_TYPE21 init_##ARG_NAME21)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
			s << init_##ARG_NAME17;									\
			s << init_##ARG_NAME18;									\
			s << init_##ARG_NAME19;									\
			s << init_##ARG_NAME20;									\
			s << init_##ARG_NAME21;									\
		}															\
		static void staticAddToStream(MemoryStream& s,				\
								ARG_TYPE1 init_##ARG_NAME1,			\
								ARG_TYPE2 init_##ARG_NAME2,			\
								ARG_TYPE3 init_##ARG_NAME3,			\
								ARG_TYPE4 init_##ARG_NAME4,			\
								ARG_TYPE5 init_##ARG_NAME5,			\
								ARG_TYPE6 init_##ARG_NAME6,			\
								ARG_TYPE7 init_##ARG_NAME7,			\
								ARG_TYPE8 init_##ARG_NAME8,			\
								ARG_TYPE9 init_##ARG_NAME9,			\
								ARG_TYPE10 init_##ARG_NAME10,		\
								ARG_TYPE11 init_##ARG_NAME11,		\
								ARG_TYPE12 init_##ARG_NAME12,		\
								ARG_TYPE13 init_##ARG_NAME13,		\
								ARG_TYPE14 init_##ARG_NAME14,		\
								ARG_TYPE15 init_##ARG_NAME15,		\
								ARG_TYPE16 init_##ARG_NAME16,		\
								ARG_TYPE17 init_##ARG_NAME17,		\
								ARG_TYPE18 init_##ARG_NAME18,		\
								ARG_TYPE19 init_##ARG_NAME19,		\
								ARG_TYPE20 init_##ARG_NAME20,		\
								ARG_TYPE21 init_##ARG_NAME21)		\
		{															\
			s << init_##ARG_NAME1;									\
			s << init_##ARG_NAME2;									\
			s << init_##ARG_NAME3;									\
			s << init_##ARG_NAME4;									\
			s << init_##ARG_NAME5;									\
			s << init_##ARG_NAME6;									\
			s << init_##ARG_NAME7;									\
			s << init_##ARG_NAME8;									\
			s << init_##ARG_NAME9;									\
			s << init_##ARG_NAME10;									\
			s << init_##ARG_NAME11;									\
			s << init_##ARG_NAME12;									\
			s << init_##ARG_NAME13;									\
			s << init_##ARG_NAME14;									\
			s << init_##ARG_NAME15;									\
			s << init_##ARG_NAME16;									\
			s << init_##ARG_NAME17;									\
			s << init_##ARG_NAME18;									\
			s << init_##ARG_NAME19;									\
			s << init_##ARG_NAME20;									\
			s << init_##ARG_NAME21;									\
		}															\
		virtual int32 dataSize(void)								\
		{															\
			return	sizeof(ARG_TYPE1) +								\
					sizeof(ARG_TYPE2) +								\
					sizeof(ARG_TYPE3) +								\
					sizeof(ARG_TYPE4) +								\
					sizeof(ARG_TYPE5) +								\
					sizeof(ARG_TYPE6) +								\
					sizeof(ARG_TYPE7) +								\
					sizeof(ARG_TYPE8) +								\
					sizeof(ARG_TYPE9) +								\
					sizeof(ARG_TYPE10) +							\
					sizeof(ARG_TYPE11) +							\
					sizeof(ARG_TYPE12) +							\
					sizeof(ARG_TYPE13) +							\
					sizeof(ARG_TYPE14) +							\
					sizeof(ARG_TYPE15) +							\
					sizeof(ARG_TYPE16) +							\
					sizeof(ARG_TYPE17) +							\
					sizeof(ARG_TYPE18) +							\
					sizeof(ARG_TYPE19) +							\
					sizeof(ARG_TYPE20) +							\
					sizeof(ARG_TYPE21);								\
		}															\
		virtual void addToStream(MemoryStream& s)					\
		{															\
			s << ARG_NAME1;											\
			s << ARG_NAME2;											\
			s << ARG_NAME3;											\
			s << ARG_NAME4;											\
			s << ARG_NAME5;											\
			s << ARG_NAME6;											\
			s << ARG_NAME7;											\
			s << ARG_NAME8;											\
			s << ARG_NAME9;											\
			s << ARG_NAME10;										\
			s << ARG_NAME11;										\
			s << ARG_NAME12;										\
			s << ARG_NAME13;										\
			s << ARG_NAME14;										\
			s << ARG_NAME15;										\
			s << ARG_NAME16;										\
			s << ARG_NAME17;										\
			s << ARG_NAME18;										\
			s << ARG_NAME19;										\
			s << ARG_NAME20;										\
			s << ARG_NAME21;										\
		}															\
		virtual void createFromStream(MemoryStream& s)				\
		{															\
			s >> ARG_NAME1;											\
			s >> ARG_NAME2;											\
			s >> ARG_NAME3;											\
			s >> ARG_NAME4;											\
			s >> ARG_NAME5;											\
			s >> ARG_NAME6;											\
			s >> ARG_NAME7;											\
			s >> ARG_NAME8;											\
			s >> ARG_NAME9;											\
			s >> ARG_NAME10;										\
			s >> ARG_NAME11;										\
			s >> ARG_NAME12;										\
			s >> ARG_NAME13;										\
			s >> ARG_NAME14;										\
			s >> ARG_NAME15;										\
			s >> ARG_NAME16;										\
			s >> ARG_NAME17;										\
			s >> ARG_NAME18;										\
			s >> ARG_NAME19;										\
			s >> ARG_NAME20;										\
			s >> ARG_NAME21;										\
		}															\
	};																\
				
#endif

#define NETWORK_MESSAGE_DECLARE_ARGS21(DOMAIN, NAME, MSGHANDLER,	\
											MSG_LENGTH,				\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19,					\
							ARG_TYPE20, ARG_NAME20,					\
							ARG_TYPE21, ARG_NAME21)					\
	NETWORK_MESSAGE_HANDLER(DOMAIN, NAME,MSGHANDLER, MSG_LENGTH, 21)\
	MESSAGE_ARGS21(NAME, 											\
							ARG_TYPE1, ARG_NAME1,					\
							ARG_TYPE2, ARG_NAME2,					\
							ARG_TYPE3, ARG_NAME3,					\
							ARG_TYPE4, ARG_NAME4,					\
							ARG_TYPE5, ARG_NAME5,					\
							ARG_TYPE6, ARG_NAME6,					\
							ARG_TYPE7, ARG_NAME7,					\
							ARG_TYPE8, ARG_NAME8,					\
							ARG_TYPE9, ARG_NAME9,					\
							ARG_TYPE10, ARG_NAME10,					\
							ARG_TYPE11, ARG_NAME11,					\
							ARG_TYPE12, ARG_NAME12,					\
							ARG_TYPE13, ARG_NAME13,					\
							ARG_TYPE14, ARG_NAME14,					\
							ARG_TYPE15, ARG_NAME15,					\
							ARG_TYPE16, ARG_NAME16,					\
							ARG_TYPE17, ARG_NAME17,					\
							ARG_TYPE18, ARG_NAME18,					\
							ARG_TYPE19, ARG_NAME19,					\
							ARG_TYPE20, ARG_NAME20,					\
							ARG_TYPE21, ARG_NAME21)					\


}
}

