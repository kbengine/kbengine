/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#include "dbmgr.h"
#include "dbtasks.h"
#include "update_dblog_handler.h"
#include "db_interface/db_interface.h"
#include "db_interface/kbe_tables.h"
#include "server/serverconfig.h"
#include "thread/threadpool.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
UpdateDBServerLogHandler::UpdateDBServerLogHandler() : 
pTimerHandle_(NULL)
{
	pTimerHandle_ = new TimerHandle();
	(*pTimerHandle_) = Dbmgr::getSingleton().dispatcher().addTimer(KBEServerLogTable::TIMEOUT * 1000000, this,
							NULL);
}

//-------------------------------------------------------------------------------------
UpdateDBServerLogHandler::~UpdateDBServerLogHandler()
{
	cancel();
}

//-------------------------------------------------------------------------------------
void UpdateDBServerLogHandler::cancel()
{
	if(pTimerHandle_)
	{
		pTimerHandle_->cancel();
		delete pTimerHandle_;
		pTimerHandle_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
void UpdateDBServerLogHandler::handleTimeout(TimerHandle, void * arg)
{
	std::string dbInterfaceName = "default";
	
	DBUtil::pThreadPool(dbInterfaceName)->
		addTask(new DBTaskServerLog());	
}

//-------------------------------------------------------------------------------------

}
