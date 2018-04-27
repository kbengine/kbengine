// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_TASKS_H
#define KBE_TASKS_H

#include "common/task.h"
#include "common/common.h"

namespace KBEngine
{

/**
 *	ÈÎÎñÈÝÆ÷
 */
class Tasks
{
public:
	Tasks();
	~Tasks();

	void add(Task * pTask);
	bool cancel(Task * pTask);
	void process();
private:
	
	typedef std::vector<KBEngine::Task *> Container;
	Container container_;
};

}

#endif // KBE_TASKS_H
