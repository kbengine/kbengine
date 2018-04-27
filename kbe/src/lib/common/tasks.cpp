// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
