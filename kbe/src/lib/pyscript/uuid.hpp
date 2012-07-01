/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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

#ifndef __KBESCRIPTUUID_H__
#define __KBESCRIPTUUID_H__
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "scriptobject.hpp"

namespace KBEngine{ namespace script{

class Uuid
{						
public:	
	/** 这里返回值是不对的， uuid为128位。 目前这个模块没有被使用到暂时不做解决方案。 */
	static uint64 uuid();

	/** 初始化 */
	static bool initialize(void);
	static void finalise(void);
private:
	static PyObject* uuidMethod_;							// uuid.uuid4方法指针
	static bool	isInit;										// 是否已经被初始化
} ;

}
}
#endif