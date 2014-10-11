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
#include "real_entity_method.hpp"
#include "entitydef/method.hpp"
#include "network/bundle.hpp"
#include "helper/eventhistory_stats.hpp"

#include "cellapp_interface.hpp"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(RealEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(RealEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(RealEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(RealEntityMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
RealEntityMethod::RealEntityMethod(MethodDescription* methodDescription, 
		Entity* ghostEntity):
script::ScriptObject(getScriptType(), false),
methodDescription_(methodDescription),
ghostEntityID_(ghostEntity->id()),
realCell_(ghostEntity->realCell()),
scriptName_(ghostEntity->scriptName())
{
}

//-------------------------------------------------------------------------------------
RealEntityMethod::~RealEntityMethod()
{
}

//-------------------------------------------------------------------------------------
PyObject* RealEntityMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{
	RealEntityMethod* rmethod = static_cast<RealEntityMethod*>(self);
	return rmethod->callmethod(args, kwds);	
}		

//-------------------------------------------------------------------------------------
PyObject* RealEntityMethod::callmethod(PyObject* args, PyObject* kwds)
{
	MethodDescription* methodDescription = getDescription();
	if(methodDescription->checkArgs(args))
	{
		MemoryStream* mstream = MemoryStream::ObjPool().createObject();
		methodDescription->addToStream(mstream, args);

		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
		
		(*pForwardBundle).newMessage(CellappInterface::onRemoteRealMethodCall);
		(*pForwardBundle) << ghostEntityID_;
		
		if(mstream->wpos() > 0)
			(*pForwardBundle).append(mstream->data(), mstream->wpos());

		if(Mercury::g_trace_packet > 0)
		{
			if(Mercury::g_trace_packet_use_logfile)
				DebugHelper::getSingleton().changeLogger("packetlogs");

			DEBUG_MSG(fmt::format("RealEntityMethod::callmethod: pushUpdateData: CellappInterface::onRemoteRealMethodCall({2}({0})::{1})\n",
				ghostEntityID_, methodDescription->getName(), scriptName_));

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

		// ��¼����¼���������������С
		g_publicCellEventHistoryStats.trackEvent(scriptName_, 
			methodDescription->getName(), 
			pForwardBundle->currMsgLength(), 
			"::");

		MemoryStream::ObjPool().reclaimObject(mstream);
		
		GhostManager* gm = Cellapp::getSingleton().pGhostManager();
		if(gm)
		{
			gm->pushMessage(realCell_, pForwardBundle);
		}
		else
		{
			Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
		}
	}

	S_Return;
}

//-------------------------------------------------------------------------------------

}
