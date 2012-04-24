/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#if defined(DEFINE_IN_INTERFACE)
	#undef __CELLAPP_INTERFACE_H__
#endif


#ifndef __CELLAPP_INTERFACE_H__
#define __CELLAPP_INTERFACE_H__

// common include	
#include "entity.hpp"
#include "cellapp.hpp"
#include "network/interface_defs.hpp"
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

#if defined(NETWORK_INTERFACE_DECLARE_BEGIN)
	#undef ENTITY_MESSAGE_HANDLER_ARGS1
#endif

#if defined(DEFINE_IN_INTERFACE)

#define ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME)					\
	void NAME##EntityMessagehandler::handle(KBEngine::MemoryStream& s)			\
	{																			\
			ENTITY_ID eid;														\
			s >> eid;															\
			KBEngine::Entity* e =												\
					KBEngine::CellApp::getSingleton().findEntity(eid);			\
			ARG_TYPE1 ARG_NAME;													\
			s >> ARG_NAME;														\
			e->NAME(ARG_NAME);													\
	}																			\

#else
#define ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME)					\
	class NAME##EntityMessagehandler : public Mercury::MessageHandler			\
	{																			\
	public:																		\
		virtual void handle(KBEngine::MemoryStream& s);							\
	};																			\

#endif

#define ENTITY_MESSAGE_DECLARE_ARGS1(NAME, ARG_TYPE1, ARG_NAME)					\
	ENTITY_MESSAGE_HANDLER_ARGS1(NAME, ARG_TYPE1, ARG_NAME)						\
	NETWORK_MESSAGE_DECLARE_ARGS1(Entity, NAME,									\
				NAME##EntityMessagehandler, ARG_TYPE1, ARG_NAME)				\
																				\


NETWORK_INTERFACE_DECLARE_BEGIN(CellAppInterface)
	
	ENTITY_MESSAGE_DECLARE_ARGS1(test,
								std::string, name
	)
	
NETWORK_INTERFACE_DECLARE_END()
	
}
#endif
