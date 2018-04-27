// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com



#ifndef KBE_CONCURENCY_H
#define KBE_CONCURENCY_H

#include "common/platform.h"
#include "helper/debug_helper.h"
namespace KBEngine{

extern void (*pMainThreadIdleStartCallback)();
extern void (*pMainThreadIdleEndCallback)();

namespace KBEConcurrency
{

/**
	主线程处于空闲时触发
*/
inline void onStartMainThreadIdling()
{
	if(pMainThreadIdleStartCallback)
		(*pMainThreadIdleStartCallback)();
}

/**
	主线程结束空闲开始繁忙时触发
*/
inline void onEndMainThreadIdling()
{
	if(pMainThreadIdleEndCallback)
		(*pMainThreadIdleEndCallback)();
}

/**
	设置回调函数
	当回调触发时通知他们
*/
inline void setMainThreadIdleCallbacks(void (*pStartCallback)(), void (*pEndCallback)())
{
	pMainThreadIdleStartCallback = pStartCallback;
	pMainThreadIdleEndCallback = pEndCallback;
}

}

}

#endif // KBE_CONCURENCY_H
