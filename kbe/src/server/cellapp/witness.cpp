/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "witness.hpp"
#include "entity.hpp"	
#include "profile.hpp"
#include "cellapp.hpp"
#include "aoi_trigger.hpp"
#include "network/channel.hpp"	
#include "network/bundle.hpp"
#include "client_lib/client_interface.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Witness::Witness():
pEntity_(NULL),
aoiRadius_(0.0f),
aoiHysteresisArea_(5.0f),
pAOITrigger_(NULL),
aoiEntities_()
{
}

//-------------------------------------------------------------------------------------
Witness::~Witness()
{
	pEntity_ = NULL;
	SAFE_RELEASE(pAOITrigger_);
}

//-------------------------------------------------------------------------------------
void Witness::attach(Entity* pEntity)
{
	DEBUG_MSG(boost::format("Witness::attach: %1%(%2%).\n") % 
		pEntity->getScriptName() % pEntity->getID());

	pEntity_ = pEntity;

	// 初始化默认AOI范围
	ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();
	setAoiRadius(ecinfo.defaultAoIRadius, ecinfo.defaultAoIHysteresisArea);
	
	if(aoiRadius_ > 0.0f)
		pAOITrigger_ = new AOITrigger((RangeNode*)pEntity->pEntityRangeNode(), aoiRadius_, aoiRadius_);

	Cellapp::getSingleton().addUpdatable(this);
}

//-------------------------------------------------------------------------------------
void Witness::detach(Entity* pEntity)
{
	DEBUG_MSG(boost::format("Witness::detach: %1%(%2%).\n") % 
		pEntity->getScriptName() % pEntity->getID());

	pEntity_ = NULL;
	aoiRadius_ = 0.0f;
	aoiHysteresisArea_ = 0.0f;
	SAFE_RELEASE(pAOITrigger_);
	
	aoiEntities_.clear();
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
void Witness::setAoiRadius(float radius, float hyst)
{
	aoiRadius_ = radius;
	aoiHysteresisArea_ = hyst;

	if(aoiRadius_ + aoiHysteresisArea_ > g_kbeSrvConfig.getCellApp().ghostDistance)
	{
		aoiRadius_ = g_kbeSrvConfig.getCellApp().ghostDistance - 5.0f;
		aoiHysteresisArea_ = 5.0f;
	}

	if(aoiRadius_ > 0.0f)
	{
		if(pAOITrigger_ == NULL)
		{
			pAOITrigger_ = new AOITrigger((RangeNode*)pEntity_->pEntityRangeNode(), aoiRadius_, aoiRadius_);
		}
		else
		{
			pAOITrigger_->range(aoiRadius_, aoiRadius_);
		}
	}
}

//-------------------------------------------------------------------------------------
void Witness::onEnterAOI(Entity* pEntity)
{
	AOI_ENTITIES::iterator iter = std::find(aoiEntities_.begin(), aoiEntities_.end(), pEntity);
	if(iter != aoiEntities_.end())
		return;

	DEBUG_MSG(boost::format("Witness::onEnterAOI: %1% entity=%2%\n") % pEntity_->getID() % pEntity->getID());

	aoiEntities_.push_back(pEntity);
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveAOI(Entity* pEntity)
{
	AOI_ENTITIES::iterator iter = std::find(aoiEntities_.begin(), aoiEntities_.end(), pEntity);
	if(iter == aoiEntities_.end())
		return;

	DEBUG_MSG(boost::format("Witness::onLeaveAOI: %1% entity=%2%\n") % pEntity_->getID() % pEntity->getID());

	aoiEntities_.erase(iter);
}

//-------------------------------------------------------------------------------------
void Witness::onEnterSpace(Space* pSpace)
{
	Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle* pForwardPosDirBundle = Mercury::Bundle::ObjPool().createObject();
	
	(*pForwardPosDirBundle).newMessage(ClientInterface::onUpdatePropertys);
	MemoryStream* s1 = MemoryStream::ObjPool().createObject();
	(*pForwardPosDirBundle) << pEntity_->getID();
	pEntity_->addPositionAndDirectionToStream(*s1);
	(*pForwardPosDirBundle).append(*s1);
	MemoryStream::ObjPool().reclaimObject(s1);
	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardPosDirBundle));

	(*pForwardBundle).newMessage(ClientInterface::onEntityEnterWorld);
	(*pForwardBundle) << pEntity_->getID();
	(*pForwardBundle) << pEntity_->getScriptModule()->getUType();
	(*pForwardBundle) << pSpace->getID();

	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardBundle));
	pEntity_->getClientMailbox()->postMail(*pSendBundle);

	Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardPosDirBundle);

	pAOITrigger_->install();
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveSpace(Space* pSpace)
{
	pAOITrigger_->uninstall();

	Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

	(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveWorld);
	(*pForwardBundle) << pEntity_->getID();
	(*pForwardBundle) << pSpace->getID();

	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardBundle));
	pEntity_->getClientMailbox()->postMail(*pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
}

//-------------------------------------------------------------------------------------
Witness::Bundles* Witness::pBundles()
{
	if(pEntity_ == NULL)
		return NULL;

	if(!pEntity_->getClientMailbox())
		return NULL;

	Mercury::Channel* pChannel = pEntity_->getClientMailbox()->getChannel();
	if(!pChannel)
		return NULL;

	return &pChannel->bundles();
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
	Bundles* lpBundles = pBundles();

	if(lpBundles)
	{
		lpBundles->push_back(pBundle);
		return true;
	}

	ERROR_MSG(boost::format("Witness::sendToClient: %1% pBundles is NULL, not found channel.\n") % pEntity_->getID());
	return false;
}

//-------------------------------------------------------------------------------------
}
