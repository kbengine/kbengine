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
	#undef KBE_MESSAGELOG_INTERFACE_MACRO_HPP
#endif


#ifndef KBE_MESSAGELOG_INTERFACE_MACRO_HPP
#define KBE_MESSAGELOG_INTERFACE_MACRO_HPP

// common include	
#include "network/interface_defs.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Messagelog��Ϣ�꣬  ����Ϊ���� ��Ҫ�Լ��⿪
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_STREAM(NAME)									\
	void NAME##MessagelogMessagehandler_stream::handle(Mercury::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
			KBEngine::Messagelog::getSingleton().NAME(pChannel, s);				\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_STREAM(NAME)									\
	void NAME##MessagelogMessagehandler_stream::handle(Mercury::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_STREAM(NAME)									\
	class NAME##MessagelogMessagehandler_stream : public Mercury::MessageHandler\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)						\
	MESSAGELOG_MESSAGE_HANDLER_STREAM(NAME)										\
	NETWORK_MESSAGE_DECLARE_STREAM(Messagelog, NAME,							\
				NAME##MessagelogMessagehandler_stream, MSG_LENGTH)				\
																				\

/**
	Messagelog��Ϣ�꣬  ֻ�������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS0(NAME)									\
	void NAME##MessagelogMessagehandler0::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			KBEngine::Messagelog::getSingleton().NAME(pChannel);				\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS0(NAME)									\
	void NAME##MessagelogMessagehandler0::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS0(NAME)									\
	class NAME##MessagelogMessagehandler0 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)						\
	MESSAGELOG_MESSAGE_HANDLER_ARGS0(NAME)										\
	NETWORK_MESSAGE_DECLARE_ARGS0(Messagelog, NAME,								\
				NAME##MessagelogMessagehandler0, MSG_LENGTH)					\
																				\


/**
	Messagelog��Ϣ�꣬  ֻ��һ����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	void NAME##MessagelogMessagehandler1::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			KBEngine::Messagelog::getSingleton().NAME(pChannel, ARG_NAME1);		\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	void NAME##MessagelogMessagehandler1::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)			\
	class NAME##MessagelogMessagehandler1 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)\
	MESSAGELOG_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	NETWORK_MESSAGE_DECLARE_ARGS1(Messagelog, NAME,								\
				NAME##MessagelogMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
																				\

/**
	Messagelog��Ϣ�꣬  ֻ�ж�����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS2
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##MessagelogMessagehandler2::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			KBEngine::Messagelog::getSingleton().NAME(pChannel,					\
													ARG_NAME1, ARG_NAME2);		\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##MessagelogMessagehandler2::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2)				\
	class NAME##MessagelogMessagehandler2 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2)				\
	MESSAGELOG_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2)				\
	NETWORK_MESSAGE_DECLARE_ARGS2(Messagelog, NAME,								\
				NAME##MessagelogMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2)				\


/**
	Messagelog��Ϣ�꣬  ֻ��������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS3
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,			\
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
			KBEngine::Messagelog::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3);														\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##CellAppMessagehandler3::handle(Mercury::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	class NAME##CellAppMessagehandler3 : public Mercury::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
												KBEngine::MemoryStream& s);		\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS3(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	MESSAGELOG_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	NETWORK_MESSAGE_DECLARE_ARGS3(Messagelog, NAME,								\
				NAME##CellAppMessagehandler3, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\


/**
	Messagelog��Ϣ�꣬  ֻ���ĸ���������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS4
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##MessagelogMessagehandler4::handle(Mercury::Channel* pChannel,	\
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
			KBEngine::Messagelog::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4);											\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##MessagelogMessagehandler4::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	class NAME##MessagelogMessagehandler4 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	MESSAGELOG_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	NETWORK_MESSAGE_DECLARE_ARGS4(Messagelog, NAME,								\
				NAME##MessagelogMessagehandler4, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\

/**
	Messagelog��Ϣ�꣬  ֻ�������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS5
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##MessagelogMessagehandler5::handle(Mercury::Channel* pChannel,	\
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
			KBEngine::Messagelog::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4, ARG_NAME5);								\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##MessagelogMessagehandler5::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	class NAME##MessagelogMessagehandler5 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	MESSAGELOG_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	NETWORK_MESSAGE_DECLARE_ARGS5(Messagelog, NAME,								\
				NAME##MessagelogMessagehandler5, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\


/**
	Messagelog��Ϣ�꣬  ֻ��������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS6
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##MessagelogMessagehandler6::handle(Mercury::Channel* pChannel,	\
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
			KBEngine::Messagelog::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4, ARG_NAME5, ARG_NAME6);					\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##MessagelogMessagehandler6::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	class NAME##MessagelogMessagehandler6 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	MESSAGELOG_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	NETWORK_MESSAGE_DECLARE_ARGS6(Messagelog, NAME,								\
				NAME##MessagelogMessagehandler6, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\

/**
	Messagelog��Ϣ�꣬  ֻ�а˸���������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS8
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##MessagelogMessagehandler8::handle(Mercury::Channel* pChannel,	\
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
			KBEngine::Messagelog::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8);					\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##MessagelogMessagehandler8::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	class NAME##MessagelogMessagehandler8 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS8(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	MESSAGELOG_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	NETWORK_MESSAGE_DECLARE_ARGS8(Messagelog, NAME,								\
				NAME##MessagelogMessagehandler8, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\


/**
	Messagelog��Ϣ�꣬  ֻ�оŸ���������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS9
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	void NAME##MessagelogMessagehandler9::handle(Mercury::Channel* pChannel,	\
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
			KBEngine::Messagelog::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9);		\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	void NAME##MessagelogMessagehandler9::handle(Mercury::Channel* pChannel,	\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,			\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	class NAME##MessagelogMessagehandler9 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS9(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	MESSAGELOG_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1, 				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	NETWORK_MESSAGE_DECLARE_ARGS9(Messagelog, NAME,								\
				NAME##MessagelogMessagehandler9, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\

/**
	Messagelog��Ϣ�꣬  ֻ��ʮ����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS10
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	void NAME##MessagelogMessagehandler10::handle(Mercury::Channel* pChannel,		\
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
			KBEngine::Messagelog::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10);							\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	void NAME##MessagelogMessagehandler10::handle(Mercury::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	class NAME##MessagelogMessagehandler10 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS10(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	MESSAGELOG_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	NETWORK_MESSAGE_DECLARE_ARGS10(Messagelog, NAME,								\
				NAME##MessagelogMessagehandler10, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
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
	Messagelog��Ϣ�꣬  ֻ��ʮһ����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MESSAGELOG_MESSAGE_HANDLER_ARGS11
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MESSAGELOG)
#define MESSAGELOG_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
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
	void NAME##MessagelogMessagehandler11::handle(Mercury::Channel* pChannel,		\
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
			KBEngine::Messagelog::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11);				\
	}																			\

#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
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
	void NAME##MessagelogMessagehandler11::handle(Mercury::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MESSAGELOG_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
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
	class NAME##MessagelogMessagehandler11 : public Mercury::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Mercury::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MESSAGELOG_MESSAGE_DECLARE_ARGS11(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
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
	MESSAGELOG_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1, 					\
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
	NETWORK_MESSAGE_DECLARE_ARGS11(Messagelog, NAME,								\
				NAME##MessagelogMessagehandler11, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
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
