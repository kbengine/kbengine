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


#include "entitydef/method.hpp"
#include "clients_remote_entity_method.hpp"
#include "network/bundle.hpp"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(ClientsRemoteEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ClientsRemoteEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ClientsRemoteEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ClientsRemoteEntityMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
ClientsRemoteEntityMethod::ClientsRemoteEntityMethod(MethodDescription* methodDescription,
													 bool otherClients,
													 ENTITY_ID id):
script::ScriptObject(getScriptType(), false),
methodDescription_(methodDescription),
otherClients_(otherClients),
id_(id)
{
}

//-------------------------------------------------------------------------------------
ClientsRemoteEntityMethod::~ClientsRemoteEntityMethod()
{
}

//-------------------------------------------------------------------------------------
PyObject* ClientsRemoteEntityMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{	
	// 获取entityAOI范围内其他entity
	// 向这些entity的client推送这个方法的调用
	ClientsRemoteEntityMethod* rmethod = static_cast<ClientsRemoteEntityMethod*>(self);
	MethodDescription* methodDescription = rmethod->getDescription();

	if(methodDescription->checkArgs(args))
	{
	}

	S_Return;
}		
	
//-------------------------------------------------------------------------------------

}
