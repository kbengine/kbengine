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


#include "entity_mailbox.hpp"
#include "scriptdef_module.hpp"
#include "helper/debug_helper.hpp"
#include "network/channel.hpp"	
#include "pyscript/pickler.hpp"
#include "entitydef/method.hpp"
#include "remote_entity_method.hpp"
#include "entitydef/entitydef.hpp"

#ifdef KBE_SERVER
#include "server/components.hpp"
#endif

namespace KBEngine
{

// 获得某个entity的函数地址
EntityMailbox::GetEntityFunc EntityMailbox::__getEntityFunc;

EntityMailbox::MailboxCallHookFunc*	EntityMailbox::__hookCallFuncPtr = NULL;

SCRIPT_METHOD_DECLARE_BEGIN(EntityMailbox)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(EntityMailbox)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(EntityMailbox)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(EntityMailbox, 0, 0, 0, 0, 0)		

//-------------------------------------------------------------------------------------
EntityMailbox::EntityMailbox(ScriptDefModule* scriptModule, 
							 const Mercury::Address* pAddr, 
							 COMPONENT_ID componentID, 
ENTITY_ID eid, ENTITY_MAILBOX_TYPE type):
EntityMailboxAbstract(getScriptType(),
					  pAddr, 
					  componentID, 
					  eid, scriptModule->getUType(), 
					  type),
scriptModule_(scriptModule)
{
}

//-------------------------------------------------------------------------------------
EntityMailbox::~EntityMailbox()
{
	char s[1024];
	c_str(s, 1024);

	DEBUG_MSG(boost::format("EntityMailbox::~EntityMailbox(): %1%.\n") % s);
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod* EntityMailbox::createRemoteMethod(MethodDescription* md)
{
	if(__hookCallFuncPtr != NULL)
	{
		return (*__hookCallFuncPtr)(md, this);
	}

	return new RemoteEntityMethod(md, this);
}

//-------------------------------------------------------------------------------------
PyObject* EntityMailbox::onScriptGetAttribute(PyObject* attr)
{
	wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(attr, NULL);
	char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
	PyMem_Free(PyUnicode_AsWideCharStringRet0);

	MethodDescription* md = NULL;

	switch(type_)
	{
	case MAILBOX_TYPE_CELL:
		md = scriptModule_->findCellMethodDescription(ccattr);
		break;
	case MAILBOX_TYPE_BASE:
		md = scriptModule_->findBaseMethodDescription(ccattr);
		break;
	case MAILBOX_TYPE_CLIENT:
		md = scriptModule_->findClientMethodDescription(ccattr);
		break;
	case MAILBOX_TYPE_CELL_VIA_BASE:
		md = scriptModule_->findCellMethodDescription(ccattr);
		break;
	case MAILBOX_TYPE_BASE_VIA_CELL:
		md = scriptModule_->findBaseMethodDescription(ccattr);
		break;
	case MAILBOX_TYPE_CLIENT_VIA_CELL:
		md = scriptModule_->findClientMethodDescription(ccattr);
		break;
	case MAILBOX_TYPE_CLIENT_VIA_BASE:
		md = scriptModule_->findClientMethodDescription(ccattr);
		break;
	default:
		break;
	};
	
	if(md != NULL)
	{
		free(ccattr);
		return createRemoteMethod(md);
	}

	// 首先要求名称不能为自己  比如：自身是一个cell， 不能使用cell.cell
	if(strcmp(ccattr, ENTITY_MAILBOX_TYPE_TO_NAME_TABLE[type_]) != 0)
	{
		int8 mbtype = -1;

		if(strcmp(ccattr, "cell") == 0)
		{
			if(type_ == MAILBOX_TYPE_BASE_VIA_CELL)
				mbtype = MAILBOX_TYPE_CELL;
			else
				mbtype = MAILBOX_TYPE_CELL_VIA_BASE;
		}
		else if(strcmp(ccattr, "base") == 0)
		{
			if(type_ == MAILBOX_TYPE_CELL_VIA_BASE)
				mbtype = MAILBOX_TYPE_BASE;
			else
				mbtype = MAILBOX_TYPE_BASE_VIA_CELL;
		}
		else if(strcmp(ccattr, "client") == 0)
		{
			if(type_ == MAILBOX_TYPE_BASE)
				mbtype = MAILBOX_TYPE_CLIENT_VIA_BASE;
			else if(type_ == MAILBOX_TYPE_CELL)
				mbtype = MAILBOX_TYPE_CLIENT_VIA_CELL;
		}
		
		if(mbtype != -1)
		{
			free(ccattr);

			if(g_componentType != CLIENT_TYPE && g_componentType != BOTS_TYPE)
			{
				return new EntityMailbox(scriptModule_, &addr_, componentID_, 
					id_, (ENTITY_MAILBOX_TYPE)mbtype);
			}
			else
			{
				Py_INCREF(this);
				return this;
			}
		}
	}
	
	free(ccattr);
	return ScriptObject::onScriptGetAttribute(attr);
}

//-------------------------------------------------------------------------------------
PyObject* EntityMailbox::tp_repr()
{
	char s[1024];
	c_str(s, 1024);
	return PyUnicode_FromString(s);
}

//-------------------------------------------------------------------------------------
void EntityMailbox::c_str(char* s, size_t size)
{
	const char * mailboxName =
		(type_ == MAILBOX_TYPE_CELL)				? "Cell" :
		(type_ == MAILBOX_TYPE_BASE)				? "Base" :
		(type_ == MAILBOX_TYPE_CLIENT)				? "Client" :
		(type_ == MAILBOX_TYPE_BASE_VIA_CELL)		? "BaseViaCell" :
		(type_ == MAILBOX_TYPE_CLIENT_VIA_CELL)		? "ClientViaCell" :
		(type_ == MAILBOX_TYPE_CELL_VIA_BASE)		? "CellViaBase" :
		(type_ == MAILBOX_TYPE_CLIENT_VIA_BASE)		? "ClientViaBase" : "???";
	
	Mercury::Channel* pChannel = getChannel();

	kbe_snprintf(s, size, "%s mailbox id:%d, utype:%u, component=%s[%"PRIu64"], addr: %s.", 
		mailboxName, id_,  utype_,
		COMPONENT_NAME_EX(ENTITY_MAILBOX_COMPONENT_TYPE_MAPPING[type_]), 
		componentID_, (pChannel) ? pChannel->addr().c_str() : "None");
}

//-------------------------------------------------------------------------------------
PyObject* EntityMailbox::tp_str()
{
	return tp_repr();
}

//-------------------------------------------------------------------------------------
PyObject* EntityMailbox::__unpickle__(PyObject* self, PyObject* args)
{
	ENTITY_ID eid = 0;
	COMPONENT_ID componentID = 0;
	COMPONENT_TYPE componentType;
	ENTITY_SCRIPT_UID utype = 0;
	int16 type = 0;

	Py_ssize_t size = PyTuple_Size(args);
	if(size != 4)
	{
		ERROR_MSG("EntityMailbox::__unpickle__: args is error! size != 4.\n");
		S_Return;
	}

	if(!PyArg_ParseTuple(args, "iKHh", &eid, &componentID, &utype, &type))
	{
		ERROR_MSG("EntityMailbox::__unpickle__: args is error!\n");
		S_Return;
	}

	ScriptDefModule* sm = EntityDef::findScriptModule(utype);
	if(sm == NULL)
	{
		ERROR_MSG(boost::format("EntityMailbox::__unpickle__: not found utype %%1!\n") % utype);
		S_Return;
	}

	// Mercury::Channel* pChannel = NULL;
	componentType = ENTITY_MAILBOX_COMPONENT_TYPE_MAPPING[(ENTITY_MAILBOX_TYPE)type];
	
#ifdef KBE_SERVER
	// 如果组件类别和组件ID和当前服务器相同， 则说明这个entity在本服务器上存在， 那么直接返回entity的实例
	if(componentType == g_componentType)
	{
		PyObject* entity = __getEntityFunc(componentID, eid);
		if(entity != NULL)
		{
			Py_INCREF(entity);
			return entity;
		}
	}
	else
	{
		Components::ComponentInfos* pcomponent = 
			Components::getSingleton().findComponent(componentType, componentID);

		if(pcomponent != NULL)
		{
			// pChannel = pcomponent->pChannel;
		}
		else
		{
			// 在服务器起来的时候可能会出现找不到， 因为组件先连接了dbmgr， dbmgr就开始初始化了
			// ERROR_MSG("EntityMailbox::__unpickle__: not found %s%ld!\n", 
			// COMPONENT_NAME_EX(componentType), componentID);
			// S_Return;
		}
	}
#else
	PyObject* entity = __getEntityFunc(componentID, eid);
	if(entity != NULL)
	{
		Py_INCREF(entity);
		return entity;
	}
#endif

	return new EntityMailbox(sm, NULL, componentID, eid, (ENTITY_MAILBOX_TYPE)type);
}

//-------------------------------------------------------------------------------------
void EntityMailbox::onInstallScript(PyObject* mod)
{
	static PyMethodDef __unpickle__Method = 
	{"Mailbox", (PyCFunction)&EntityMailbox::__unpickle__, METH_VARARGS, 0};

	PyObject* pyFunc = PyCFunction_New(&__unpickle__Method, NULL);
	script::Pickler::registerUnpickleFunc(pyFunc, "Mailbox");

	Py_DECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
}
