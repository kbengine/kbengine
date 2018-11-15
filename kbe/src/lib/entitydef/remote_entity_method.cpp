// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "method.h"
#include "remote_entity_method.h"
#include "network/bundle.h"
#include "helper/debug_helper.h"
#include "entitydef/scriptdef_module.h"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(RemoteEntityMethod)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(RemoteEntityMethod, tp_call, 0, 0, 0, 0)	

//-------------------------------------------------------------------------------------
RemoteEntityMethod::RemoteEntityMethod(MethodDescription* methodDescription, 
	EntityCallAbstract* entityCall, PyTypeObject* pyType):
script::ScriptObject((pyType == NULL ? getScriptType() : pyType), false),
methodDescription_(methodDescription),
pEntityCall_(entityCall)
{
	Py_INCREF(pEntityCall_);
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod::~RemoteEntityMethod()
{
	Py_DECREF(pEntityCall_);
}

//-------------------------------------------------------------------------------------
const char* RemoteEntityMethod::getName(void) const
{ 
	return methodDescription_->getName(); 
};

//-------------------------------------------------------------------------------------
PyObject* RemoteEntityMethod::tp_call(PyObject* self, PyObject* args, 
	PyObject* kwds)	
{	
	RemoteEntityMethod* rmethod = static_cast<RemoteEntityMethod*>(self);
	MethodDescription* methodDescription = rmethod->getDescription();
	EntityCallAbstract* entityCall = rmethod->getEntityCall();
	// DEBUG_MSG(fmt::format("RemoteEntityMethod::tp_call:{}.\n"), methodDescription->getName()));

	if(methodDescription->checkArgs(args))
	{
		Network::Channel* pChannel = entityCall->getChannel();
		Network::Bundle* pSendBundle = NULL;

		MemoryStream* mstream = MemoryStream::createPoolObject(OBJECTPOOL_POINT);

		try
		{
			methodDescription->addToStream(mstream, args);
		}
		catch (MemoryStreamWriteOverflow & err)
		{
			ERROR_MSG(fmt::format("RemoteEntityMethod::tp_call(): {}.{}() {}, error={}\n", 
				entityCall->pScriptDefModule()->getName(), rmethod->getName(), entityCall->id(), err.what()));

			MemoryStream::reclaimPoolObject(mstream);
			S_Return;
		}

		if (!pChannel)
			pSendBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		else
			pSendBundle = pChannel->createSendBundle();

		entityCall->newCall((*pSendBundle));

		if(mstream->wpos() > 0)
			(*pSendBundle).append(mstream->data(), mstream->wpos());

		MemoryStream::reclaimPoolObject(mstream);
		entityCall->sendCall(pSendBundle);
	}
	else
	{
        ERROR_MSG(fmt::format("RemoteEntityMethod::tp_call:{} checkArgs error!\n",
                methodDescription->getName()));
	}

	S_Return;
}		
	
//-------------------------------------------------------------------------------------

}
