/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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
						EntityCallAbstract* entitycall):
RemoteEntityMethod(methodDescription, entitycall, getScriptType())
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
	EntityCallAbstract* entitycall = rmethod->getEntityCall();

	if (!entitycall->isClient() || entitycall->type() == ENTITYCALL_TYPE_CLIENT_VIA_CELL /* 需要先经过cell */ )
	{
		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	Entity* pEntity = Baseapp::getSingleton().findEntity(entitycall->id());
	if(pEntity == NULL)
	{
		//WARNING_MSG(fmt::format("EntityRemoteMethod::callClientMethod: not found entity({}).\n",
		//	entitycall->id()));

		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	// 如果是调用客户端方法， 我们记录事件并且记录带宽
	if(methodDescription->checkArgs(args))
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		entitycall->newCall((*pBundle));

		MemoryStream* mstream = MemoryStream::createPoolObject();
		methodDescription->addToStream(mstream, args);

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
