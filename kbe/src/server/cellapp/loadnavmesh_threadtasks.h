// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_LOADNAVMESH_THREADTASKS_H
#define KBE_LOADNAVMESH_THREADTASKS_H

#include "common/common.h"
#include "thread/threadtask.h"
#include "helper/debug_helper.h"

namespace KBEngine{ 

class LoadNavmeshTask : public thread::TPTask
{
public:
	LoadNavmeshTask(const std::string& resPath, SPACE_ID spaceID, const std::map< int, std::string >& params):
	resPath_(resPath),
	spaceID_(spaceID),
	params_(params)
	{
	}

	virtual ~LoadNavmeshTask(){}
	virtual bool process();
	virtual thread::TPTask::TPTaskState presentMainThread();

protected:
	std::string resPath_;
	SPACE_ID spaceID_;
	std::map< int, std::string > params_;
};


}

#endif // KBE_LOADNAVMESH_THREADTASKS_H
