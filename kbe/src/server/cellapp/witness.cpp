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
	DEBUG_MSG(boost::format("Witness::attach: %1%(%2%).\n") % 
		pEntity->getScriptName() % pEntity->getID());

	pEntity_ = pEntity;

	// 初始化默认AOI范围
	ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();
	setAoiRadius(ecinfo.defaultAoIRadius, ecinfo.defaultAoIHysteresisArea);

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
