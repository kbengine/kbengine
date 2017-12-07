
#include "Mailbox.h"
#include "Bundle.h"
#include "KBEngine.h"
#include "Message.h"
#include "KBDebug.h"

Mailbox::Mailbox():
	id(0),
	className(),
	type(MAILBOX_TYPE_CELL),
	pBundle(NULL)
{
}

Mailbox::~Mailbox()
{
}

Bundle* Mailbox::newMail()
{
	if (!pBundle)
		pBundle = Bundle::createObject();

	if (type == MAILBOX_TYPE_CELL)
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Baseapp_onRemoteCallCellMethodFromClient"]));
	else
		pBundle->newMessage(Messages::getSingleton().messages[TEXT("Base_onRemoteMethodCall"]));

	(*pBundle) << id;
	return pBundle;
}

void Mailbox::postMail(Bundle* inBundle)
{
	if (!inBundle)
		inBundle = pBundle;

	inBundle->send(KBEngineApp::getSingleton().pNetworkInterface());

	if (inBundle == pBundle)
		pBundle = NULL;
}