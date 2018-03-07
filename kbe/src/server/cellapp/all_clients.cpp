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


#include "all_clients.h"
#include "entity.h"
#include "cellapp.h"
#include "pyscript/pickler.h"
#include "helper/debug_helper.h"
#include "network/packet.h"
#include "network/bundle.h"
#include "network/network_interface.h"
#include "server/components.h"
#include "client_lib/client_interface.h"
#include "entitydef/method.h"
#include "entitydef/scriptdef_module.h"
#include "clients_remote_entity_method.h"

#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"

namespace KBEngine{


SCRIPT_METHOD_DECLARE_BEGIN(AllClients)
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(AllClients)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(AllClients)
SCRIPT_GET_DECLARE("id",							pyGetID,				0,					0)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(AllClients, 0, 0, 0, 0, 0)

//-------------------------------------------------------------------------------------
AllClients::AllClients(const ScriptDefModule* pScriptModule, 
						ENTITY_ID eid, 
						bool otherClients):
ScriptObject(getScriptType(), false),
pScriptModule_(pScriptModule),
id_(eid),
otherClients_(otherClients)
{
}

//-------------------------------------------------------------------------------------
AllClients::~AllClients()
{
}

//-------------------------------------------------------------------------------------
PyObject* AllClients::pyGetID()
{ 
	return PyLong_FromLong(id()); 
}

//-------------------------------------------------------------------------------------
PyObject* AllClients::onScriptGetAttribute(PyObject* attr)
{
	Entity* pEntity = Cellapp::getSingleton().findEntity(id_);
	if(pEntity == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "AllClients::onScriptGetAttribute: not found entity(%d).", 
			id());
		PyErr_PrintEx(0);
		return 0;
	}

	if(!pEntity->isReal())
	{
		PyErr_Format(PyExc_AssertionError, "AllClients::onScriptGetAttribute: %s not is real entity(%d).", 
			pEntity->scriptName(), pEntity->id());
		PyErr_PrintEx(0);
		return 0;
	}
	
	wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);
	char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
	PyMem_Free(PyUnicode_AsWideCharStringRet0);

	MethodDescription* pMethodDescription = const_cast<ScriptDefModule*>(pScriptModule_)->findClientMethodDescription(ccattr);
	
	if(pMethodDescription != NULL)
	{
		free(ccattr);
		return new ClientsRemoteEntityMethod(pMethodDescription, otherClients_, id_);
	}

	free(ccattr);
	return ScriptObject::onScriptGetAttribute(attr);
}

//-------------------------------------------------------------------------------------
PyObject* AllClients::tp_repr()
{
	char s[1024];
	c_str(s, 1024);
	return PyUnicode_FromString(s);
}

//-------------------------------------------------------------------------------------
void AllClients::c_str(char* s, size_t size)
{
	kbe_snprintf(s, size, "clients id:%d.", id_);
}

//-------------------------------------------------------------------------------------
PyObject* AllClients::tp_str()
{
	return tp_repr();
}

//-------------------------------------------------------------------------------------

}

