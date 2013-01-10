#include "baseapp.hpp"
#include "base_remotemethod.hpp"
#include "entitydef/method.hpp"
#include "helper/profile.hpp"	
#include "network/bundle.hpp"
#include "server/eventhistory_stats.hpp"

#include "client_lib/client_interface.hpp"

namespace KBEngine{	

SCRIPT_METHOD_DECLARE_BEGIN(BaseRemoteMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(BaseRemoteMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(BaseRemoteMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(BaseRemoteMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
BaseRemoteMethod::BaseRemoteMethod(MethodDescription* methodDescription, 
						EntityMailboxAbstract* mailbox):
RemoteEntityMethod(methodDescription, mailbox, getScriptType())
{
}

//-------------------------------------------------------------------------------------
BaseRemoteMethod::~BaseRemoteMethod()
{
}

//-------------------------------------------------------------------------------------
PyObject* BaseRemoteMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{	
	BaseRemoteMethod* rmethod = static_cast<BaseRemoteMethod*>(self);
	MethodDescription* methodDescription = rmethod->getDescription();
	EntityMailboxAbstract* mailbox = rmethod->getMailbox();

	if(!mailbox->isClient())
	{
		return RemoteEntityMethod::tp_call(self, args, kwds);
	}

	Base* pEntity = Baseapp::getSingleton().findEntity(mailbox->getID());
	if(pEntity == NULL)
	{
		//WARNING_MSG(boost::format("BaseRemoteMethod::callClientMethod: not found entity(%1%).\n") % 
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

		// 记录这个事件产生的数据量大小
		g_privateClientEventHistoryStats.trackEvent(pEntity->getScriptName(), 
			methodDescription->getName(), 
			pBundle->currMsgLength(), 
			"::");

		mailbox->postMail((*pBundle));

		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		MemoryStream::ObjPool().reclaimObject(mstream);
	}
	
	S_Return;
}	

//-------------------------------------------------------------------------------------
}
