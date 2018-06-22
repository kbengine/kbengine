// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "timer.h"
#include "helper/debug_helper.h"
#include "thread/threadguard.h"

namespace KBEngine
{
//-------------------------------------------------------------------------------------
void TimerHandle::cancel()
{
	if (pTime_ != NULL)
	{
		TimeBase* pTime = pTime_;
		pTime_ = NULL;
		pTime->cancel();
	}
}

//-------------------------------------------------------------------------------------
} 
