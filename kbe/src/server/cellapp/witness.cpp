#include "witness.hpp"
#include "entity.hpp"	
#include "network/channel.hpp"	
#include "network/bundle.hpp"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Witness::Witness():
pEntity_(NULL),
aoiRadius_(0.0f),
aoiHysteresisArea_(0.0f)
{
}

//-------------------------------------------------------------------------------------
Witness::~Witness()
{
	pEntity_ = NULL;
}

//-------------------------------------------------------------------------------------
static ObjectPool<Witness> _g_objPool;
ObjectPool<Witness>& Witness::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
Witness::SmartPoolObjectPtr Witness::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<Witness>(ObjPool().createObject(), _g_objPool));
}

//-------------------------------------------------------------------------------------
void Witness::onReclaimObject()
{
}

//-------------------------------------------------------------------------------------
Mercury::Bundle & Witness::bundle()
{
	KBE_ASSERT(pEntity_->clientMailbox_);
	Mercury::Channel* pChannel = pEntity_->clientMailbox_->getChannel();
	KBE_ASSERT(pChannel);

	return pChannel->bundle();
}

//-------------------------------------------------------------------------------------
void Witness::update()
{
}

//-------------------------------------------------------------------------------------
bool Witness::sendToClient(const Mercury::MessageHandler& msgHandler, MemoryStream& s)
{
	return true;
}

//-------------------------------------------------------------------------------------
}
