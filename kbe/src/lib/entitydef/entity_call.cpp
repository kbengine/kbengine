// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "entity_call.h"
#include "entity_component_call.h"
#include "scriptdef_module.h"
#include "helper/debug_helper.h"
#include "network/channel.h"	
#include "pyscript/pickler.h"
#include "pyscript/py_gc.h"
#include "entitydef/method.h"
#include "remote_entity_method.h"
#include "entitydef/entitydef.h"

namespace KBEngine
{
EntityCall::ENTITYCALLS EntityCall::entityCalls;

SCRIPT_METHOD_DECLARE_BEGIN(EntityCall)
SCRIPT_METHOD_DECLARE("getComponent",		pyGetComponent,		METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(EntityCall)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(EntityCall)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(EntityCall, 0, 0, 0, 0, 0)		

//-------------------------------------------------------------------------------------
EntityCall::EntityCall(ScriptDefModule* pScriptModule, 
							 const Network::Address* pAddr, 
							 COMPONENT_ID componentID, 
ENTITY_ID eid, ENTITYCALL_TYPE type):
EntityCallAbstract(getScriptType(),
					  pAddr, 
					  componentID, 
					  eid, pScriptModule->getUType(),
					  type),
					  scriptModuleName_(pScriptModule->getName()),
					  pScriptModule_(pScriptModule),
atIdx_(ENTITYCALLS::size_type(-1))
{
	atIdx_ = EntityCall::entityCalls.size();
	EntityCall::entityCalls.push_back(this);

	script::PyGC::incTracing("EntityCall");
}

//-------------------------------------------------------------------------------------
EntityCall::~EntityCall()
{
	//char s[1024];
	//c_str(s, 1024);
	//DEBUG_MSG(fmt::format("EntityCall::~EntityCall(): {}.\n", s));

	KBE_ASSERT(atIdx_ < EntityCall::entityCalls.size());
	KBE_ASSERT(EntityCall::entityCalls[ atIdx_ ] == this);

	// 如果有2个或以上的EntityCall则将最后一个EntityCall移至删除的这个EntityCall所在位置
	EntityCall* pBack = EntityCall::entityCalls.back();
	pBack->_setATIdx(atIdx_);
	EntityCall::entityCalls[atIdx_] = pBack;
	atIdx_ = ENTITYCALLS::size_type(-1);
	EntityCall::entityCalls.pop_back();

	script::PyGC::decTracing("EntityCall");
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod* EntityCall::createRemoteMethod(MethodDescription* pMethodDescription)
{
	if(__hookCallFuncPtr != NULL)
	{
		return (*__hookCallFuncPtr)(pMethodDescription, this);
	}

	return new RemoteEntityMethod(pMethodDescription, this);
}

//-------------------------------------------------------------------------------------
PyObject* EntityCall::onScriptGetAttribute(PyObject* attr)
{
	const char* ccattr = PyUnicode_AsUTF8AndSize(attr, NULL);

	MethodDescription* pMethodDescription = NULL;

	switch(type_)
	{
	case ENTITYCALL_TYPE_CELL:
		pMethodDescription = pScriptModule_->findCellMethodDescription(ccattr);
		break;
	case ENTITYCALL_TYPE_BASE:
		pMethodDescription = pScriptModule_->findBaseMethodDescription(ccattr);
		break;
	case ENTITYCALL_TYPE_CLIENT:
		pMethodDescription = pScriptModule_->findClientMethodDescription(ccattr);
		break;
	case ENTITYCALL_TYPE_CELL_VIA_BASE:
		pMethodDescription = pScriptModule_->findCellMethodDescription(ccattr);
		break;
	case ENTITYCALL_TYPE_BASE_VIA_CELL:
		pMethodDescription = pScriptModule_->findBaseMethodDescription(ccattr);
		break;
	case ENTITYCALL_TYPE_CLIENT_VIA_CELL:
		pMethodDescription = pScriptModule_->findClientMethodDescription(ccattr);
		break;
	case ENTITYCALL_TYPE_CLIENT_VIA_BASE:
		pMethodDescription = pScriptModule_->findClientMethodDescription(ccattr);
		break;
	default:
		break;
	};
	
	if(pMethodDescription != NULL)
	{
		if(g_componentType == CLIENT_TYPE || g_componentType == BOTS_TYPE)
		{
			if(!pMethodDescription->isExposed())
				return ScriptObject::onScriptGetAttribute(attr);
		}

		return createRemoteMethod(pMethodDescription);
	}
	else
	{
		// 是否是组件方法调用
		PropertyDescription* pComponentPropertyDescription = pScriptModule_->findComponentPropertyDescription(ccattr);
		if (pComponentPropertyDescription)
		{
			return new EntityComponentCall(this, pComponentPropertyDescription);
		}
	}

	// 首先要求名称不能为自己  比如：自身是一个cell， 不能使用cell.cell
	if(strcmp(ccattr, ENTITYCALL_TYPE_TO_NAME_TABLE[type_]) != 0)
	{
		int8 mbtype = -1;

		if(strcmp(ccattr, "cell") == 0)
		{
			if(type_ == ENTITYCALL_TYPE_BASE_VIA_CELL)
				mbtype = ENTITYCALL_TYPE_CELL;
			else
				mbtype = ENTITYCALL_TYPE_CELL_VIA_BASE;
		}
		else if(strcmp(ccattr, "base") == 0)
		{
			if(type_ == ENTITYCALL_TYPE_CELL_VIA_BASE)
				mbtype = ENTITYCALL_TYPE_BASE;
			else
				mbtype = ENTITYCALL_TYPE_BASE_VIA_CELL;
		}
		else if(strcmp(ccattr, "client") == 0)
		{
			if(type_ == ENTITYCALL_TYPE_BASE)
				mbtype = ENTITYCALL_TYPE_CLIENT_VIA_BASE;
			else if(type_ == ENTITYCALL_TYPE_CELL)
				mbtype = ENTITYCALL_TYPE_CLIENT_VIA_CELL;
		}
		
		if(mbtype != -1)
		{
			if(g_componentType != CLIENT_TYPE && g_componentType != BOTS_TYPE)
			{
				return new EntityCall(pScriptModule_, &addr_, componentID_, 
					id_, (ENTITYCALL_TYPE)mbtype);
			}
			else
			{
				Py_INCREF(this);
				return this;
			}
		}
	}

	return ScriptObject::onScriptGetAttribute(attr);
}

//-------------------------------------------------------------------------------------
PyObject* EntityCall::tp_repr()
{
	char s[1024];
	c_str(s, 1024);
	return PyUnicode_FromString(s);
}

//-------------------------------------------------------------------------------------
void EntityCall::c_str(char* s, size_t size)
{
	const char * entitycallName =
		(type_ == ENTITYCALL_TYPE_CELL)					? "Cell" :
		(type_ == ENTITYCALL_TYPE_BASE)					? "Base" :
		(type_ == ENTITYCALL_TYPE_CLIENT)				? "Client" :
		(type_ == ENTITYCALL_TYPE_BASE_VIA_CELL)		? "BaseViaCell" :
		(type_ == ENTITYCALL_TYPE_CLIENT_VIA_CELL)		? "ClientViaCell" :
		(type_ == ENTITYCALL_TYPE_CELL_VIA_BASE)		? "CellViaBase" :
		(type_ == ENTITYCALL_TYPE_CLIENT_VIA_BASE)		? "ClientViaBase" : "???";
	
	Network::Channel* pChannel = getChannel();

	kbe_snprintf(s, size, "%s id:%d, utype:%u, component=%s[%" PRIu64 "], addr: %s.", 
		entitycallName, id_,  utype_,
		COMPONENT_NAME_EX(ENTITYCALL_COMPONENT_TYPE_MAPPING[type_]), 
		componentID_, (pChannel && pChannel->pEndPoint()) ? pChannel->addr().c_str() : "None");
}

//-------------------------------------------------------------------------------------
PyObject* EntityCall::tp_str()
{
	return tp_repr();
}

//-------------------------------------------------------------------------------------
PyObject* EntityCall::__unpickle__(PyObject* self, PyObject* args)
{
	ENTITY_ID eid = 0;
	COMPONENT_ID componentID = 0;
	ENTITY_SCRIPT_UID utype = 0;
	int16 type = 0;

	Py_ssize_t size = PyTuple_Size(args);
	if(size != 4)
	{
		ERROR_MSG("EntityCall::__unpickle__: args error! size != 4.\n");
		S_Return;
	}

	if(!PyArg_ParseTuple(args, "iKHh", &eid, &componentID, &utype, &type))
	{
		ERROR_MSG("EntityCall::__unpickle__: args error!\n");
		S_Return;
	}

	ScriptDefModule* sm = EntityDef::findScriptModule(utype);
	if(sm == NULL)
	{
		ERROR_MSG(fmt::format("EntityCall::__unpickle__: not found utype {}!\n", utype));
		S_Return;
	}

	// COMPONENT_TYPE componentType = ENTITYCALL_COMPONENT_TYPE_MAPPING[(ENTITYCALL_TYPE)type];
	
	PyObject* entity = EntityDef::tryGetEntity(componentID, eid);
	if(entity != NULL)
	{
		Py_INCREF(entity);
		return entity;
	}

	return new EntityCall(sm, NULL, componentID, eid, (ENTITYCALL_TYPE)type);
}

//-------------------------------------------------------------------------------------
void EntityCall::onInstallScript(PyObject* mod)
{
	static PyMethodDef __unpickle__Method = 
	{"EntityCall", (PyCFunction)&EntityCall::__unpickle__, METH_VARARGS, 0};

	PyObject* pyFunc = PyCFunction_New(&__unpickle__Method, NULL);
	script::Pickler::registerUnpickleFunc(pyFunc, "EntityCall");

	Py_DECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
void EntityCall::reload()
{ 
	pScriptModule_ = EntityDef::findScriptModule(scriptModuleName_.c_str());
}

//-------------------------------------------------------------------------------------
void EntityCall::newCall(Network::Bundle& bundle)
{
	newCall_(bundle);

	if (isClient() && pScriptModule_->usePropertyDescrAlias())
		bundle << (uint8)0;
	else
		bundle << (ENTITY_PROPERTY_UID)0;
}

//-------------------------------------------------------------------------------------
PyObject* EntityCall::pyGetComponent(const std::string& componentName, bool all)
{
	std::vector<EntityComponentCall*> founds =
		EntityComponentCall::getComponents(componentName, this, pScriptModule_);

	if (!all)
	{
		if (founds.size() > 0)
			return founds[0];

		Py_RETURN_NONE;
	}
	else
	{
		PyObject* pyObj = PyTuple_New(founds.size());

		for (int i = 0; i < (int)founds.size(); ++i)
		{
			PyTuple_SetItem(pyObj, i, founds[i]);
		}

		return pyObj;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
PyObject* EntityCall::__py_pyGetComponent(PyObject* self, PyObject* args)
{
	uint16 currargsSize = (uint16)PyTuple_Size(args);
	EntityCall* pobj = static_cast<EntityCall*>(self);

	if (currargsSize == 0 || currargsSize > 2)
	{
		PyErr_Format(PyExc_AssertionError,
			"EntityCall::getComponent: args require 1-2 args, gived %d!\n",
			currargsSize);

		PyErr_PrintEx(0);
		Py_RETURN_NONE;
	}

	char* componentName = NULL;
	if (currargsSize == 1)
	{
		if (!PyArg_ParseTuple(args, "s", &componentName))
		{
			PyErr_Format(PyExc_AssertionError, "EntityCall::getComponent:: args error!");
			PyErr_PrintEx(0);
			Py_RETURN_NONE;
		}

		if (!componentName)
		{
			PyErr_Format(PyExc_AssertionError, "EntityCall::getComponent:: componentName error!");
			PyErr_PrintEx(0);
			Py_RETURN_NONE;
		}

		return pobj->pyGetComponent(componentName, false);
	}
	else if (currargsSize == 2)
	{
		PyObject* pyobj = NULL;
		if (!PyArg_ParseTuple(args, "sO", &componentName, &pyobj))
		{
			PyErr_Format(PyExc_AssertionError, "EntityCall::getComponent:: args error!");
			PyErr_PrintEx(0);
			Py_RETURN_NONE;
		}

		if (!componentName)
		{
			PyErr_Format(PyExc_AssertionError, "EntityCall::getComponent:: componentName error!");
			PyErr_PrintEx(0);
			Py_RETURN_NONE;
		}

		return pobj->pyGetComponent(componentName, (pyobj == Py_True));
	}

	Py_RETURN_NONE;
}
//-------------------------------------------------------------------------------------
}
