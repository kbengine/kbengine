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
	#undef __BASEAPPMG_INTERFACE_MACRO_H__
#endif


#ifndef __BASEAPPMG_INTERFACE_MACRO_H__
#define __BASEAPPMG_INTERFACE_MACRO_H__

// common include	
#include "network/interface_defs.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Baseappmgr消息宏，  参数为流， 需要自己解开
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BASEAPPMGR_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPPMGR)
#define BASEAPPMGR_MESSAGE_HANDLER_STREAM(NAME)									\
	void NAME##BaseappmgrMessagehandler_stream::handle(Mercury::Channel* pChannel,\
													KBEngine::MemoryStream& s)	\
	{																			\
			KBEngine::Baseappmgr::getSingleton().NAME(s);						\
	}																			\

#else
#define BASEAPPMGR_MESSAGE_HANDLER_STREAM(NAME)									\
	void NAME##BaseappmgrMessagehandler_stream::handle(Mercury::Channel* pChannel,\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\
		
#endif
#else
#define BASEAPPMGR_MESSAGE_HANDLER_STREAM(NAME)									\
	class NAME##BaseappmgrMessagehandler_stream : public Mercury::MessageHandler\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BASEAPPMGR_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)						\
	BASEAPPMGR_MESSAGE_HANDLER_STREAM(NAME)										\
	NETWORK_MESSAGE_DECLARE_STREAM(Baseappmgr, NAME,							\
				NAME##BaseappmgrMessagehandler_stream, MSG_LENGTH)				\
																				\

/**
	Baseappmgr消息宏，  只有一个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BASEAPPMGR_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPPMGR)
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	void NAME##BaseappmgrMessagehandler1::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			KBEngine::Baseappmgr::getSingleton().NAME(ARG_NAME1);				\
	}																			\

#else
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	void NAME##BaseappmgrMessagehandler1::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	class NAME##BaseappmgrMessagehandler1 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BASEAPPMGR_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)\
	BASEAPPMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	NETWORK_MESSAGE_DECLARE_ARGS1(Baseappmgr, NAME,								\
				NAME##BaseappmgrMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)\
																				\
	
/**
	Baseappmgr消息宏，  只有四个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BASEAPPMGR_MESSAGE_HANDLER_ARGS4
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPPMGR)
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##BaseappmgrMessagehandler4::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			ARG_TYPE4 ARG_NAME4;												\
			s >> ARG_NAME4;														\
			KBEngine::Baseappmgr::getSingleton().NAME(ARG_NAME1, ARG_NAME2, 	\
				ARG_NAME3, ARG_NAME4);											\
	}																			\

#else
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##BaseappmgrMessagehandler4::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	class NAME##BaseappmgrMessagehandler4 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BASEAPPMGR_MESSAGE_DECLARE_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	BASEAPPMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	NETWORK_MESSAGE_DECLARE_ARGS4(Baseappmgr, NAME,								\
				NAME##BaseappmgrMessagehandler4, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\

/**
	Baseappmgr消息宏，  只有五个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BASEAPPMGR_MESSAGE_HANDLER_ARGS5
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPPMGR)
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##BaseappmgrMessagehandler5::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			ARG_TYPE4 ARG_NAME4;												\
			s >> ARG_NAME4;														\
			ARG_TYPE5 ARG_NAME5;												\
			s >> ARG_NAME5;														\
			KBEngine::Baseappmgr::getSingleton().NAME(ARG_NAME1, ARG_NAME2, 	\
				ARG_NAME3, ARG_NAME4, ARG_NAME5);								\
	}																			\

#else
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##BaseappmgrMessagehandler5::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	class NAME##BaseappmgrMessagehandler5 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BASEAPPMGR_MESSAGE_DECLARE_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	BASEAPPMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	NETWORK_MESSAGE_DECLARE_ARGS5(Baseappmgr, NAME,								\
				NAME##BaseappmgrMessagehandler5, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\


/**
	Baseappmgr消息宏，  只有六个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BASEAPPMGR_MESSAGE_HANDLER_ARGS6
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BASEAPPMGR)
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##BaseappmgrMessagehandler6::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			ARG_TYPE4 ARG_NAME4;												\
			s >> ARG_NAME4;														\
			ARG_TYPE5 ARG_NAME5;												\
			s >> ARG_NAME5;														\
			ARG_TYPE6 ARG_NAME6;												\
			s >> ARG_NAME6;														\
			KBEngine::Baseappmgr::getSingleton().NAME(ARG_NAME1, ARG_NAME2, 	\
				ARG_NAME3, ARG_NAME4, ARG_NAME5, ARG_NAME6);					\
	}																			\

#else
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##BaseappmgrMessagehandler6::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BASEAPPMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	class NAME##BaseappmgrMessagehandler6 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BASEAPPMGR_MESSAGE_DECLARE_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	BASEAPPMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	NETWORK_MESSAGE_DECLARE_ARGS6(Baseappmgr, NAME,								\
				NAME##BaseappmgrMessagehandler6, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\

}
#endif
