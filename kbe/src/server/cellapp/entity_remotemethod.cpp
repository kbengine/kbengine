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

#include "cellapp.hpp"
#include "witness.hpp"
#include "entity_remotemethod.hpp"
#include "entitydef/method.hpp"
#include "helper/profile.hpp"	
#include "helper/eventhistory_stats.hpp"
#include "network/bundle.hpp"
#include "client_lib/client_interface.hpp"

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
						EntityMailboxAbstract* mailbox):
RemoteEntityMethod(methodDescription, mailbox, getScriptType())
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
	EntityMailboxAbstract* mailbox = rmethod->getMailbox();

	if(!mailbox->isClient())
	{
		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	Entity* pEntity = Cellapp::getSingleton().findEntity(mailbox->id());
	if(pEntity == NULL || pEntity->pWitness() == NULL)
	{
		//WARNING_MSG(fmt::format("EntityRemoteMethod::callClientMethod: not found entity({}).\n", 
		//	mailbox->id()));

		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	// ����ǵ��ÿͻ��˷����� ���Ǽ�¼�¼����Ҽ�¼����
	if(methodDescription->checkArgs(args))
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		mailbox->newMail((*pBundle));

		MemoryStream* mstream = MemoryStream::ObjPool().createObject();
		methodDescription->addToStream(mstream, args);

		if(mstream->wpos() > 0)
			(*pBundle).append(mstream->data(), mstream->wpos());

		if(Mercury::g_trace_packet > 0)
		{
			if(Mercury::g_trace_packet_use_logfile)
				DebugHelper::getSingleton().changeLogger("packetlogs");

			DEBUG_MSG(fmt::format("EntityRemoteMethod::tp_call: pushUpdateData: ClientInterface::onRemoteMethodCall({}::{})\n",
				pEntity->scriptName(), methodDescription->getName()));
																								
			switch(Mercury::g_trace_packet)																	
			{																								
			case 1:																							
				mstream->hexlike();																			
				break;																						
			case 2:																							
				mstream->textlike();																			
				break;																						
			default:																						
				mstream->print_storage();																	
				break;																						
			};																								

			if(Mercury::g_trace_packet_use_logfile)	
				DebugHelper::getSingleton().changeLogger(COMPONENT_NAME_EX(g_componentType));																				
		}

		//mailbox->postMail((*pBundle));
		pEntity->pWitness()->sendToClient(ClientInterface::onRemoteMethodCall, pBundle);

		//Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		MemoryStream::ObjPool().reclaimObject(mstream);

		// ��¼����¼���������������С
		g_privateClientEventHistoryStats.trackEvent(pEntity->scriptName(), 
			methodDescription->getName(), 
			pBundle->currMsgLength(), 
			"::");
	}
	
	S_Return;
}	

//-------------------------------------------------------------------------------------
}
