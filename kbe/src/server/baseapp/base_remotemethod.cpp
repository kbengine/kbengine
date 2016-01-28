/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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
#include "base_remotemethod.h"
#include "entitydef/method.h"
#include "helper/profile.h"	
#include "network/bundle.h"
#include "helper/eventhistory_stats.h"

#include "client_lib/client_interface.h"

namespace KBEngine{	

SCRIPT_METHOD_DECLARE_BEGIN(BaseRemoteMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(BaseRemoteMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(BaseRemoteMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(BaseRemoteMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
BaseRemoteMethod::BaseRemoteMethod(MethodDescription* methodDescription, 
						EntityMailboxAbstract* mailbox):
RemoteEntityMethod(methodDescription, mailbox, getScriptType())
{
}

//-------------------------------------------------------------------------------------
BaseRemoteMethod::~BaseRemoteMethod()
{
}

//-------------------------------------------------------------------------------------
PyObject* BaseRemoteMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{	
	BaseRemoteMethod* rmethod = static_cast<BaseRemoteMethod*>(self);
	MethodDescription* methodDescription = rmethod->getDescription();
	EntityMailboxAbstract* mailbox = rmethod->getMailbox();

	if (!mailbox->isClient() || mailbox->type() == MAILBOX_TYPE_CLIENT_VIA_CELL /* 需要先经过cell */ )
	{
		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	Base* pEntity = Baseapp::getSingleton().findEntity(mailbox->id());
	if(pEntity == NULL)
	{
		//WARNING_MSG(fmt::format("BaseRemoteMethod::callClientMethod: not found entity({}).\n",
		//	mailbox->id()));

		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	// 如果是调用客户端方法， 我们记录事件并且记录带宽
	if(methodDescription->checkArgs(args))
	{
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		mailbox->newMail((*pBundle));

		MemoryStream* mstream = MemoryStream::ObjPool().createObject();
		methodDescription->addToStream(mstream, args);

		if(mstream->wpos() > 0)
			(*pBundle).append(mstream->data(), (int)mstream->wpos());

		// 记录这个事件产生的数据量大小
		g_privateClientEventHistoryStats.trackEvent(pEntity->scriptName(), 
			methodDescription->getName(), 
			pBundle->currMsgLength(), 
			"::");
		
		static_cast<Proxy*>(pEntity)->sendToClient(ClientInterface::onRemoteMethodCall, pBundle);

		MemoryStream::ObjPool().reclaimObject(mstream);
	}
	
	S_Return;
}	

//-------------------------------------------------------------------------------------
}
