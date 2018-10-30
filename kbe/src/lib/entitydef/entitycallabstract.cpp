// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "entitycallabstract.h"
#include "pyscript/pickler.h"
#include "helper/debug_helper.h"
#include "network/packet.h"
#include "network/bundle.h"
#include "network/network_interface.h"
#include "server/components.h"
#include "entitydef/entitydef.h"
#include "client_lib/client_interface.h"

#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"

#ifndef CODE_INLINE
#include "entitycallabstract.inl"
#endif

namespace KBEngine{
EntityCallAbstract::EntityCallCallHookFunc*	EntityCallAbstract::__hookCallFuncPtr = NULL;
EntityCallAbstract::FindChannelFunc EntityCallAbstract::__findChannelFunc;

SCRIPT_METHOD_DECLARE_BEGIN(EntityCallAbstract)
SCRIPT_METHOD_DECLARE("__reduce_ex__",				reduce_ex__,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(EntityCallAbstract)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(EntityCallAbstract)
SCRIPT_GET_DECLARE("id",							pyGetID,				0,					0)	
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(EntityCallAbstract, 0, 0, 0, 0, 0)		

//-------------------------------------------------------------------------------------
EntityCallAbstract::EntityCallAbstract(PyTypeObject* scriptType, 
											const Network::Address* pAddr, 
											COMPONENT_ID componentID, 
											ENTITY_ID eid, 
											uint16 utype, 
											ENTITYCALL_TYPE type):
ScriptObject(scriptType, false),
componentID_(componentID),
addr_((pAddr == NULL) ? Network::Address::NONE : *pAddr),
type_(type),
id_(eid),
utype_(utype)
{
}

//-------------------------------------------------------------------------------------
EntityCallAbstract::~EntityCallAbstract()
{
}

//-------------------------------------------------------------------------------------
void EntityCallAbstract::newCall(Network::Bundle& bundle)
{
	newCall_(bundle);
}

//-------------------------------------------------------------------------------------
void EntityCallAbstract::newCall_(Network::Bundle& bundle)
{
	// 如果是server端的entityCall
	if(g_componentType != CLIENT_TYPE && g_componentType != BOTS_TYPE)
	{
		// 如果ID为0，则这是一个客户端组件，否则为服务端。
		if(componentID_ == 0)
		{
			bundle.newMessage(ClientInterface::onRemoteMethodCall);
		}
		else
		{
			Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentID_);

			if(cinfos != NULL)
			{
				// 找到对应的组件投递过去， 如果这个entityCall还需要中转比如 e.base.cell ， 则由baseapp转往cellapp
				if(cinfos->componentType == BASEAPP_TYPE)
				{
					bundle.newMessage(BaseappInterface::onEntityCall);
				}
				else
				{
					bundle.newMessage(CellappInterface::onEntityCall);
				}
			}
			else
			{
				ERROR_MSG(fmt::format("EntityCallAbstract::newCall_: not found component({}), entityID({})!\n",
					componentID_, id_));
			}
		}

		bundle << id_;
		
		// 如果是发往客户端的包则无需附加这样一个类型
		if(componentID_ > 0)
			bundle << type_;
	}
	else
	{
		// 如果是客户端上的entityCall调用服务端方法只存在调用cell或者base
		switch(type_)
		{
		case ENTITYCALL_TYPE_BASE:
			bundle.newMessage(BaseappInterface::onRemoteMethodCall);
			break;
		case ENTITYCALL_TYPE_CELL:
			bundle.newMessage(BaseappInterface::onRemoteCallCellMethodFromClient);
			break;
		default:
			KBE_ASSERT(false && "no support!\n");
			break;
		};

		bundle << id_;
	}
}

//-------------------------------------------------------------------------------------
bool EntityCallAbstract::sendCall(Network::Bundle* pBundle)
{
	KBE_ASSERT(Components::getSingleton().pNetworkInterface() != NULL);
	Network::Channel* pChannel = getChannel();

	if(pChannel && !pChannel->isDestroyed())
	{
		pChannel->send(pBundle);
		return true;
	}
	else
	{
		ERROR_MSG(fmt::format("EntityCallAbstract::sendCall: invalid channel({}), entityID({})!\n",
			addr_.c_str(), id_));
	}

	Network::Bundle::reclaimPoolObject(pBundle);
	return false;
}

//-------------------------------------------------------------------------------------
PyObject* EntityCallAbstract::__py_reduce_ex__(PyObject* self, PyObject* protocol)
{
	EntityCallAbstract* eentitycall = static_cast<EntityCallAbstract*>(self);
	
	PyObject* args = PyTuple_New(2);
	PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("EntityCall");
	PyTuple_SET_ITEM(args, 0, unpickleMethod);
	
	PyObject* args1 = PyTuple_New(4);
	PyTuple_SET_ITEM(args1, 0, PyLong_FromLong(eentitycall->id()));
	PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLongLong(eentitycall->componentID()));
	PyTuple_SET_ITEM(args1, 2, PyLong_FromUnsignedLong(eentitycall->utype()));

	int16 mbType = static_cast<int16>(eentitycall->type());
	
	PyTuple_SET_ITEM(args1, 3, PyLong_FromLong(mbType));
	PyTuple_SET_ITEM(args, 1, args1);

	if(unpickleMethod == NULL){
		Py_DECREF(args);
		return NULL;
	}
	return args;
}

//-------------------------------------------------------------------------------------
PyObject* EntityCallAbstract::pyGetID()
{ 
	return PyLong_FromLong(id()); 
}

//-------------------------------------------------------------------------------------
Network::Channel* EntityCallAbstract::getChannel(void)
{
	if (__findChannelFunc == NULL)
		return NULL;

	return __findChannelFunc(*this);
}

//-------------------------------------------------------------------------------------
ScriptDefModule* EntityCallAbstract::pScriptDefModule()
{
	return EntityDef::findScriptModule(utype_);
}

//-------------------------------------------------------------------------------------

}
