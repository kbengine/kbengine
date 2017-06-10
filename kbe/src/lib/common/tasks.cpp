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

#include "tasks.h"
#include "helper/debug_helper.h"
#include "thread/threadguard.h"

namespace KBEngine
{

Tasks::Tasks() : 
	container_()
{
}

//-------------------------------------------------------------------------------------
Tasks::~Tasks()
{
}

//-------------------------------------------------------------------------------------
void Tasks::add( Task * pTask )
{
	container_.push_back( pTask );
}

//-------------------------------------------------------------------------------------
bool Tasks::cancel( Task * pTask )
{
	Container::iterator iter = std::find(container_.begin(), container_.end(), pTask);
	if (iter != container_.end())
	{
		container_.erase( iter );
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
void Tasks::process()
{
	Container::iterator iter = container_.begin();

	while (iter != container_.end())
	{
		Task * pTask = *iter;
		if(!pTask->process())
			iter = container_.erase(iter);
		else
			++iter;
	}
}

//-------------------------------------------------------------------------------------
} 
