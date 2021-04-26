// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
#include "entitydef/property.h"
#include "entitydef/scriptdef_module.h"
#include "clients_remote_entity_method.h"

#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(AllClientsComponent)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(AllClientsComponent)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(AllClientsComponent)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(AllClientsComponent, 0, 0, 0, 0, 0)

//-------------------------------------------------------------------------------------
AllClientsComponent::AllClientsComponent(PropertyDescription* pComponentPropertyDescription, AllClients* pAllClients):
	ScriptObject(getScriptType(), false),
	pAllClients_(pAllClients),
	pComponentPropertyDescription_(pComponentPropertyDescription)
{
	Py_INCREF(pAllClients_);
}

//-------------------------------------------------------------------------------------
AllClientsComponent::~AllClientsComponent()
{
	Py_DECREF(pAllClients_);
}

//-------------------------------------------------------------------------------------
ScriptDefModule* AllClientsComponent::pComponentScriptDefModule()
{
	EntityComponentType* pEntityComponentType = static_cast<EntityComponentType*>(pComponentPropertyDescription_->getDataType());
	return pEntityComponentType->pScriptDefModule();
}

//-------------------------------------------------------------------------------------
PyObject* AllClientsComponent::onScriptGetAttribute(PyObject* attr)
{
	ENTITY_ID entityID = pAllClients_->id();

	Entity* pEntity = Cellapp::getSingleton().findEntity(entityID);
	if (pEntity == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "AllClientsComponent::onScriptGetAttribute: not found entity(%d).",
			entityID);

		return 0;
	}

	if (!pEntity->isReal())
	{
		PyErr_Format(PyExc_AssertionError, "AllClientsComponent::onScriptGetAttribute: %s not is real entity(%d).",
			pEntity->scriptName(), pEntity->id());

		return 0;
	}

	const char* ccattr = PyUnicode_AsUTF8AndSize(attr, NULL);

	ScriptDefModule* pScriptDefModule = pComponentScriptDefModule();
	MethodDescription* pMethodDescription = pScriptDefModule->findClientMethodDescription(ccattr);

	if (pMethodDescription != NULL)
	{
		return new ClientsRemoteEntityMethod(pComponentPropertyDescription_, pScriptDefModule, pMethodDescription, pAllClients_->isOtherClients(), entityID);
	}
	
	return ScriptObject::onScriptGetAttribute(attr);
}

//-------------------------------------------------------------------------------------
PyObject* AllClientsComponent::tp_repr()
{
	char s[1024];
	c_str(s, 1024);
	return PyUnicode_FromString(s);
}

//-------------------------------------------------------------------------------------
void AllClientsComponent::c_str(char* s, size_t size)
{
	kbe_snprintf(s, size, "component_clients id:%d.", pAllClients_->id());
}

//-------------------------------------------------------------------------------------
PyObject* AllClientsComponent::tp_str()
{
	return tp_repr();
}

//-------------------------------------------------------------------------------------
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

		return 0;
	}

	if(!pEntity->isReal())
	{
		PyErr_Format(PyExc_AssertionError, "AllClients::onScriptGetAttribute: %s not is real entity(%d).", 
			pEntity->scriptName(), pEntity->id());

		return 0;
	}
	
	const char* ccattr = PyUnicode_AsUTF8AndSize(attr, NULL);

	MethodDescription* pMethodDescription = const_cast<ScriptDefModule*>(pScriptModule_)->findClientMethodDescription(ccattr);
	
	if(pMethodDescription != NULL)
	{
		return new ClientsRemoteEntityMethod(NULL, pScriptModule_, pMethodDescription, otherClients_, id_);
	}
	else
	{
		// 是否是组件方法调用
		PropertyDescription* pComponentPropertyDescription = const_cast<ScriptDefModule*>(pScriptModule_)->findComponentPropertyDescription(ccattr);
		if (pComponentPropertyDescription)
		{
			return new AllClientsComponent(pComponentPropertyDescription, this);
		}
	}

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

