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

	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
	{
		// 初始化默认AOI范围
		ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();
		setAoiRadius(ecinfo.defaultAoIRadius, ecinfo.defaultAoIHysteresisArea);
		
		if(aoiRadius_ > 0.0f)
			pAOITrigger_ = new AOITrigger((RangeNode*)pEntity->pEntityRangeNode(), aoiRadius_, aoiRadius_);
	}

	Cellapp::getSingleton().addUpdatable(this);
}

//-------------------------------------------------------------------------------------
void Witness::detach(Entity* pEntity)
{
	DEBUG_MSG(boost::format("Witness::detach: %1%(%2%).\n") % 
		pEntity->getScriptName() % pEntity->getID());

	pEntity_ = NULL;
	aoiRadius_ = 0.0f;
	aoiHysteresisArea_ = 5.0f;
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
	if(!g_kbeSrvConfig.getCellApp().use_coordinate_system)
		return;

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
	AOI_ENTITIES::iterator iter = std::find_if(aoiEntities_.begin(), aoiEntities_.end(), findif_vector_entityref_exist_handler(pEntity));
	if(iter != aoiEntities_.end())
		return;

	DEBUG_MSG(boost::format("Witness::onEnterAOI: %1% entity=%2%\n") % pEntity_->getID() % pEntity->getID());
	
	EntityRef* pEntityRef = new EntityRef(pEntity);
	pEntityRef->flags(pEntityRef->flags() | ENTITYREF_FLAG_ENTER_CLIENT_PENDING);
	aoiEntities_.push_back(pEntityRef);
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveAOI(Entity* pEntity)
{
	AOI_ENTITIES::iterator iter = std::find_if(aoiEntities_.begin(), aoiEntities_.end(), findif_vector_entityref_exist_handler(pEntity));
	if(iter == aoiEntities_.end())
		return;

	DEBUG_MSG(boost::format("Witness::onLeaveAOI: %1% entity=%2%\n") % pEntity_->getID() % pEntity->getID());

	delete (*iter);
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

	if(pAOITrigger_)
		pAOITrigger_->install();
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveSpace(Space* pSpace)
{
	if(pAOITrigger_)
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
	
	// 获取每帧剩余可写大小， 将优先更新的内容写入， 剩余的内容往下一个周期递推
	int currPacketSize = pChannel->bundlesLength();
	int remainPacketSize = PACKET_MAX_SIZE_TCP - currPacketSize;
	if(remainPacketSize <= 0)
		return;
	
	SPACE_ID spaceID = pEntity_->getSpaceID();

	Mercury::Bundle* pSendBundle = NEW_BUNDLE();
	if(aoiEntities_.size() > 0)
	{
		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity_->getID(), (*pSendBundle));

		AOI_ENTITIES::iterator iter = aoiEntities_.begin();
		for(; iter != aoiEntities_.end(); iter++)
		{
			if(remainPacketSize <= 0)
				break;

			Entity* otherEntity = (*iter)->pEntity();

			if(((*iter)->flags() & ENTITYREF_FLAG_ENTER_CLIENT_PENDING) > 0)
			{
				(*iter)->removeflags(ENTITYREF_FLAG_ENTER_CLIENT_PENDING);

				Mercury::Bundle* pForwardBundle1 = Mercury::Bundle::ObjPool().createObject();
				Mercury::Bundle* pForwardBundle2 = Mercury::Bundle::ObjPool().createObject();

				MemoryStream* s1 = MemoryStream::ObjPool().createObject();
				otherEntity->addPositionAndDirectionToStream(*s1);
				otherEntity->addClientDataToStream(s1);

				(*pForwardBundle1).newMessage(ClientInterface::onUpdatePropertys);
				(*pForwardBundle1) << otherEntity->getID();
				(*pForwardBundle1).append(*s1);
				MemoryStream::ObjPool().reclaimObject(s1);
		
				(*pForwardBundle2).newMessage(ClientInterface::onEntityEnterWorld);
				(*pForwardBundle2) << otherEntity->getID();
				(*pForwardBundle2) << otherEntity->getScriptModule()->getUType();
				(*pForwardBundle2) << spaceID;

				MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle1));
				MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle2));
				
				remainPacketSize -= pForwardBundle1->packetsLength();
				remainPacketSize -= pForwardBundle2->packetsLength();

				Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle1);
				Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle2);
			}
			else if(((*iter)->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
			{
				(*iter)->removeflags(ENTITYREF_FLAG_LEAVE_CLIENT_PENDING);

				Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

				(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveWorld);
				(*pForwardBundle) << otherEntity->getID();
				(*pForwardBundle) << spaceID;

				MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));
				Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
			}
			else
			{
				Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
				MemoryStream* s1 = MemoryStream::ObjPool().createObject();
				otherEntity->addVolatileDataToStream(s1);

				(*pForwardBundle).newMessage(ClientInterface::onUpdateVolatileData);
				(*pForwardBundle) << otherEntity->getID();
				(*pForwardBundle).append(*s1);
				MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));
				Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
			}
		}
	}

	if(!pSendBundle->isEmpty())
		pChannel->bundles().push_back(pSendBundle);
	else
		DELETE_BUNDLE(pSendBundle);

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
