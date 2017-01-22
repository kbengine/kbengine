/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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
	#undef KBE_MACHINE_INTERFACE_MACRO_H
#endif


#ifndef KBE_MACHINE_INTERFACE_MACRO_H
#define KBE_MACHINE_INTERFACE_MACRO_H

// common include	
#include "network/interface_defs.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	Machine��Ϣ�꣬  ����Ϊ���� ��Ҫ�Լ��⿪
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_STREAM
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_STREAM(NAME)									\
	void NAME##MachineMessagehandler_stream::handle(Network::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
			KBEngine::Machine::getSingleton().NAME(pChannel, s);				\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_STREAM(NAME)									\
	void NAME##MachineMessagehandler_stream::handle(Network::Channel* pChannel,	\
													KBEngine::MemoryStream& s)	\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_STREAM(NAME)									\
	class NAME##MachineMessagehandler_stream : public Network::MessageHandler	\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_STREAM(NAME, MSG_LENGTH)						\
	MACHINE_MESSAGE_HANDLER_STREAM(NAME)										\
	NETWORK_MESSAGE_DECLARE_STREAM(Machine, NAME,								\
				NAME##MachineMessagehandler_stream, MSG_LENGTH)					\
																				\

/**
	Machine��Ϣ�꣬  ֻ�������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS0
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##MachineMessagehandler0::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			KBEngine::Machine::getSingleton().NAME(pChannel);					\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS0(NAME)										\
	void NAME##MachineMessagehandler0::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS0(NAME)										\
	class NAME##MachineMessagehandler0 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS0(NAME, MSG_LENGTH)							\
	MACHINE_MESSAGE_HANDLER_ARGS0(NAME)											\
	NETWORK_MESSAGE_DECLARE_ARGS0(Machine, NAME,								\
				NAME##MachineMessagehandler0, MSG_LENGTH)						\
																				\


/**
	Machine��Ϣ�꣬  ֻ��һ����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	void NAME##MachineMessagehandler1::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			KBEngine::Machine::getSingleton().NAME(pChannel, ARG_NAME1);		\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	void NAME##MachineMessagehandler1::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)				\
	class NAME##MachineMessagehandler1 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS1(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
	MACHINE_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME1)					\
	NETWORK_MESSAGE_DECLARE_ARGS1(Machine, NAME,								\
				NAME##MachineMessagehandler1, MSG_LENGTH, ARG_TYPE1, ARG_NAME1)	\
																				\

/**
	Machine��Ϣ�꣬  ֻ�ж�����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS2
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##MachineMessagehandler2::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2);											\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2)				\
	void NAME##MachineMessagehandler2::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2)				\
	class NAME##MachineMessagehandler2 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS2(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2)				\
	MACHINE_MESSAGE_HANDLER_ARGS2(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2)				\
	NETWORK_MESSAGE_DECLARE_ARGS2(Machine, NAME,								\
				NAME##MachineMessagehandler2, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2)				\


/**
	Machine��Ϣ�꣬  ֻ��������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS3
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##MachineMessagehandler3::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
			ARG_TYPE1 ARG_NAME1;												\
			s >> ARG_NAME1;														\
			ARG_TYPE2 ARG_NAME2;												\
			s >> ARG_NAME2;														\
			ARG_TYPE3 ARG_NAME3;												\
			s >> ARG_NAME3;														\
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3);														\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	void NAME##MachineMessagehandler3::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	class NAME##MachineMessagehandler3 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS3(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	MACHINE_MESSAGE_HANDLER_ARGS3(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\
	NETWORK_MESSAGE_DECLARE_ARGS3(Machine, NAME,								\
				NAME##MachineMessagehandler3, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3)				\


/**
	Machine��Ϣ�꣬  ֻ���ĸ���������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS4
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##MachineMessagehandler4::handle(Network::Channel* pChannel,		\
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
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4);											\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	void NAME##MachineMessagehandler4::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	class NAME##MachineMessagehandler4 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS4(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	MACHINE_MESSAGE_HANDLER_ARGS4(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\
	NETWORK_MESSAGE_DECLARE_ARGS4(Machine, NAME,								\
				NAME##MachineMessagehandler4, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4)				\

/**
	Machine��Ϣ�꣬  ֻ�������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS5
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##MachineMessagehandler5::handle(Network::Channel* pChannel,		\
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
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4, ARG_NAME5);								\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	void NAME##MachineMessagehandler5::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	class NAME##MachineMessagehandler5 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS5(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	MACHINE_MESSAGE_HANDLER_ARGS5(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\
	NETWORK_MESSAGE_DECLARE_ARGS5(Machine, NAME,								\
				NAME##MachineMessagehandler5, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5)				\


/**
	Machine��Ϣ�꣬  ֻ��������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS6
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##MachineMessagehandler6::handle(Network::Channel* pChannel,		\
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
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
				ARG_NAME1, ARG_NAME2, 											\
				ARG_NAME3, ARG_NAME4, ARG_NAME5, ARG_NAME6);					\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	void NAME##MachineMessagehandler6::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	class NAME##MachineMessagehandler6 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS6(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	MACHINE_MESSAGE_HANDLER_ARGS6(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\
	NETWORK_MESSAGE_DECLARE_ARGS6(Machine, NAME,								\
				NAME##MachineMessagehandler6, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6)				\


/**
	Machine��Ϣ�꣬  ֻ���߸���������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS7
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	void NAME##MachineMessagehandler7::handle(Network::Channel* pChannel,		\
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
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7);								\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	void NAME##MachineMessagehandler7::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	class NAME##MachineMessagehandler7 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS7(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	MACHINE_MESSAGE_HANDLER_ARGS7(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\
	NETWORK_MESSAGE_DECLARE_ARGS7(Machine, NAME,								\
				NAME##MachineMessagehandler7, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7)				\



/**
	Machine��Ϣ�꣬  ֻ�а˸���������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS8
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##MachineMessagehandler8::handle(Network::Channel* pChannel,		\
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
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8);					\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	void NAME##MachineMessagehandler8::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	class NAME##MachineMessagehandler8 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS8(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	MACHINE_MESSAGE_HANDLER_ARGS8(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\
	NETWORK_MESSAGE_DECLARE_ARGS8(Machine, NAME,								\
				NAME##MachineMessagehandler8, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8)				\


/**
	Machine��Ϣ�꣬  ֻ�оŸ���������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS9
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	void NAME##MachineMessagehandler9::handle(Network::Channel* pChannel,		\
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
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9);		\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	void NAME##MachineMessagehandler9::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	class NAME##MachineMessagehandler9 : public Network::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS9(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	MACHINE_MESSAGE_HANDLER_ARGS9(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\
	NETWORK_MESSAGE_DECLARE_ARGS9(Machine, NAME,								\
				NAME##MachineMessagehandler9, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9)				\


/**
	Machine��Ϣ�꣬  ֻ��ʮ����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS10
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	void NAME##MachineMessagehandler10::handle(Network::Channel* pChannel,		\
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
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10);							\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	void NAME##MachineMessagehandler10::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	class NAME##MachineMessagehandler10 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS10(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	MACHINE_MESSAGE_HANDLER_ARGS10(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10)				\
	NETWORK_MESSAGE_DECLARE_ARGS10(Machine, NAME,								\
				NAME##MachineMessagehandler10, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
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
	Machine��Ϣ�꣬  ֻ��ʮһ����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS11
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
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
	void NAME##MachineMessagehandler11::handle(Network::Channel* pChannel,		\
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
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11);				\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
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
	void NAME##MachineMessagehandler11::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1,				\
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
	class NAME##MachineMessagehandler11 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS11(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
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
	MACHINE_MESSAGE_HANDLER_ARGS11(NAME, ARG_TYPE1, ARG_NAME1, 					\
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
	NETWORK_MESSAGE_DECLARE_ARGS11(Machine, NAME,								\
				NAME##MachineMessagehandler11, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
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


/**
	Machine��Ϣ�꣬  ֻ��ʮ�����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS15
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS15(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15)				\
	void NAME##MachineMessagehandler15::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE12 ARG_NAME12;												\
			s >> ARG_NAME12;													\
			ARG_TYPE13 ARG_NAME13;												\
			s >> ARG_NAME13;													\
			ARG_TYPE14 ARG_NAME14;												\
			s >> ARG_NAME14;													\
			ARG_TYPE15 ARG_NAME15;												\
			s >> ARG_NAME15;													\
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11, ARG_NAME12,		\
										ARG_NAME13, ARG_NAME14, ARG_NAME15);	\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS15(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15)				\
	void NAME##MachineMessagehandler15::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS15(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15)				\
	class NAME##MachineMessagehandler15 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS15(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15)				\
	MACHINE_MESSAGE_HANDLER_ARGS15(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15)				\
	NETWORK_MESSAGE_DECLARE_ARGS15(Machine, NAME,								\
				NAME##MachineMessagehandler15, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15)				\


/**
	Machine��Ϣ�꣬  ֻ��ʮ������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS16
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS16(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16)				\
	void NAME##MachineMessagehandler16::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE12 ARG_NAME12;												\
			s >> ARG_NAME12;													\
			ARG_TYPE13 ARG_NAME13;												\
			s >> ARG_NAME13;													\
			ARG_TYPE14 ARG_NAME14;												\
			s >> ARG_NAME14;													\
			ARG_TYPE15 ARG_NAME15;												\
			s >> ARG_NAME15;													\
			ARG_TYPE16 ARG_NAME16;												\
			s >> ARG_NAME16;													\
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11, ARG_NAME12,		\
										ARG_NAME13, ARG_NAME14, ARG_NAME15,		\
										ARG_NAME16);							\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS16(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16)				\
	void NAME##MachineMessagehandler16::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS16(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16)				\
	class NAME##MachineMessagehandler16 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS16(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16)				\
	MACHINE_MESSAGE_HANDLER_ARGS16(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16)				\
	NETWORK_MESSAGE_DECLARE_ARGS16(Machine, NAME,								\
				NAME##MachineMessagehandler16, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16)				\


/**
	Machine��Ϣ�꣬  ֻ��ʮ�˸���������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS18
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS18(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18)				\
	void NAME##MachineMessagehandler18::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE12 ARG_NAME12;												\
			s >> ARG_NAME12;													\
			ARG_TYPE13 ARG_NAME13;												\
			s >> ARG_NAME13;													\
			ARG_TYPE14 ARG_NAME14;												\
			s >> ARG_NAME14;													\
			ARG_TYPE15 ARG_NAME15;												\
			s >> ARG_NAME15;													\
			ARG_TYPE16 ARG_NAME16;												\
			s >> ARG_NAME16;													\
			ARG_TYPE17 ARG_NAME17;												\
			s >> ARG_NAME17;													\
			ARG_TYPE18 ARG_NAME18;												\
			s >> ARG_NAME18;													\
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11, ARG_NAME12,		\
										ARG_NAME13, ARG_NAME14, ARG_NAME15,		\
										ARG_NAME16, ARG_NAME17, ARG_NAME18);	\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS18(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18)				\
	void NAME##MachineMessagehandler18::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS18(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18)				\
	class NAME##MachineMessagehandler18 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS18(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18)				\
	MACHINE_MESSAGE_HANDLER_ARGS18(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18)				\
	NETWORK_MESSAGE_DECLARE_ARGS18(Machine, NAME,								\
				NAME##MachineMessagehandler18, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18)				\


/**
	Machine��Ϣ�꣬  ֻ�ж�ʮ����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS20
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS20(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20)				\
	void NAME##MachineMessagehandler20::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE12 ARG_NAME12;												\
			s >> ARG_NAME12;													\
			ARG_TYPE13 ARG_NAME13;												\
			s >> ARG_NAME13;													\
			ARG_TYPE14 ARG_NAME14;												\
			s >> ARG_NAME14;													\
			ARG_TYPE15 ARG_NAME15;												\
			s >> ARG_NAME15;													\
			ARG_TYPE16 ARG_NAME16;												\
			s >> ARG_NAME16;													\
			ARG_TYPE17 ARG_NAME17;												\
			s >> ARG_NAME17;													\
			ARG_TYPE18 ARG_NAME18;												\
			s >> ARG_NAME18;													\
			ARG_TYPE19 ARG_NAME19;												\
			s >> ARG_NAME19;													\
			ARG_TYPE20 ARG_NAME20;												\
			s >> ARG_NAME20;													\
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11, ARG_NAME12,		\
										ARG_NAME13, ARG_NAME14, ARG_NAME15,		\
										ARG_NAME16, ARG_NAME17, ARG_NAME18,		\
										ARG_NAME19, ARG_NAME20);				\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS20(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20)				\
	void NAME##MachineMessagehandler20::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS20(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20)				\
	class NAME##MachineMessagehandler20 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS20(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20)				\
	MACHINE_MESSAGE_HANDLER_ARGS20(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20)				\
	NETWORK_MESSAGE_DECLARE_ARGS20(Machine, NAME,								\
				NAME##MachineMessagehandler20, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20)				\


/**
	Machine��Ϣ�꣬  ֻ�ж�ʮһ����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS21
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS21(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21)				\
	void NAME##MachineMessagehandler21::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE12 ARG_NAME12;												\
			s >> ARG_NAME12;													\
			ARG_TYPE13 ARG_NAME13;												\
			s >> ARG_NAME13;													\
			ARG_TYPE14 ARG_NAME14;												\
			s >> ARG_NAME14;													\
			ARG_TYPE15 ARG_NAME15;												\
			s >> ARG_NAME15;													\
			ARG_TYPE16 ARG_NAME16;												\
			s >> ARG_NAME16;													\
			ARG_TYPE17 ARG_NAME17;												\
			s >> ARG_NAME17;													\
			ARG_TYPE18 ARG_NAME18;												\
			s >> ARG_NAME18;													\
			ARG_TYPE19 ARG_NAME19;												\
			s >> ARG_NAME19;													\
			ARG_TYPE20 ARG_NAME20;												\
			s >> ARG_NAME20;													\
			ARG_TYPE21 ARG_NAME21;												\
			s >> ARG_NAME21;													\
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11, ARG_NAME12,		\
										ARG_NAME13, ARG_NAME14, ARG_NAME15,		\
										ARG_NAME16, ARG_NAME17, ARG_NAME18,		\
										ARG_NAME19, ARG_NAME20, ARG_NAME21);	\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS21(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21)				\
	void NAME##MachineMessagehandler21::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS21(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21)				\
	class NAME##MachineMessagehandler21 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS21(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21)				\
	MACHINE_MESSAGE_HANDLER_ARGS21(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21)				\
	NETWORK_MESSAGE_DECLARE_ARGS21(Machine, NAME,								\
				NAME##MachineMessagehandler21, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21)				\



/**
	Machine��Ϣ�꣬  ֻ�ж�ʮ������������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS22
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS22(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22)				\
	void NAME##MachineMessagehandler22::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE12 ARG_NAME12;												\
			s >> ARG_NAME12;													\
			ARG_TYPE13 ARG_NAME13;												\
			s >> ARG_NAME13;													\
			ARG_TYPE14 ARG_NAME14;												\
			s >> ARG_NAME14;													\
			ARG_TYPE15 ARG_NAME15;												\
			s >> ARG_NAME15;													\
			ARG_TYPE16 ARG_NAME16;												\
			s >> ARG_NAME16;													\
			ARG_TYPE17 ARG_NAME17;												\
			s >> ARG_NAME17;													\
			ARG_TYPE18 ARG_NAME18;												\
			s >> ARG_NAME18;													\
			ARG_TYPE19 ARG_NAME19;												\
			s >> ARG_NAME19;													\
			ARG_TYPE20 ARG_NAME20;												\
			s >> ARG_NAME20;													\
			ARG_TYPE21 ARG_NAME21;												\
			s >> ARG_NAME21;													\
			ARG_TYPE22 ARG_NAME22;												\
			s >> ARG_NAME22;													\
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11, ARG_NAME12,		\
										ARG_NAME13, ARG_NAME14, ARG_NAME15,		\
										ARG_NAME16, ARG_NAME17, ARG_NAME18,		\
										ARG_NAME19, ARG_NAME20, ARG_NAME21,		\
										ARG_NAME22);							\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS22(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22)				\
	void NAME##MachineMessagehandler22::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS22(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22)				\
	class NAME##MachineMessagehandler22 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS22(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22)				\
	MACHINE_MESSAGE_HANDLER_ARGS22(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22)				\
	NETWORK_MESSAGE_DECLARE_ARGS22(Machine, NAME,								\
				NAME##MachineMessagehandler22, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22)				\



/**
	Machine��Ϣ�꣬  ֻ�ж�ʮ�ĸ���������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef MACHINE_MESSAGE_HANDLER_ARGS24
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS24(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24)				\
	void NAME##MachineMessagehandler24::handle(Network::Channel* pChannel,		\
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
			ARG_TYPE12 ARG_NAME12;												\
			s >> ARG_NAME12;													\
			ARG_TYPE13 ARG_NAME13;												\
			s >> ARG_NAME13;													\
			ARG_TYPE14 ARG_NAME14;												\
			s >> ARG_NAME14;													\
			ARG_TYPE15 ARG_NAME15;												\
			s >> ARG_NAME15;													\
			ARG_TYPE16 ARG_NAME16;												\
			s >> ARG_NAME16;													\
			ARG_TYPE17 ARG_NAME17;												\
			s >> ARG_NAME17;													\
			ARG_TYPE18 ARG_NAME18;												\
			s >> ARG_NAME18;													\
			ARG_TYPE19 ARG_NAME19;												\
			s >> ARG_NAME19;													\
			ARG_TYPE20 ARG_NAME20;												\
			s >> ARG_NAME20;													\
			ARG_TYPE21 ARG_NAME21;												\
			s >> ARG_NAME21;													\
			ARG_TYPE22 ARG_NAME22;												\
			s >> ARG_NAME22;													\
			ARG_TYPE23 ARG_NAME23;												\
			s >> ARG_NAME23;													\
			ARG_TYPE24 ARG_NAME24;												\
			s >> ARG_NAME24;													\
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11, ARG_NAME12,		\
										ARG_NAME13, ARG_NAME14, ARG_NAME15,		\
										ARG_NAME16, ARG_NAME17, ARG_NAME18,		\
										ARG_NAME19, ARG_NAME20, ARG_NAME21,		\
										ARG_NAME22, ARG_NAME23, ARG_NAME24);	\
	}																			\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS24(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24)				\
	void NAME##MachineMessagehandler24::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
	{																			\
	}																			\
		
#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS24(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24)				\
	class NAME##MachineMessagehandler24 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS24(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24)				\
	MACHINE_MESSAGE_HANDLER_ARGS24(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24)				\
	NETWORK_MESSAGE_DECLARE_ARGS24(Machine, NAME,								\
				NAME##MachineMessagehandler24, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24)				\



/**
Machine��Ϣ�꣬  ֻ�ж�ʮ�����������Ϣ
*/
#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
#undef MACHINE_MESSAGE_HANDLER_ARGS25
#endif

#if defined(DEFINE_IN_INTERFACE)
#if defined(MACHINE)
#define MACHINE_MESSAGE_HANDLER_ARGS25(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24,				\
											ARG_TYPE25, ARG_NAME25)				\
	void NAME##MachineMessagehandler25::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
		{																		\
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
			ARG_TYPE12 ARG_NAME12;												\
			s >> ARG_NAME12;													\
			ARG_TYPE13 ARG_NAME13;												\
			s >> ARG_NAME13;													\
			ARG_TYPE14 ARG_NAME14;												\
			s >> ARG_NAME14;													\
			ARG_TYPE15 ARG_NAME15;												\
			s >> ARG_NAME15;													\
			ARG_TYPE16 ARG_NAME16;												\
			s >> ARG_NAME16;													\
			ARG_TYPE17 ARG_NAME17;												\
			s >> ARG_NAME17;													\
			ARG_TYPE18 ARG_NAME18;												\
			s >> ARG_NAME18;													\
			ARG_TYPE19 ARG_NAME19;												\
			s >> ARG_NAME19;													\
			ARG_TYPE20 ARG_NAME20;												\
			s >> ARG_NAME20;													\
			ARG_TYPE21 ARG_NAME21;												\
			s >> ARG_NAME21;													\
			ARG_TYPE22 ARG_NAME22;												\
			s >> ARG_NAME22;													\
			ARG_TYPE23 ARG_NAME23;												\
			s >> ARG_NAME23;													\
			ARG_TYPE24 ARG_NAME24;												\
			s >> ARG_NAME24;													\
			ARG_TYPE25 ARG_NAME25;												\
			s >> ARG_NAME25;													\
			KBEngine::Machine::getSingleton().NAME(pChannel,					\
										ARG_NAME1, ARG_NAME2, ARG_NAME3, 		\
										ARG_NAME4, ARG_NAME5, ARG_NAME6,		\
										ARG_NAME7, ARG_NAME8, ARG_NAME9,		\
										ARG_NAME10, ARG_NAME11, ARG_NAME12,		\
										ARG_NAME13, ARG_NAME14, ARG_NAME15,		\
										ARG_NAME16, ARG_NAME17, ARG_NAME18,		\
										ARG_NAME19, ARG_NAME20, ARG_NAME21,		\
										ARG_NAME22, ARG_NAME23, ARG_NAME24,		\
										ARG_NAME25);							\
		}																		\

#else
#define MACHINE_MESSAGE_HANDLER_ARGS25(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24,				\
											ARG_TYPE25, ARG_NAME25)				\
	void NAME##MachineMessagehandler25::handle(Network::Channel* pChannel,		\
												KBEngine::MemoryStream& s)		\
		{																		\
		}																		\

#endif
#else
#define MACHINE_MESSAGE_HANDLER_ARGS25(NAME, ARG_TYPE1, ARG_NAME1,				\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24,				\
											ARG_TYPE25, ARG_NAME25)				\
	class NAME##MachineMessagehandler25 : public Network::MessageHandler		\
	{																			\
	public:																		\
		virtual void handle(Network::Channel* pChannel,							\
							KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define MACHINE_MESSAGE_DECLARE_ARGS25(NAME, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,	\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24,				\
											ARG_TYPE25, ARG_NAME25)				\
	MACHINE_MESSAGE_HANDLER_ARGS25(NAME, ARG_TYPE1, ARG_NAME1, 					\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24,				\
											ARG_TYPE25, ARG_NAME25)				\
	NETWORK_MESSAGE_DECLARE_ARGS25(Machine, NAME,								\
				NAME##MachineMessagehandler25, MSG_LENGTH, ARG_TYPE1, ARG_NAME1,\
											ARG_TYPE2, ARG_NAME2,				\
											ARG_TYPE3, ARG_NAME3,				\
											ARG_TYPE4, ARG_NAME4,				\
											ARG_TYPE5, ARG_NAME5,				\
											ARG_TYPE6, ARG_NAME6,				\
											ARG_TYPE7, ARG_NAME7,				\
											ARG_TYPE8, ARG_NAME8,				\
											ARG_TYPE9, ARG_NAME9,				\
											ARG_TYPE10, ARG_NAME10,				\
											ARG_TYPE11, ARG_NAME11,				\
											ARG_TYPE12, ARG_NAME12,				\
											ARG_TYPE13, ARG_NAME13,				\
											ARG_TYPE14, ARG_NAME14,				\
											ARG_TYPE15, ARG_NAME15,				\
											ARG_TYPE16, ARG_NAME16,				\
											ARG_TYPE17, ARG_NAME17,				\
											ARG_TYPE18, ARG_NAME18,				\
											ARG_TYPE19, ARG_NAME19,				\
											ARG_TYPE20, ARG_NAME20,				\
											ARG_TYPE21, ARG_NAME21,				\
											ARG_TYPE22, ARG_NAME22,				\
											ARG_TYPE23, ARG_NAME23,				\
											ARG_TYPE24, ARG_NAME24,				\
											ARG_TYPE25, ARG_NAME25)				\


}
#endif
