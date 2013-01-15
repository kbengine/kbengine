#include "witness.hpp"
#include "entity.hpp"	
#include "profile.hpp"
#include "cellapp.hpp"
#include "network/channel.hpp"	
#include "network/bundle.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"

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

	// 初始化默认AOI范围
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
Witness::Bundles & Witness::bundles()
{
	KBE_ASSERT(pEntity_);
	KBE_ASSERT(pEntity_->getClientMailbox());
	Mercury::Channel* pChannel = pEntity_->getClientMailbox()->getChannel();
	KBE_ASSERT(pChannel);

	return pChannel->bundles();
}

//-------------------------------------------------------------------------------------
void Witness::update()
{
	SCOPED_PROFILE(CLIENT_UPDATE_PROFILE);

	if(pEntity_ == NULL)
		return;

	if(!pEntity_->getClientMailbox())
		return;

	Mercury::Channel* pChannel = pEntity_->getClientMailbox()->getChannel();
	if(!pChannel)
		return;

	{
		// 如果数据大量阻塞发不出去将会报警
		AUTO_SCOPED_PROFILE("updateClientSend");
		pChannel->send();
	}
}

//-------------------------------------------------------------------------------------
bool Witness::sendToClient(const Mercury::MessageHandler& msgHandler, Mercury::Bundle* pBundle)
{
	bundles().push_back(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
}
