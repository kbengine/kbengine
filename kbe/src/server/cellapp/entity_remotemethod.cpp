#include "cellapp.hpp"
#include "witness.hpp"
#include "entity_remotemethod.hpp"
#include "entitydef/method.hpp"
#include "helper/profile.hpp"	
#include "network/bundle.hpp"

#include "client_lib/client_interface.hpp"

namespace KBEngine{	

SCRIPT_METHOD_DECLARE_BEGIN(EntityRemoteMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(EntityRemoteMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(EntityRemoteMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(EntityRemoteMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
EntityRemoteMethod::EntityRemoteMethod(MethodDescription* methodDescription, 
						EntityMailboxAbstract* mailbox):
RemoteEntityMethod(methodDescription, mailbox, getScriptType())
{
}

//-------------------------------------------------------------------------------------
EntityRemoteMethod::~EntityRemoteMethod()
{
}

//-------------------------------------------------------------------------------------
PyObject* EntityRemoteMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{	
	EntityRemoteMethod* rmethod = static_cast<EntityRemoteMethod*>(self);
	MethodDescription* methodDescription = rmethod->getDescription();
	EntityMailboxAbstract* mailbox = rmethod->getMailbox();

	if(!mailbox->isClient())
	{
		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	Entity* pEntity = Cellapp::getSingleton().findEntity(mailbox->getID());
	if(pEntity == NULL || pEntity->pWitness() == NULL)
	{
		//WARNING_MSG(boost::format("EntityRemoteMethod::callClientMethod: not found entity(%1%).\n") % 
		//	mailbox->getID());

		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	// 如果是调用客户端方法， 我们记录事件并且记录带宽
	if(methodDescription->checkArgs(args))
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		mailbox->newMail((*pBundle));

		MemoryStream* mstream = MemoryStream::ObjPool().createObject();
		methodDescription->addToStream(mstream, args);

		if(mstream->wpos() > 0)
			(*pBundle).append(mstream->data(), mstream->wpos());

		//mailbox->postMail((*pBundle));
		pEntity->pWitness()->sendToClient(ClientInterface::onRemoteMethodCall, pBundle);

		//Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		MemoryStream::ObjPool().reclaimObject(mstream);
	}
	
	S_Return;
}	

//-------------------------------------------------------------------------------------
}
