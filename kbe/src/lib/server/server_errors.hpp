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

#ifndef __SERVER_ERRORS_H__
#define __SERVER_ERRORS_H__

// common include
#include "cstdkbe/cstdkbe.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine { 

/**
	服务器错误， 主要是服务器返回给客户端用的。
*/
	
typedef uint16 SERVER_ERROR_CODE;								// 错误码类别


#define SERVER_SUCCESS								0			// 成功。
#define SERVER_ERR_SRV_NO_READY						1			// 服务器没有准备好。
#define SERVER_ERR_SRV_OVERLOAD						2			// 服务器负载过重。
#define SERVER_ERR_ILLEGAL_LOGIN					3			// 非法登录。
#define SERVER_ERR_NAME_PASSWORD					4			// 用户名或者密码不正确。
#define SERVER_ERR_NAME								5			// 用户名不正确。
#define SERVER_ERR_PASSWORD							6			// 密码不正确。
#define SERVER_ERR_ACCOUNT_CREATE					7			// 创建账号失败（已经存在一个相同的账号）。
#define SERVER_ERR_BUSY								8			// 操作过于繁忙(例如：在服务器前一次请求未执行完毕的情况下连续N次创建账号)。
#define SERVER_ERR_ANOTHER_LOGON					9			// 当前账号在另一处登录了。
#define SERVER_ERR_ACCOUNT_ONLINE					10			// 你已经登录了， 服务器拒绝再次登录。
#define SERVER_ERR_PROXY_DESTROYED					11			// 与客户端关联的proxy在服务器上已经销毁。
#define SERVER_ERR_DIGEST							12			// defmd5不匹配。
#define SERVER_ERR_SHUTTINGDOWN						13			// 服务器正在关闭中

const char SERVER_ERR_STR[][256] = {
	"SERVER_SUCCESS",
	"SERVER_ERR_SRV_NO_READY",
	"SERVER_ERR_SRV_OVERLOAD",
	"SERVER_ERR_ILLEGAL_LOGIN",
	"SERVER_ERR_NAME_PASSWORD",
	"SERVER_ERR_NAME",
	"SERVER_ERR_PASSWORD",
	"SERVER_ERR_ACCOUNT_CREATE",
	"SERVER_ERR_BUSY",
	"SERVER_ERR_ANOTHER_LOGON",
	"SERVER_ERR_ACCOUNT_ONLINE",
	"SERVER_ERR_PROXY_DESTROYED",
	"SERVER_ERR_DIGEST",
	"SERVER_ERR_SHUTTINGDOWN"
};

}
#endif // __SERVER_ERRORS_H__
