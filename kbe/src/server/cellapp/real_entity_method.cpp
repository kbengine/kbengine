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

#include "cellapp.h"
#include "real_entity_method.h"
#include "entitydef/method.h"
#include "network/bundle.h"
#include "helper/eventhistory_stats.h"

#include "cellapp_interface.h"

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
	GhostManager* gm = Cellapp::getSingleton().pGhostManager();
	if (!gm)
	{
		S_Return;
	}

	MethodDescription* methodDescription = getDescription();
	if(methodDescription->checkArgs(args))
	{
		MemoryStream* mstream = MemoryStream::createPoolObject();
		methodDescription->addToStream(mstream, args);

		Network::Bundle* pForwardBundle = gm->createSendBundle(realCell_);

		(*pForwardBundle).newMessage(CellappInterface::onRemoteRealMethodCall);
		(*pForwardBundle) << ghostEntityID_;

		if(mstream->wpos() > 0)
			(*pForwardBundle).append(mstream->data(), mstream->wpos());

		if(Network::g_trace_packet > 0)
		{
			if(Network::g_trace_packet_use_logfile)
				DebugHelper::getSingleton().changeLogger("packetlogs");

			DEBUG_MSG(fmt::format("RealEntityMethod::callmethod: pushUpdateData: CellappInterface::onRemoteRealMethodCall({2}({0})::{1})\n",
				ghostEntityID_, methodDescription->getName(), scriptName_));

			switch(Network::g_trace_packet)
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

			if(Network::g_trace_packet_use_logfile)
				DebugHelper::getSingleton().changeLogger(COMPONENT_NAME_EX(g_componentType));
		}

		// 记录这个事件产生的数据量大小
		g_publicCellEventHistoryStats.trackEvent(scriptName_, 
			methodDescription->getName(), 
			pForwardBundle->currMsgLength(), 
			"::");

		MemoryStream::reclaimPoolObject(mstream);
		gm->pushMessage(realCell_, pForwardBundle);
	}

	S_Return;
}

//-------------------------------------------------------------------------------------

}
