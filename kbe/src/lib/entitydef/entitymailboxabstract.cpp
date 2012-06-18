#include "entitymailboxabstract.hpp"
#include "pyscript/pickler.hpp"
#include "helper/debug_helper.hpp"
#include "network/packet.hpp"

namespace KBEngine{


SCRIPT_METHOD_DECLARE_BEGIN(EntityMailboxAbstract)
SCRIPT_METHOD_DECLARE("__reduce_ex__",				reduce_ex__,			METH_VARARGS,		0)
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(EntityMailboxAbstract)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(EntityMailboxAbstract)
SCRIPT_GET_DECLARE("id",							pyGetID,				0,					0)	
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(EntityMailboxAbstract, 0, 0, 0, 0, 0)		

//-------------------------------------------------------------------------------------
EntityMailboxAbstract::EntityMailboxAbstract(PyTypeObject* scriptType, Mercury::Channel* pChannel, 
COMPONENT_ID componentID, ENTITY_ID eid, uint16 utype, ENTITY_MAILBOX_TYPE type):
ScriptObject(scriptType, false),
pChannelPtr_(pChannel),
componentID_(componentID),
type_(type),
id_(eid),
utype_(utype)
{
}

//-------------------------------------------------------------------------------------
EntityMailboxAbstract::~EntityMailboxAbstract()
{
}

//-------------------------------------------------------------------------------------
/*
void EntityMailboxAbstract::post(SocketPacket* sp)
{
	if(sp != NULL && pChannelPtr_ != NULL && !pChannelPtr_->isClosed())
		pChannelPtr_->sendPacket(sp);
}

//-------------------------------------------------------------------------------------
SocketPacket* EntityMailboxAbstract::createStream(Opcodes code)
{
	SocketPacket* sp = new SocketPacket(code, 32);
	(*sp) << (ENTITY_ID)id_;
	return sp;
}

//-------------------------------------------------------------------------------------
SocketPacket* EntityMailboxAbstract::createMail(MAIL_TYPE mailType)
{
	SocketPacket* sp = new SocketPacket(OP_ENTITY_MAIL, 64);
	(*sp) << (ENTITY_ID)id_;
	(*sp) << (uint8)type_;
	(*sp) << (MAIL_TYPE)mailType;
	return sp;
}
*/
//-------------------------------------------------------------------------------------
PyObject* EntityMailboxAbstract::__py_reduce_ex__(PyObject* self, PyObject* protocol)
{
	EntityMailboxAbstract* emailbox = static_cast<EntityMailboxAbstract*>(self);
	PyObject* args = PyTuple_New(2);
	PyObject* unpickleMethod = script::Pickler::getUnpickleFunc("Mailbox");
	PyTuple_SET_ITEM(args, 0, unpickleMethod);
	PyObject* args1 = PyTuple_New(4);
	PyTuple_SET_ITEM(args1, 0, PyLong_FromUnsignedLong(emailbox->getID()));
	PyTuple_SET_ITEM(args1, 1, PyLong_FromUnsignedLong(emailbox->getComponentID()));
	PyTuple_SET_ITEM(args1, 2, PyLong_FromUnsignedLong(emailbox->getUType()));
	PyTuple_SET_ITEM(args1, 3, PyLong_FromUnsignedLong(emailbox->getType()));
	PyTuple_SET_ITEM(args, 1, args1);

	if(unpickleMethod == NULL){
		Py_DECREF(args);
		return NULL;
	}
	return args;
}

//-------------------------------------------------------------------------------------
PyObject* EntityMailboxAbstract::pyGetID()
{ 
	return PyLong_FromLong(getID()); 
}

//-------------------------------------------------------------------------------------

}
