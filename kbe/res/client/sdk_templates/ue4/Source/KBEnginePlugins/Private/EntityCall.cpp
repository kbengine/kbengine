
#include "EntityCall.h"
#include "Bundle.h"
#include "KBEngine.h"
#include "Message.h"
#include "KBDebug.h"

EntityCall::EntityCall():
	id(0),
	className(),
	type(ENTITYCALL_TYPE_CELL),
	pBundle(NULL)
{
}

EntityCall::~EntityCall()
{
}

Bundle* EntityCall::newCall()
{
	if (!pBundle)
		pBundle = Bundle::createObject();

	if (type == ENTITYCALL_TYPE_CELL)
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_onRemoteCallCellMethodFromClient"]));
	else
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Entity_onRemoteMethodCall"]));

	(*pBundle) << id;
	return pBundle;
}

void EntityCall::sendCall(Bundle* inBundle)
{
	if (!inBundle)
		inBundle = pBundle;

	inBundle->send(KBEngineApp::getSingleton().pNetworkInterface());

	if (inBundle == pBundle)
		pBundle = NULL;
}