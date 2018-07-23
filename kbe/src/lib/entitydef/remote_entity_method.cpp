// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "method.h"
#include "remote_entity_method.h"
#include "network/bundle.h"
#include "helper/debug_helper.h"

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

		if (!pChannel)
			pSendBundle = Network::Bundle::createPoolObject();
		else
			pSendBundle = pChannel->createSendBundle();

		entityCall->newCall((*pSendBundle));

		MemoryStream mstream;
		methodDescription->addToStream(&mstream, args);

		if(mstream.wpos() > 0)
			(*pSendBundle).append(mstream.data(), mstream.wpos());

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
