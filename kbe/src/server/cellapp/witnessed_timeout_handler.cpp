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

#include "cellapp.hpp"
#include "entity.hpp"
#include "witnessed_timeout_handler.hpp"
#include "server/serverconfig.hpp"

namespace KBEngine{	

static const uint8 TICKSECS = 5;

//-------------------------------------------------------------------------------------
WitnessedTimeoutHandler::WitnessedTimeoutHandler() : 
witnessedEntityIDs_(), pTimerHandle_(NULL)
{
}

//-------------------------------------------------------------------------------------
WitnessedTimeoutHandler::~WitnessedTimeoutHandler()
{
	cancel();
}

//-------------------------------------------------------------------------------------
void WitnessedTimeoutHandler::cancel()
{
	if(pTimerHandle_)
	{
		pTimerHandle_->cancel();
		delete pTimerHandle_;
		pTimerHandle_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
void WitnessedTimeoutHandler::handleTimeout(TimerHandle, void * arg)
{
	std::map<ENTITY_ID, uint16>::iterator iter = witnessedEntityIDs_.begin();
	for(; iter != witnessedEntityIDs_.end();)
	{
		if(iter->second > TICKSECS)
		{
			iter->second -= TICKSECS;
			iter++;
		}
		else
		{
			Entity* pEntity = Cellapp::getSingleton().findEntity(iter->first);
			
			witnessedEntityIDs_.erase(iter++);

			if(pEntity)
				pEntity->onDelWitnessed();

		}
	}

	if(witnessedEntityIDs_.size() == 0)
	{
		cancel();
		DEBUG_MSG("WitnessedTimeoutHandler::handleTimeout: witnesseds is empty, timer is canceled!\n");
	}
}

//-------------------------------------------------------------------------------------
void WitnessedTimeoutHandler::addWitnessed(Entity* pEntity)
{
	if(pEntity == NULL)
		return;

	const uint16 witness_timeout_dec = ServerConfig::getSingleton().getCellApp().witness_timeout;
	if(witness_timeout_dec == 0)
	{
		pEntity->onDelWitnessed();
		return;
	}

	if(witnessedEntityIDs_.find(pEntity->id()) != witnessedEntityIDs_.end())
		witnessedEntityIDs_[pEntity->id()] = witness_timeout_dec;
	else
		witnessedEntityIDs_.insert(std::map<ENTITY_ID, uint16>::value_type(pEntity->id(), witness_timeout_dec));
	
	if(pTimerHandle_ == NULL)
	{
		pTimerHandle_ = new TimerHandle();
		(*pTimerHandle_) = Cellapp::getSingleton().mainDispatcher().addTimer(TICKSECS * 1000000, this,
								NULL);
	}
}

//-------------------------------------------------------------------------------------
void WitnessedTimeoutHandler::delWitnessed(Entity* pEntity)
{
	if(witnessedEntityIDs_.size() == 0)
		return;

	std::map<ENTITY_ID, uint16>::iterator iter = witnessedEntityIDs_.find(pEntity->id());

	if(iter != witnessedEntityIDs_.end())
	{
		witnessedEntityIDs_.erase(iter);
	}
}

//-------------------------------------------------------------------------------------

}
