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

#include "witness.hpp"
#include "cellapp.hpp"
#include "entitydef/method.hpp"
#include "client_entity_method.hpp"
#include "network/bundle.hpp"
#include "helper/eventhistory_stats.hpp"

#include "client_lib/client_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(ClientEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ClientEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ClientEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ClientEntityMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
ClientEntityMethod::ClientEntityMethod(MethodDescription* methodDescription, 
		ENTITY_ID srcEntityID, ENTITY_ID clientEntityID):
script::ScriptObject(getScriptType(), false),
methodDescription_(methodDescription),
srcEntityID_(srcEntityID),
clientEntityID_(clientEntityID)
{
}

//-------------------------------------------------------------------------------------
ClientEntityMethod::~ClientEntityMethod()
{
}

//-------------------------------------------------------------------------------------
PyObject* ClientEntityMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{
	ClientEntityMethod* rmethod = static_cast<ClientEntityMethod*>(self);
	return rmethod->callmethod(args, kwds);	
}		

//-------------------------------------------------------------------------------------
PyObject* ClientEntityMethod::callmethod(PyObject* args, PyObject* kwds)
{
	Entity* srcEntity = Cellapp::getSingleton().findEntity(srcEntityID_);

	if(srcEntity == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "Entity::clientEntity(%s): srcEntityID(%d) not found!\n",		
			methodDescription_->getName(), srcEntityID_);		
		PyErr_PrintEx(0);
		return 0;
	}

	if(srcEntity->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "Entity::clientEntity(%s): srcEntityID(%d) is destroyed!\n",		
			methodDescription_->getName(), srcEntityID_);		
		PyErr_PrintEx(0);
		return 0;
	}

	if(srcEntity->pWitness() == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity(%s): no client, srcEntityID(%d).\n",		
			srcEntity->scriptName(), methodDescription_->getName(), srcEntity->id());		
		PyErr_PrintEx(0);
		return 0;
	}

	EntityRef::AOI_ENTITIES::iterator iter = srcEntity->pWitness()->aoiEntities().begin();
	Entity* e = NULL;

	for(; iter != srcEntity->pWitness()->aoiEntities().end(); iter++)
	{
		if((*iter)->id() == clientEntityID_ && ((*iter)->flags() & 
			(ENTITYREF_FLAG_ENTER_CLIENT_PENDING | ENTITYREF_FLAG_LEAVE_CLIENT_PENDING)) <= 0)
		{
			e = (*iter)->pEntity();
			break;
		}
	}

	if(e == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity(%s): not found entity(%d), srcEntityID(%d).\n",		
			srcEntity->scriptName(), methodDescription_->getName(), clientEntityID_, srcEntity->id());	

		PyErr_PrintEx(0);

		return 0;
	}

	MethodDescription* methodDescription = getDescription();
	if(methodDescription->checkArgs(args))
	{
		MemoryStream* mstream = MemoryStream::ObjPool().createObject();
		methodDescription->addToStream(mstream, args);

		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
		Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();

		srcEntity->pWitness()->addSmartAOIEntityMessageToBundle(pForwardBundle, ClientInterface::onRemoteMethodCall, 
				ClientInterface::onRemoteMethodCallOptimized, clientEntityID_);

		if(mstream->wpos() > 0)
			(*pForwardBundle).append(mstream->data(), mstream->wpos());

		if(Mercury::g_trace_packet > 0)
		{
			if(Mercury::g_trace_packet_use_logfile)
				DebugHelper::getSingleton().changeLogger("packetlogs");

			DEBUG_MSG(fmt::format("ClientEntityMethod::callmethod: pushUpdateData: ClientInterface::onRemoteOtherEntityMethodCall({}::{})\n",
				srcEntity->scriptName(), methodDescription->getName()));
																								
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

		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(srcEntity->id(), (*pSendBundle), (*pForwardBundle));

		srcEntity->pWitness()->sendToClient(ClientInterface::onRemoteMethodCallOptimized, pSendBundle);

		// ��¼����¼���������������С
		g_publicClientEventHistoryStats.trackEvent(srcEntity->scriptName(), 
			(std::string(e->scriptName()) + "." + methodDescription->getName()), 
			pForwardBundle->currMsgLength(), 
			"::");

		MemoryStream::ObjPool().reclaimObject(mstream);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	}

	S_Return;
}

//-------------------------------------------------------------------------------------

}
