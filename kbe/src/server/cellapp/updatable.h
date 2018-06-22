// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_UPDATABLE_H
#define KBE_UPDATABLE_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"

// #define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32	
#else
// linux include
#endif

namespace KBEngine{

/*
	用来描述一个总是会被更新的对象， app每个tick都会调用所有的
	Updatable来更新状态， 需要实现不同的Updatable来完成不同的更新特性。
*/
class Updatable
{
public:
	Updatable();
	~Updatable();

	virtual bool update() = 0;

	virtual uint8 updatePriority() const {
		return 0;
	}

	std::string c_str() { return updatableName; }

	// 自身在Updatables容器中的位置
	int removeIdx;

	std::string updatableName;
};

}
#endif
