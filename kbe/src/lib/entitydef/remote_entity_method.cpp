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


#include "method.hpp"
#include "remote_entity_method.hpp"
#include "network/bundle.hpp"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(RemoteEntityMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
RemoteEntityMethod::RemoteEntityMethod(MethodDescription* methodDescription, EntityMailboxAbstract* mailbox):
script::ScriptObject(getScriptType(), false),
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
PyObject* RemoteEntityMethod::tp_call(PyObject* self, PyObject* args, PyObject* kwds)	
{	
	RemoteEntityMethod* rmethod = static_cast<RemoteEntityMethod*>(self);
	MethodDescription* methodDescription = rmethod->getDescription();
	EntityMailboxAbstract* mailbox = rmethod->getMailbox();
	// DEBUG_MSG("RemoteEntityMethod::tp_call:%s.\n", methodDescription->getName());

	if(methodDescription->checkArgs(args))
	{
		Mercury::Bundle bundle;
		mailbox->newMail(bundle);

		MemoryStream mstream;
		methodDescription->addToStream(&mstream, args);

		if(mstream.wpos() > 0)
			bundle.append(mstream.data(), mstream.wpos());

		mailbox->postMail(bundle);
	}
	
	S_Return;
}		
	
//-------------------------------------------------------------------------------------

}
