// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_TASK_H
#define KBE_TASK_H

#include "common/common.h"

namespace KBEngine
{

/**
 *	抽象一个任务
 */
class Task
{
public:
	virtual ~Task() {}
	virtual bool process() = 0;
};


}

#endif // KBE_TASK_H
