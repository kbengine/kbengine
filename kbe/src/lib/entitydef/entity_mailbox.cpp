#include "entity_mailbox.hpp"
#include "helper/debug_helper.hpp"
#include "network/channel.hpp"	
#include "server/engine_componentmgr.hpp"

namespace KBEngine
{
// 获得某个entity的函数地址
GetEntityFunc EntityMailbox::__getEntityFunc;

SCRIPT_METHOD_DECLARE_BEGIN(EntityMailbox)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(EntityMailbox)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(EntityMailbox)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(EntityMailbox, 0, 0, 0, 0, 0)		

//-------------------------------------------------------------------------------------
EntityMailbox::EntityMailbox(Mercury::Channel* pChannel, ScriptModule* scriptModule, COMPONENT_ID componentID, 
ENTITY_ID& eid, ENTITY_MAILBOX_TYPE type):
EntityMailboxAbstract(getScriptType(), pChannel, componentID, eid, scriptModule->getUType(), type),
scriptModule_(scriptModule)
{
}

//-------------------------------------------------------------------------------------
EntityMailbox::~EntityMailbox()
{
}

//-------------------------------------------------------------------------------------
PyObject* EntityMailbox::onScriptGetAttribute(PyObject* attr)
{
	char* ccattr = wchar2char(PyUnicode_AsWideCharString(attr, NULL));

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
		delete ccattr;
		return new RemoteEntityMethod(md, this);
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
			delete ccattr;
			return new EntityMailbox(pChannelPtr_, scriptModule_, componentID_, id_, (ENTITY_MAILBOX_TYPE)mbtype);
		}
	}
	
	delete ccattr;
	return ScriptObject::onScriptGetAttribute(attr);
}

//-------------------------------------------------------------------------------------
PyObject* EntityMailbox::tp_repr()
{
	char s[1024];
	
	const char * mailboxName =
		(type_ == MAILBOX_TYPE_CELL)				? "Cell" :
		(type_ == MAILBOX_TYPE_BASE)				? "Base" :
		(type_ == MAILBOX_TYPE_CLIENT)				? "Client" :
		(type_ == MAILBOX_TYPE_BASE_VIA_CELL)		? "BaseViaCell" :
		(type_ == MAILBOX_TYPE_CLIENT_VIA_CELL)		? "ClientViaCell" :
		(type_ == MAILBOX_TYPE_CELL_VIA_BASE)		? "CellViaBase" :
		(type_ == MAILBOX_TYPE_CLIENT_VIA_BASE)		? "ClientViaBase" : "???";
	
	sprintf(s, "%s mailbox id:%d, component=%s%d, addr: %s.", mailboxName, id_, 
		COMPONENT_NAME[ENTITY_MAILBOX_COMPONENT_TYPE_MAPPING[type_]], 
			componentID_, pChannelPtr_->addr().c_str());

	return PyUnicode_FromString(s);
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
	uint16 utype = 0, type = 0;

	Py_ssize_t size = PyTuple_Size(args);
	if(size != 4)
	{
		ERROR_MSG("EntityMailbox::__unpickle__: args is error! size != 4");
		S_Return;
	}

	if(!PyArg_ParseTuple(args, "llHH", &eid, &componentID, &utype, &type))
	{
		ERROR_MSG("EntityMailbox::__unpickle__: args is error!");
		S_Return;
	}

	ScriptModule* sm = EntityDef::findScriptModule(utype);
	if(sm == NULL)
	{
		ERROR_MSG("EntityMailbox::__unpickle__: not found utype %ld!", utype);
		S_Return;
	}

	Mercury::Channel* pChannel = NULL;
	componentType = ENTITY_MAILBOX_COMPONENT_TYPE_MAPPING[(ENTITY_MAILBOX_TYPE)type];
	
#ifdef KBE_SERVER
	// 如果组件类别和组件ID和当前服务器相同， 则说明这个entity在本服务器上存在， 那么直接返回entity的实例
	if(componentType == g_componentType)
	{
		PyObject* entity = __getEntityFunc(componentID, eid);
		if(entity != NULL)
			return entity;
	}
	else
	{
		EngineComponentMgr::COMPONENT_MAP& components = EngineComponentMgr::getSingleton().getComponents(componentType);
		EngineComponentMgr::COMPONENT_MAP::iterator iter = components.find(componentID);
		if(iter != components.end())
		{
			pChannel = iter->second.pChannel;
		}
		else
		{
			ERROR_MSG("EntityMailbox::__unpickle__: not found %s%ld!", COMPONENT_NAME[componentType], componentID);
			S_Return;
		}
	}
#else
	PyObject* entity = __getEntityFunc(componentID, eid);
	if(entity != NULL)
		return entity;
#endif

	return new EntityMailbox(pChannel, sm, componentID, eid, (ENTITY_MAILBOX_TYPE)type);
}

//-------------------------------------------------------------------------------------
void EntityMailbox::onInstallScript(PyObject* mod)
{
	static PyMethodDef __unpickle__Method = {"Mailbox", (PyCFunction)&EntityMailbox::__unpickle__, METH_VARARGS, 0};
	PyObject* pyFunc = PyCFunction_New(&__unpickle__Method, NULL);
	script::Pickler::registerUnpickleFunc(pyFunc, "Mailbox");
	Py_DECREF(pyFunc);
}

//-------------------------------------------------------------------------------------
}
