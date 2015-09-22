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


#include "method.h"
#include "remote_entity_method.h"
#include "network/bundle.h"
#include "helper/debug_helper.h"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(RemoteEntityMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
RemoteEntityMethod::RemoteEntityMethod(MethodDescription* methodDescription, 
	EntityMailboxAbstract* mailbox, PyTypeObject* pyType):
script::ScriptObject((pyType == NULL ? getScriptType() : pyType), false),
methodDescription_(methodDescription),
pMailbox_(mailbox)
{
	Py_INCREF(pMailbox_);
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod::~RemoteEntityMethod()
{
	Py_DECREF(pMailbox_);
}

//-------------------------------------------------------------------------------------
const char* RemoteEntityMethod::getName(void) const
{ 
	return methodDescription_->getName(); 
};

//-------------------------------------------------------------------------------------
PyObject* RemoteEntityMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{	
	RemoteEntityMethod* rmethod = static_cast<RemoteEntityMethod*>(self);
	MethodDescription* methodDescription = rmethod->getDescription();
	EntityMailboxAbstract* mailbox = rmethod->getMailbox();
	// DEBUG_MSG(fmt::format("RemoteEntityMethod::tp_call:{}.\n"), methodDescription->getName()));

	if(methodDescription->checkArgs(args))
	{
		Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
		mailbox->newMail((*pBundle));

		MemoryStream mstream;
		methodDescription->addToStream(&mstream, args);

		if(mstream.wpos() > 0)
			(*pBundle).append(mstream.data(), mstream.wpos());

		mailbox->postMail(pBundle);
	}
	else
	{
        ERROR_MSG(fmt::format("RemoteEntityMethod::tp_call:{} checkArgs error!\n",
                methodDescription->getName()));
	}

	S_Return;
}		
	
//-------------------------------------------------------------------------------------

}
