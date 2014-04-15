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

#include "space.hpp"	
#include "spaces.hpp"	
#include "loadnavmesh_threadtasks.hpp"
#include "server/serverconfig.hpp"
#include "cstdkbe/deadline.hpp"
#include "navigation/navigation.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
bool LoadNavmeshTask::process()
{
	Navigation::getSingleton().loadNavmesh(name_);
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState LoadNavmeshTask::presentMainThread()
{
	NavMeshHandle* pNavMeshHandle = Navigation::getSingleton().findNavmesh(name_);
	
	Space* pSpace = Spaces::findSpace(spaceID_);
	if(pSpace == NULL)
	{
		ERROR_MSG(boost::format("LoadNavmeshTask::presentMainThread(): not found space(%1%)\n") % 
			spaceID_);
	}
	else
	{
		pSpace->onLoadedSpaceGeometryMapping(pNavMeshHandle);
	}
	
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
}
