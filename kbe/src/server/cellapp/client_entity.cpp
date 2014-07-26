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
#include "entityref.hpp"
#include "client_entity.hpp"
#include "helper/debug_helper.hpp"
#include "network/packet.hpp"
#include "network/bundle.hpp"
#include "network/network_interface.hpp"
#include "server/components.hpp"
#include "client_lib/client_interface.hpp"
#include "entitydef/method.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "client_entity_method.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"

namespace KBEngine{


SCRIPT_METHOD_DECLARE_BEGIN(ClientEntity)
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(ClientEntity)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ClientEntity)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ClientEntity, 0, 0, 0, 0, 0)		

//-------------------------------------------------------------------------------------
ClientEntity::ClientEntity(ENTITY_ID srcEntityID,
		ENTITY_ID clientEntityID):
ScriptObject(getScriptType(), false),
srcEntityID_(srcEntityID),
clientEntityID_(clientEntityID)
{
}

//-------------------------------------------------------------------------------------
ClientEntity::~ClientEntity()
{
}

//-------------------------------------------------------------------------------------
PyObject* ClientEntity::onScriptGetAttribute(PyObject* attr)
{
	Entity* srcEntity = Cellapp::getSingleton().findEntity(srcEntityID_);

	if(srcEntity == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "Entity::clientEntity: srcEntityID(%d) not found!\n",		
			 srcEntityID_);		
		PyErr_PrintEx(0);
		return 0;
	}

	if(srcEntity->isDestroyed())
	{
		PyErr_Format(PyExc_AssertionError, "Entity::clientEntity: srcEntityID(%d) is destroyed!\n",		
			srcEntityID_);		
		PyErr_PrintEx(0);
		return 0;
	}

	if(srcEntity->pWitness() == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity: no client, srcEntityID(%d).\n",		
			srcEntity->getScriptName(), srcEntity->getID());		
		PyErr_PrintEx(0);
		return 0;
	}

	EntityRef::AOI_ENTITIES::iterator iter = srcEntity->pWitness()->aoiEntities().begin();
	Entity* e = NULL;

	for(; iter != srcEntity->pWitness()->aoiEntities().end(); iter++)
	{
		if((*iter)->id() == clientEntityID_ && ((*iter)->flags() & ENTITYREF_FLAG_ENTER_CLIENT_PENDING) <= 0)
		{
			e = (*iter)->pEntity();
			break;
		}
	}

	if(e == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "%s::clientEntity: not found entity(%d), srcEntityID(%d).\n",		
			srcEntity->getScriptName(), clientEntityID_, srcEntity->getID());		
		PyErr_PrintEx(0);
		return 0;
	}

	wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);
	char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
	PyMem_Free(PyUnicode_AsWideCharStringRet0);

	MethodDescription* md = const_cast<ScriptDefModule*>(e->getScriptModule())->findClientMethodDescription(ccattr);
	free(ccattr);

	if(md != NULL)
	{
		return new ClientEntityMethod(md, srcEntityID_, clientEntityID_);
	}

	return ScriptObject::onScriptGetAttribute(attr);
}

//-------------------------------------------------------------------------------------
PyObject* ClientEntity::tp_repr()
{
	char s[1024];
	c_str(s, 1024);
	return PyUnicode_FromString(s);
}

//-------------------------------------------------------------------------------------
void ClientEntity::c_str(char* s, size_t size)
{
	kbe_snprintf(s, size, "clientEntity id:%d, srcEntityID=%d.", clientEntityID_, srcEntityID_);
}

//-------------------------------------------------------------------------------------
PyObject* ClientEntity::tp_str()
{
	return tp_repr();
}


//-------------------------------------------------------------------------------------

}

