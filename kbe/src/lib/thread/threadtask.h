// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_THREADTASK_H
#define KBE_THREADTASK_H

// common include	
// #define NDEBUG
#include "common/common.h"
#include "common/task.h"
#include "helper/debug_helper.h"

namespace KBEngine{ namespace Thread{

/*
	�̳߳ص��̻߳���
*/

class TPTask : public Task
{
public:
	enum TPTaskState
	{
		/// һ�������Ѿ����
		TPTASK_STATE_COMPLETED = 0,

		/// ���������߳�ִ��
		TPTASK_STATE_CONTINUE_MAINTHREAD = 1,

		// ���������߳�ִ��
		TPTASK_STATE_CONTINUE_CHILDTHREAD = 2,
	};

	/**
		����ֵ�� Thread::TPTask::TPTaskState�� ��ο�TPTaskState
	*/
	virtual Thread::TPTask::TPTaskState presentMainThread(){ 
		return Thread::TPTask::TPTASK_STATE_COMPLETED; 
	}
};

}
}

#endif // KBE_THREADTASK_H
