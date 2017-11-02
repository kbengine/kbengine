/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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


#include "shutdowner.h"
#include "network/event_dispatcher.h"

namespace KBEngine { 


//-------------------------------------------------------------------------------------
Shutdowner::Shutdowner(ShutdownHandler* pShutdownHandler) :
	pShutdownHandler_(pShutdownHandler),
	pTimerHandle_(),
	pDispatcher_(0),
	tickPeriod_(1.0f)
{
}

//-------------------------------------------------------------------------------------
Shutdowner::~Shutdowner()
{
	cancel();
}

//-------------------------------------------------------------------------------------
void Shutdowner::cancel()
{
	pTimerHandle_.cancel();
}

//-------------------------------------------------------------------------------------
void Shutdowner::shutdown(float period, float tickPeriod, Network::EventDispatcher& dispatcher)
{
	pTimerHandle_.cancel();
	INFO_MSG(fmt::format("Shutdowner::onShutdownBegin: shutting down(period={}, tickPeriod={})\n", 
		period, tickPeriod));
	
	tickPeriod_ = tickPeriod;
	pShutdownHandler_->setShuttingdown(ShutdownHandler::SHUTDOWN_STATE_BEGIN);
	
	if(period <= 0.f)
	{
		pShutdownHandler_->onShutdownBegin();
		
		INFO_MSG( "Shutdowner::onShutdown: shutting down\n" );
		pShutdownHandler_->setShuttingdown(ShutdownHandler::SHUTDOWN_STATE_RUNNING);
		pShutdownHandler_->onShutdown(true);
		
		INFO_MSG( "Shutdowner::onShutdownEnd: shutting down\n" );
		pShutdownHandler_->setShuttingdown(ShutdownHandler::SHUTDOWN_STATE_END);
		pShutdownHandler_->onShutdownEnd();
		return;	
	}
	
	pTimerHandle_ = dispatcher.addTimer(int(period * 1000000),
									this, (void *)TIMEOUT_SHUTDOWN_TICK);
	
	pDispatcher_ = &dispatcher;
	pShutdownHandler_->onShutdownBegin();
}

//-------------------------------------------------------------------------------------
void Shutdowner::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_SHUTDOWN_TICK:
		{
			INFO_MSG( "Shutdowner::onShutdown: shutting down\n" );
			pShutdownHandler_->setShuttingdown(ShutdownHandler::SHUTDOWN_STATE_RUNNING);
			pShutdownHandler_->onShutdown(true);
			cancel();
			
			pTimerHandle_ = pDispatcher_->addTimer(int(tickPeriod_ * 1000000),
											this, (void *)TIMEOUT_SHUTDOWN_END_TICK);
	
			break;
		}
		case TIMEOUT_SHUTDOWN_END_TICK:
			pShutdownHandler_->setShuttingdown(ShutdownHandler::SHUTDOWN_STATE_END);
			if(!pShutdownHandler_->canShutdown())
			{
				//INFO_MSG(fmt::format("Shutdowner::onShutdownEnd: waiting for {} to complete!\n",
				//	pShutdownHandler_->lastShutdownFailReason()));
				
				pShutdownHandler_->onShutdown(false);
				
				cancel();
				
				pTimerHandle_ = pDispatcher_->addTimer(int(100000),
												this, (void *)TIMEOUT_SHUTDOWN_END_TICK);
			
				break;
			}
			
			INFO_MSG( "Shutdowner::onShutdownEnd: shutting down\n" );

			cancel();
			pShutdownHandler_->onShutdownEnd();
			break;
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------

}
