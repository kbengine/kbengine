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
	#undef __RESOURCEMGR_INTERFACE_MACRO_H__
#endif


#ifndef __RESOURCEMGR_INTERFACE_MACRO_H__
#define __RESOURCEMGR_INTERFACE_MACRO_H__

// common include	
#include "network/interface_defs.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Resourcemgr消息宏，  参数为流， 需要自己解开
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef RESOURCEMGR_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(RESOURCEMGR)
#define RESOURCEMGR_MESSAGE_HANDLER_STREAM(NAME)								\
	void NAME##ResourcemgrMessagehandler_stream::handle(Mercury::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
			KBEngine::Resourcemgr::getSingleton().NAME(pChannel, s);			\
	}																			\

#else
#define RESOURCEMGR_MESSAGE_HANDLER_STREAM(NAME)								\
	void NAME##ResourcemgrMessagehandler_stream::handle(Mercury::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\
		
#endif
#else
#define RESOURCEMGR_MESSAGE_HANDLER_STREAM(NAME)									\
	class NAME##ResourcemgrMessagehandler_stream : public Mercury::MessageHandler\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define RESOURCEMGR_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)					\
	RESOURCEMGR_MESSAGE_HANDLER_STREAM(NAME)									\
	NETWORK_MESSAGE_DECLARE_STREAM(Resourcemgr, NAME,							\
				NAME##ResourcemgrMessagehandler_stream, MSG_LENGTH)				\
																				\

/**
	Resourcemgr消息宏，  只有零个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef RESOURCEMGR_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(RESOURCEMGR)
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS0(NAME)									\
	void NAME##ResourcemgrMessagehandler0::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			KBEngine::Resourcemgr::getSingleton().NAME(pChannel);				\
	}																			\

#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS0(NAME)									\
	void NAME##ResourcemgrMessagehandler0::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS0(NAME)									\
	class NAME##ResourcemgrMessagehandler0 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define RESOURCEMGR_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)						\
	RESOURCEMGR_MESSAGE_HANDLER_ARGS0(NAME)										\
	NETWORK_MESSAGE_DECLARE_ARGS0(Resourcemgr, NAME,							\
				NAME##ResourcemgrMessagehandler0, MSG_LENGTH)					\
																				\


/**
	Resourcemgr消息宏，  只有一个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef RESOURCEMGR_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(RESOURCEMGR)
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	void NAME##ResourcemgrMessagehandler1::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			KBEngine::Resourcemgr::getSingleton().NAME(pChannel, ARG_NAME1);	\
	}																			\

#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	void NAME##ResourcemgrMessagehandler1::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	class NAME##ResourcemgrMessagehandler1 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define RESOURCEMGR_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)\
	RESOURCEMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	NETWORK_MESSAGE_DECLARE_ARGS1(Resourcemgr, NAME,							\
				NAME##ResourcemgrMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
																				\

/**
	Resourcemgr消息宏，  只有二个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef RESOURCEMGR_MESSAGE_HANDLER_ARGS2
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(RESOURCEMGR)
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##ResourcemgrMessagehandler2::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			KBEngine::Resourcemgr::getSingleton().NAME(pChannel,				\
													ARG_NAME1, ARG_NAME2);		\
	}																			\

#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##ResourcemgrMessagehandler2::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2)				\
	class NAME##ResourcemgrMessagehandler2 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define RESOURCEMGR_MESSAGE_DECLARE_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2)				\
	RESOURCEMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2)				\
	NETWORK_MESSAGE_DECLARE_ARGS2(Resourcemgr, NAME,							\
				NAME##ResourcemgrMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2)				\


/**
	Resourcemgr消息宏，  只有三个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef RESOURCEMGR_MESSAGE_HANDLER_ARGS3
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(RESOURCEMGR)
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##CellAppMessagehandler3::handle(Mercury::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			KBEngine::Resourcemgr::getSingleton().NAME(pChannel,				\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3);														\
	}																			\

#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##CellAppMessagehandler3::handle(Mercury::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	class NAME##CellAppMessagehandler3 : public Mercury::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define RESOURCEMGR_MESSAGE_DECLARE_ARGS3(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	RESOURCEMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	NETWORK_MESSAGE_DECLARE_ARGS3(Resourcemgr, NAME,							\
				NAME##CellAppMessagehandler3, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\


/**
	Resourcemgr消息宏，  只有四个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef RESOURCEMGR_MESSAGE_HANDLER_ARGS4
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(RESOURCEMGR)
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##ResourcemgrMessagehandler4::handle(Mercury::Channel* pChannel,	\
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
			KBEngine::Resourcemgr::getSingleton().NAME(pChannel,				\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4);											\
	}																			\

#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##ResourcemgrMessagehandler4::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	class NAME##ResourcemgrMessagehandler4 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define RESOURCEMGR_MESSAGE_DECLARE_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	RESOURCEMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	NETWORK_MESSAGE_DECLARE_ARGS4(Resourcemgr, NAME,							\
				NAME##ResourcemgrMessagehandler4, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\

/**
	Resourcemgr消息宏，  只有五个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef RESOURCEMGR_MESSAGE_HANDLER_ARGS5
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(RESOURCEMGR)
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##ResourcemgrMessagehandler5::handle(Mercury::Channel* pChannel,	\
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
			KBEngine::Resourcemgr::getSingleton().NAME(pChannel,				\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4, ARG_NAME5);								\
	}																			\

#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##ResourcemgrMessagehandler5::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	class NAME##ResourcemgrMessagehandler5 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define RESOURCEMGR_MESSAGE_DECLARE_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	RESOURCEMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	NETWORK_MESSAGE_DECLARE_ARGS5(Resourcemgr, NAME,							\
				NAME##ResourcemgrMessagehandler5, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\


/**
	Resourcemgr消息宏，  只有六个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef RESOURCEMGR_MESSAGE_HANDLER_ARGS6
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(RESOURCEMGR)
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##ResourcemgrMessagehandler6::handle(Mercury::Channel* pChannel,	\
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
			KBEngine::Resourcemgr::getSingleton().NAME(pChannel,				\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4, ARG_NAME5, ARG_NAME6);					\
	}																			\

#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##ResourcemgrMessagehandler6::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	class NAME##ResourcemgrMessagehandler6 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define RESOURCEMGR_MESSAGE_DECLARE_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	RESOURCEMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	NETWORK_MESSAGE_DECLARE_ARGS6(Resourcemgr, NAME,							\
				NAME##ResourcemgrMessagehandler6, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\

/**
	Resourcemgr消息宏，  只有八个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef RESOURCEMGR_MESSAGE_HANDLER_ARGS8
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(RESOURCEMGR)
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##ResourcemgrMessagehandler8::handle(Mercury::Channel* pChannel,	\
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
			ARG_TYPE7 ARG_NAME7;												\
			s >> ARG_NAME7;														\
			ARG_TYPE8 ARG_NAME8;												\
			s >> ARG_NAME8;														\
			KBEngine::Resourcemgr::getSingleton().NAME(pChannel,				\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8);					\
	}																			\

#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##ResourcemgrMessagehandler8::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define RESOURCEMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	class NAME##ResourcemgrMessagehandler8 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define RESOURCEMGR_MESSAGE_DECLARE_ARGS8(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	RESOURCEMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	NETWORK_MESSAGE_DECLARE_ARGS8(Resourcemgr, NAME,							\
				NAME##ResourcemgrMessagehandler8, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\



}
#endif
