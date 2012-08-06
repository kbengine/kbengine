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

#ifndef __MERCURY_ERRORS_H__
#define __MERCURY_ERRORS_H__

// common include
#include "cstdkbe/cstdkbe.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine { 

typedef uint16 MERCURY_ERROR_CODE;					// 错误码类别


#define MERCURY_SUCCESS								0			// 非法登录。
#define MERCURY_ERR_SRV_NO_READY					1			// 服务器没有准备好。
#define MERCURY_ERR_SRV_OVERLOAD					2			// 服务器负载过重。
#define MERCURY_ERR_ILLEGAL_LOGIN					3			// 非法登录。
#define MERCURY_ERR_NAME_PASSWORD					4			// 用户名或者密码不正确。
#define MERCURY_ERR_NAME							5			// 用户名不正确。
#define MERCURY_ERR_PASSWORD						6			// 密码不正确。
#define MERCURY_ERR_ACCOUNT_CREATE					7			// 创建账号失败（已经存在一个相同的账号）。
#define MERCURY_ERR_BUSY							8			// 操作过于繁忙(例如：在服务器前一次请求未执行完毕的情况下连续N次创建账号)。
}
#endif // __MERCURY_ERRORS_H__
