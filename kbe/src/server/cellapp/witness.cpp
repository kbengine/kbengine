#include "witness.hpp"
#include "entity.hpp"	
#include "profile.hpp"
#include "cellapp.hpp"
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
void Witness::attach(Entity* pEntity)
{
	pEntity_ = pEntity;

	// ³õÊ¼»¯Ä¬ÈÏAOI·¶Î§
	ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();
	setAoiRadius(ecinfo.defaultAoIRadius, ecinfo.defaultAoIHysteresisArea);

	Cellapp::getSingleton().addUpdatable(this);
}

//-------------------------------------------------------------------------------------
void Witness::detach(Entity* pEntity)
{
	pEntity_ = NULL;
	aoiRadius_ = 0.0f;
	aoiHysteresisArea_ = 0.0f;

	Cellapp::getSingleton().removeUpdatable(this);
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
	SCOPED_PROFILE(CLIENT_UPDATE_PROFILE);

	if(!pEntity_->clientMailbox_)
		return;

	Mercury::Channel* pChannel = pEntity_->clientMailbox_->getChannel();
	if(!pChannel)
		return;

	{
		AUTO_SCOPED_PROFILE("updateClientSend");
		pChannel->send();
	}
}

//-------------------------------------------------------------------------------------
bool Witness::sendToClient(const Mercury::MessageHandler& msgHandler, MemoryStream& s)
{
	return true;
}

//-------------------------------------------------------------------------------------
}
