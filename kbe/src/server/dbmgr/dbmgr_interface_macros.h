// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_DBMGR_INTERFACE_MACRO_H
#endif


#ifndef KBE_DBMGR_INTERFACE_MACRO_H
#define KBE_DBMGR_INTERFACE_MACRO_H

// common include	
#include "network/interface_defs.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine{

#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##DbmgrMessagehandler_stream::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			KBEngine::Dbmgr::getSingleton().NAME(pChannel, s);					\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##DbmgrMessagehandler_stream::handle(Network::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_STREAM(NAME)										\
	class NAME##DbmgrMessagehandler_stream : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)							\
	DBMGR_MESSAGE_HANDLER_STREAM(NAME)											\
	NETWORK_MESSAGE_DECLARE_STREAM(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler_stream, MSG_LENGTH)					\
																				\

/**
	Dbmgr消息宏，  只有零个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##DbmgrMessagehandler0::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			KBEngine::Dbmgr::getSingleton().NAME(pChannel);						\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##DbmgrMessagehandler0::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS0(NAME)										\
	class NAME##DbmgrMessagehandler0 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)							\
	DBMGR_MESSAGE_HANDLER_ARGS0(NAME)											\
	NETWORK_MESSAGE_DECLARE_ARGS0(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler0, MSG_LENGTH)							\
																				\


/**
	Dbmgr消息宏，  只有一个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##DbmgrMessagehandler1::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			KBEngine::Dbmgr::getSingleton().NAME(pChannel, ARG_NAME1);			\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##DbmgrMessagehandler1::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	class NAME##DbmgrMessagehandler1 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)		\
	DBMGR_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)						\
	NETWORK_MESSAGE_DECLARE_ARGS1(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
																				\
	
/**
	Dbmgr消息宏，  只有二个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS2
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##DbmgrMessagehandler2::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			KBEngine::Dbmgr::getSingleton().NAME(pChannel,						\
													ARG_NAME1, ARG_NAME2);		\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##DbmgrMessagehandler2::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	class NAME##DbmgrMessagehandler2 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2)				\
	DBMGR_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2)				\
	NETWORK_MESSAGE_DECLARE_ARGS2(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2)				\


/**
	Dbmgr消息宏，  只有三个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS3
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##DbmgrMessagehandler3::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			KBEngine::Dbmgr::getSingleton().NAME(pChannel,						\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3);														\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##DbmgrMessagehandler3::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	class NAME##DbmgrMessagehandler3 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS3(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	DBMGR_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	NETWORK_MESSAGE_DECLARE_ARGS3(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler3, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\


/**
	Dbmgr消息宏，  只有四个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS4
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##DbmgrMessagehandler4::handle(Network::Channel* pChannel,			\
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
			KBEngine::Dbmgr::getSingleton().NAME(pChannel,						\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4);											\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##DbmgrMessagehandler4::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	class NAME##DbmgrMessagehandler4 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	DBMGR_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	NETWORK_MESSAGE_DECLARE_ARGS4(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler4, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\

/**
	Dbmgr消息宏，  只有五个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS5
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##DbmgrMessagehandler5::handle(Network::Channel* pChannel,			\
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
			KBEngine::Dbmgr::getSingleton().NAME(pChannel,						\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4, ARG_NAME5);								\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##DbmgrMessagehandler5::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	class NAME##DbmgrMessagehandler5 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	DBMGR_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	NETWORK_MESSAGE_DECLARE_ARGS5(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler5, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\


/**
	Dbmgr消息宏，  只有六个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS6
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##DbmgrMessagehandler6::handle(Network::Channel* pChannel,			\
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
			KBEngine::Dbmgr::getSingleton().NAME(pChannel,						\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4, ARG_NAME5, ARG_NAME6);					\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##DbmgrMessagehandler6::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	class NAME##DbmgrMessagehandler6 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	DBMGR_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	NETWORK_MESSAGE_DECLARE_ARGS6(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler6, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\

/**
	Dbmgr消息宏，  只有七个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS7
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	void NAME##DbmgrMessagehandler7::handle(Network::Channel* pChannel,			\
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
			KBEngine::Dbmgr::getSingleton().NAME(pChannel,						\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7);								\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	void NAME##DbmgrMessagehandler7::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	class NAME##DbmgrMessagehandler7 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS7(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	DBMGR_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	NETWORK_MESSAGE_DECLARE_ARGS7(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler7, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\


/**
	Dbmgr消息宏，  只有八个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS8
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##DbmgrMessagehandler8::handle(Network::Channel* pChannel,			\
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
			KBEngine::Dbmgr::getSingleton().NAME(pChannel,						\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8);					\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##DbmgrMessagehandler8::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	class NAME##DbmgrMessagehandler8 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS8(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	DBMGR_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	NETWORK_MESSAGE_DECLARE_ARGS8(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler8, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\


/**
	Dbmgr消息宏，  只有九个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS9
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	void NAME##DbmgrMessagehandler9::handle(Network::Channel* pChannel,			\
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
			ARG_TYPE9 ARG_NAME9;												\
			s >> ARG_NAME9;														\
			KBEngine::Dbmgr::getSingleton().NAME(pChannel,						\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9);		\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	void NAME##DbmgrMessagehandler9::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	class NAME##DbmgrMessagehandler9 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS9(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	DBMGR_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	NETWORK_MESSAGE_DECLARE_ARGS9(Dbmgr, NAME,									\
				NAME##DbmgrMessagehandler9, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\


/**
	Dbmgr消息宏，  只有十个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS10
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	void NAME##DbmgrMessagehandler10::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE9 ARG_NAME9;												\
			s >> ARG_NAME9;														\
			ARG_TYPE10 ARG_NAME10;												\
			s >> ARG_NAME10;													\
			KBEngine::Dbmgr::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10);							\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	void NAME##DbmgrMessagehandler10::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	class NAME##DbmgrMessagehandler10 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS10(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	DBMGR_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	NETWORK_MESSAGE_DECLARE_ARGS10(Dbmgr, NAME,								\
				NAME##DbmgrMessagehandler10, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\


/**
	Dbmgr消息宏，  只有十一个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef DBMGR_MESSAGE_HANDLER_ARGS11
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(DBMGR)
#define DBMGR_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\
	void NAME##DbmgrMessagehandler11::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE9 ARG_NAME9;												\
			s >> ARG_NAME9;														\
			ARG_TYPE10 ARG_NAME10;												\
			s >> ARG_NAME10;													\
			ARG_TYPE11 ARG_NAME11;												\
			s >> ARG_NAME11;													\
			KBEngine::Dbmgr::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11);				\
	}																			\

#else
#define DBMGR_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\
	void NAME##DbmgrMessagehandler11::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define DBMGR_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\
	class NAME##DbmgrMessagehandler11 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define DBMGR_MESSAGE_DECLARE_ARGS11(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\
	DBMGR_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\
	NETWORK_MESSAGE_DECLARE_ARGS11(Dbmgr, NAME,								\
				NAME##DbmgrMessagehandler11, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11)				\


}
#endif
