// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "baseapp.h"
#include "entity_remotemethod.h"
#include "entitydef/method.h"
#include "helper/profile.h"	
#include "network/bundle.h"
#include "helper/eventhistory_stats.h"

#include "client_lib/client_interface.h"

namespace KBEngine{	

SCRIPT_METHOD_DECLARE_BEGIN(EntityRemoteMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(EntityRemoteMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(EntityRemoteMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(EntityRemoteMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
EntityRemoteMethod::EntityRemoteMethod(MethodDescription* methodDescription, 
						EntityCallAbstract* entityCall):
RemoteEntityMethod(methodDescription, entityCall, getScriptType())
{
}

//-------------------------------------------------------------------------------------
EntityRemoteMethod::~EntityRemoteMethod()
{
}

//-------------------------------------------------------------------------------------
PyObject* EntityRemoteMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{	
	EntityRemoteMethod* rmethod = static_cast<EntityRemoteMethod*>(self);
	MethodDescription* methodDescription = rmethod->getDescription();
	EntityCallAbstract* entityCall = rmethod->getEntityCall();

	if (!entityCall->isClient() || entityCall->type() == ENTITYCALL_TYPE_CLIENT_VIA_CELL /* 需要先经过cell */ )
	{
		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	Entity* pEntity = Baseapp::getSingleton().findEntity(entityCall->id());
	if(pEntity == NULL)
	{
		//WARNING_MSG(fmt::format("EntityRemoteMethod::callClientMethod: not found entity({}).\n",
		//	entityCall->id()));

		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	// 如果是调用客户端方法， 我们记录事件并且记录带宽
	if(methodDescription->checkArgs(args))
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		entityCall->newCall((*pBundle));

		MemoryStream* mstream = MemoryStream::createPoolObject(OBJECTPOOL_POINT);

		try
		{
			methodDescription->addToStream(mstream, args);
		}
		catch (MemoryStreamWriteOverflow & err)
		{
			ERROR_MSG(fmt::format("EntityRemoteMethod::tp_call(): {}.{}() {}, error={}\n",
				entityCall->pScriptDefModule()->getName(), rmethod->getName(), entityCall->id(), err.what()));

			MemoryStream::reclaimPoolObject(mstream);
			S_Return;
		}

		if(mstream->wpos() > 0)
			(*pBundle).append(mstream->data(), (int)mstream->wpos());

		// 记录这个事件产生的数据量大小
		g_privateClientEventHistoryStats.trackEvent(pEntity->scriptName(), 
			methodDescription->getName(), 
			pBundle->currMsgLength(), 
			"::");
		
		static_cast<Proxy*>(pEntity)->sendToClient(ClientInterface::onRemoteMethodCall, pBundle);

		MemoryStream::reclaimPoolObject(mstream);
	}
	
	S_Return;
}	

//-------------------------------------------------------------------------------------
}
