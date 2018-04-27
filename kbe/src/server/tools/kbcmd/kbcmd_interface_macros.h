// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_KBCMD_TOOL_INTERFACE_MACRO_H
#endif


#ifndef KBE_KBCMD_TOOL_INTERFACE_MACRO_H
#define KBE_KBCMD_TOOL_INTERFACE_MACRO_H

// common include	
#include "network/interface_defs.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	KBCMD消息宏，  参数为流， 需要自己解开
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef KBCMD_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(KBCMD)
#define KBCMD_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##KBCMDMessagehandler_stream::handle(Network::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
			KBEngine::KBCMD::getSingleton().NAME(pChannel, s);					\
	}																			\

#else
#define KBCMD_MESSAGE_HANDLER_STREAM(NAME)										\
	void NAME##KBCMDMessagehandler_stream::handle(Network::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\

#endif
#else
#define KBCMD_MESSAGE_HANDLER_STREAM(NAME)										\
	class NAME##KBCMDMessagehandler_stream : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define KBCMD_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)							\
	KBCMD_MESSAGE_HANDLER_STREAM(NAME)											\
	NETWORK_MESSAGE_DECLARE_STREAM(KBCMD, NAME,									\
				NAME##KBCMDMessagehandler_stream, MSG_LENGTH)					\
																				\

	/**
	KBCMD消息宏，  只有零个参数的消息
	*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef KBCMD_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(KBCMD)
#define KBCMD_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##KBCMDMessagehandler0::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			KBEngine::KBCMD::getSingleton().NAME(pChannel);						\
	}																			\

#else
#define KBCMD_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##KBCMDMessagehandler0::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\

#endif
#else
#define KBCMD_MESSAGE_HANDLER_ARGS0(NAME)										\
	class NAME##KBCMDMessagehandler0 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define KBCMD_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)							\
	KBCMD_MESSAGE_HANDLER_ARGS0(NAME)											\
	NETWORK_MESSAGE_DECLARE_ARGS0(KBCMD, NAME,									\
				NAME##KBCMDMessagehandler0, MSG_LENGTH)							\
																				\

	/**
	KBCMD消息宏，  只有二个参数的消息
	*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef KBCMD_MESSAGE_HANDLER_ARGS2
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(KBCMD)
#define KBCMD_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##KBCMDMessagehandler2::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			KBEngine::KBCMD::getSingleton().NAME(pChannel,						\
													ARG_NAME1, ARG_NAME2);		\
	}																			\

#else
#define KBCMD_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##KBCMDMessagehandler2::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\

#endif
#else
#define KBCMD_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2)				\
	class NAME##KBCMDMessagehandler2 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define KBCMD_MESSAGE_DECLARE_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2)				\
	KBCMD_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 						\
											ARG_TYPE2, ARG_NAME2)				\
	NETWORK_MESSAGE_DECLARE_ARGS2(KBCMD, NAME,									\
				NAME##KBCMDMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2)				\


	/**
	KBCMD消息宏，  只有一个参数的消息
	*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef KBCMD_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(KBCMD)
#define KBCMD_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##KBCMDMessagehandler1::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			KBEngine::KBCMD::getSingleton().NAME(pChannel, ARG_NAME1);			\
	}																			\

#else
#define KBCMD_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	void NAME##KBCMDMessagehandler1::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\

#endif
#else
#define KBCMD_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	class NAME##KBCMDMessagehandler1 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define KBCMD_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)		\
	KBCMD_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)						\
	NETWORK_MESSAGE_DECLARE_ARGS1(KBCMD, NAME,									\
				NAME##KBCMDMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
																				\

	/**
	KBCMD消息宏，  只有四个参数的消息
	*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef KBCMD_MESSAGE_HANDLER_ARGS4
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(KBCMD)
#define KBCMD_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##KBCMDMessagehandler4::handle(Network::Channel* pChannel,			\
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
			KBEngine::KBCMD::getSingleton().NAME(pChannel,						\
				ARG_NAME1, ARG_NAME2,										 	\
				ARG_NAME3, ARG_NAME4);											\
	}																			\

#else
#define KBCMD_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##KBCMDMessagehandler4::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\

#endif
#else
#define KBCMD_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	class NAME##KBCMDMessagehandler4 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define KBCMD_MESSAGE_DECLARE_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	KBCMD_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 						\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	NETWORK_MESSAGE_DECLARE_ARGS4(KBCMD, NAME,									\
				NAME##KBCMDMessagehandler4, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\

	/**
	KBCMD消息宏，  只有五个参数的消息
	*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef KBCMD_MESSAGE_HANDLER_ARGS5
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(KBCMD)
#define KBCMD_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##KBCMDMessagehandler5::handle(Network::Channel* pChannel,			\
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
			KBEngine::KBCMD::getSingleton().NAME(pChannel,						\
				ARG_NAME1, ARG_NAME2,										 	\
				ARG_NAME3, ARG_NAME4, ARG_NAME5);								\
	}																			\

#else
#define KBCMD_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##KBCMDMessagehandler5::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\

#endif
#else
#define KBCMD_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	class NAME##KBCMDMessagehandler5 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define KBCMD_MESSAGE_DECLARE_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	KBCMD_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 						\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	NETWORK_MESSAGE_DECLARE_ARGS5(KBCMD, NAME,									\
				NAME##KBCMDMessagehandler5, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\


	/**
	KBCMD消息宏，  只有六个参数的消息
	*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef KBCMD_MESSAGE_HANDLER_ARGS6
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(KBCMD)
#define KBCMD_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##KBCMDMessagehandler6::handle(Network::Channel* pChannel,			\
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
			KBEngine::KBCMD::getSingleton().NAME(pChannel,						\
				ARG_NAME1, ARG_NAME2,										 	\
				ARG_NAME3, ARG_NAME4, ARG_NAME5, ARG_NAME6);					\
	}																			\

#else
#define KBCMD_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##KBCMDMessagehandler6::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\

#endif
#else
#define KBCMD_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	class NAME##KBCMDMessagehandler6 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define KBCMD_MESSAGE_DECLARE_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	KBCMD_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 						\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	NETWORK_MESSAGE_DECLARE_ARGS6(KBCMD, NAME,									\
				NAME##KBCMDMessagehandler6, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\

	/**
	KBCMD消息宏，  只有八个参数的消息
	*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef KBCMD_MESSAGE_HANDLER_ARGS8
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(KBCMD)
#define KBCMD_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##KBCMDMessagehandler8::handle(Network::Channel* pChannel,			\
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
			KBEngine::KBCMD::getSingleton().NAME(pChannel,						\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8);					\
	}																			\

#else
#define KBCMD_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##KBCMDMessagehandler8::handle(Network::Channel* pChannel,			\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\

#endif
#else
#define KBCMD_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	class NAME##KBCMDMessagehandler8 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define KBCMD_MESSAGE_DECLARE_ARGS8(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,		\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	KBCMD_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1, 						\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	NETWORK_MESSAGE_DECLARE_ARGS8(KBCMD, NAME,									\
				NAME##KBCMDMessagehandler8, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\

}
#endif
