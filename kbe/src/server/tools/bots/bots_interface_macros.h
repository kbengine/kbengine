// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_BOTS_INTERFACE_MACRO_H
#endif


#ifndef KBE_BOTS_INTERFACE_MACRO_H
#define KBE_BOTS_INTERFACE_MACRO_H

// common include	
#include "network/interface_defs.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Bots消息宏，  参数为流， 需要自己解开
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BOTS_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BOTS)
#define BOTS_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##BotsMessagehandler_stream::handle(Network::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
			KBEngine::Bots::getSingleton().NAME(pChannel, s);					\
	}																			\

#else
#define BOTS_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##BotsMessagehandler_stream::handle(Network::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\
		
#endif
#else
#define BOTS_MESSAGE_HANDLER_STREAM(NAME)										\
	class NAME##BotsMessagehandler_stream : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BOTS_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)							\
	BOTS_MESSAGE_HANDLER_STREAM(NAME)											\
	NETWORK_MESSAGE_DECLARE_STREAM(Bots, NAME,									\
				NAME##BotsMessagehandler_stream, MSG_LENGTH)					\
																				\

/**
	Bots消息宏，  只有零个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BOTS_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BOTS)
#define BOTS_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##BotsMessagehandler0::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			KBEngine::Bots::getSingleton().NAME(pChannel);						\
	}																			\

#else
#define BOTS_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##BotsMessagehandler0::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BOTS_MESSAGE_HANDLER_ARGS0(NAME)										\
	class NAME##BotsMessagehandler0 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define BOTS_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)							\
	BOTS_MESSAGE_HANDLER_ARGS0(NAME)											\
	NETWORK_MESSAGE_DECLARE_ARGS0(Bots, NAME,									\
				NAME##BotsMessagehandler0, MSG_LENGTH)							\
																				\

/**
	Bots消息宏，  只有二个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BOTS_MESSAGE_HANDLER_ARGS2
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BOTS)
#define BOTS_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##BotsMessagehandler2::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			KBEngine::Bots::getSingleton().NAME(pChannel,						\
													ARG_NAME1, ARG_NAME2);		\
	}																			\

#else
#define BOTS_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##BotsMessagehandler2::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BOTS_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	class NAME##BotsMessagehandler2 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BOTS_MESSAGE_DECLARE_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2)				\
	BOTS_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 						\
											ARG_TYPE2, ARG_NAME2)				\
	NETWORK_MESSAGE_DECLARE_ARGS2(Bots, NAME,									\
				NAME##BotsMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2)				\


/**
	Bots消息宏，  只有一个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BOTS_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BOTS)
#define BOTS_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##BotsMessagehandler1::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			KBEngine::Bots::getSingleton().NAME(pChannel, ARG_NAME1);			\
	}																			\

#else
#define BOTS_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##BotsMessagehandler1::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BOTS_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	class NAME##BotsMessagehandler1 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BOTS_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)		\
	BOTS_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)						\
	NETWORK_MESSAGE_DECLARE_ARGS1(Bots, NAME,									\
				NAME##BotsMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
																				\
	
/**
	Bots消息宏，  只有四个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BOTS_MESSAGE_HANDLER_ARGS4
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BOTS)
#define BOTS_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##BotsMessagehandler4::handle(Network::Channel* pChannel,			\
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
			KBEngine::Bots::getSingleton().NAME(pChannel,						\
				ARG_NAME1, ARG_NAME2,										 	\
				ARG_NAME3, ARG_NAME4);											\
	}																			\

#else
#define BOTS_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##BotsMessagehandler4::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BOTS_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	class NAME##BotsMessagehandler4 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BOTS_MESSAGE_DECLARE_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	BOTS_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 						\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	NETWORK_MESSAGE_DECLARE_ARGS4(Bots, NAME,									\
				NAME##BotsMessagehandler4, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\

/**
	Bots消息宏，  只有五个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BOTS_MESSAGE_HANDLER_ARGS5
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BOTS)
#define BOTS_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##BotsMessagehandler5::handle(Network::Channel* pChannel,			\
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
			KBEngine::Bots::getSingleton().NAME(pChannel,						\
				ARG_NAME1, ARG_NAME2,										 	\
				ARG_NAME3, ARG_NAME4, ARG_NAME5);								\
	}																			\

#else
#define BOTS_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##BotsMessagehandler5::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BOTS_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	class NAME##BotsMessagehandler5 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BOTS_MESSAGE_DECLARE_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	BOTS_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 						\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	NETWORK_MESSAGE_DECLARE_ARGS5(Bots, NAME,									\
				NAME##BotsMessagehandler5, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\


/**
	Bots消息宏，  只有六个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BOTS_MESSAGE_HANDLER_ARGS6
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BOTS)
#define BOTS_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##BotsMessagehandler6::handle(Network::Channel* pChannel,			\
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
			KBEngine::Bots::getSingleton().NAME(pChannel,						\
				ARG_NAME1, ARG_NAME2,										 	\
				ARG_NAME3, ARG_NAME4, ARG_NAME5, ARG_NAME6);					\
	}																			\

#else
#define BOTS_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##BotsMessagehandler6::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BOTS_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	class NAME##BotsMessagehandler6 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BOTS_MESSAGE_DECLARE_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	BOTS_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 						\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	NETWORK_MESSAGE_DECLARE_ARGS6(Bots, NAME,									\
				NAME##BotsMessagehandler6, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\

/**
	Bots消息宏，  只有八个参数的消息
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef BOTS_MESSAGE_HANDLER_ARGS8
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(BOTS)
#define BOTS_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##BotsMessagehandler8::handle(Network::Channel* pChannel,			\
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
			KBEngine::Bots::getSingleton().NAME(pChannel,						\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8);					\
	}																			\

#else
#define BOTS_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##BotsMessagehandler8::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define BOTS_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	class NAME##BotsMessagehandler8 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define BOTS_MESSAGE_DECLARE_ARGS8(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	BOTS_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1, 						\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	NETWORK_MESSAGE_DECLARE_ARGS8(Bots, NAME,									\
				NAME##BotsMessagehandler8, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\


}
#endif
