/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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
