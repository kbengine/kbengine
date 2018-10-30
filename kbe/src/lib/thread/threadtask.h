// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_THREADTASK_H
#define KBE_THREADTASK_H

// common include	
// #define NDEBUG
#include "common/common.h"
#include "common/task.h"
#include "helper/debug_helper.h"

namespace KBEngine{ namespace thread{

/*
	线程池的线程基类
*/

class TPTask : public Task
{
public:
	enum TPTaskState
	{
		/// 一个任务已经完成
		TPTASK_STATE_COMPLETED = 0,

		/// 继续在主线程执行
		TPTASK_STATE_CONTINUE_MAINTHREAD = 1,

		// 继续在子线程执行
		TPTASK_STATE_CONTINUE_CHILDTHREAD = 2,
	};

	/**
		返回值： thread::TPTask::TPTaskState， 请参看TPTaskState
	*/
	virtual thread::TPTask::TPTaskState presentMainThread(){ 
		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}
};

}
}

#endif // KBE_THREADTASK_H
