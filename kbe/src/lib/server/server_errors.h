/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#ifndef KBE_SERVER_ERRORS_H
#define KBE_SERVER_ERRORS_H

#include "common/common.h"

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
#define SERVER_ERR_ACCOUNT_CREATE_FAILED			7			// 创建账号失败（已经存在一个相同的账号）。
#define SERVER_ERR_BUSY								8			// 操作过于繁忙(例如：在服务器前一次请求未执行完毕的情况下连续N次创建账号)。
#define SERVER_ERR_ACCOUNT_LOGIN_ANOTHER			9			// 当前账号在另一处登录了。
#define SERVER_ERR_ACCOUNT_IS_ONLINE				10			// 你已经登录了，服务器拒绝再次登录。
#define SERVER_ERR_PROXY_DESTROYED					11			// 与客户端关联的proxy在服务器上已经销毁。
#define SERVER_ERR_ENTITYDEFS_NOT_MATCH				12			// entityDefs不匹配。
#define SERVER_ERR_IN_SHUTTINGDOWN					13			// 服务器正在关闭中
#define SERVER_ERR_NAME_MAIL						14			// email地址错误。
#define SERVER_ERR_ACCOUNT_LOCK						15			// 账号被冻结。
#define SERVER_ERR_ACCOUNT_DEADLINE					16			// 账号已过期。
#define SERVER_ERR_ACCOUNT_NOT_ACTIVATED			17			// 账号未激活。
#define SERVER_ERR_VERSION_NOT_MATCH				18			// 与服务端的版本不匹配。
#define SERVER_ERR_OP_FAILED						19			// 操作失败。
#define SERVER_ERR_SRV_STARTING						20			// 服务器正在启动中。
#define SERVER_ERR_ACCOUNT_REGISTER_NOT_AVAILABLE	21			// 未开放账号注册功能。
#define SERVER_ERR_CANNOT_USE_MAIL					22			// 不能使用email地址。
#define SERVER_ERR_NOT_FOUND_ACCOUNT				23			// 找不到此账号。
#define SERVER_ERR_DB								24			// 数据库错误(请检查dbmgr日志和DB)。

const char SERVER_ERR_STR[][256] = {
	"SERVER_SUCCESS",
	"SERVER_ERR_SRV_NO_READY",
	"SERVER_ERR_SRV_OVERLOAD",
	"SERVER_ERR_ILLEGAL_LOGIN",
	"SERVER_ERR_NAME_PASSWORD",
	"SERVER_ERR_NAME",
	"SERVER_ERR_PASSWORD",
	"SERVER_ERR_ACCOUNT_CREATE_FAILED",
	"SERVER_ERR_BUSY",
	"SERVER_ERR_ACCOUNT_LOGIN_ANOTHER",
	"SERVER_ERR_ACCOUNT_IS_ONLINE",
	"SERVER_ERR_PROXY_DESTROYED",
	"SERVER_ERR_ENTITYDEFS_NOT_MATCH",
	"SERVER_ERR_IN_SHUTTINGDOWN",
	"SERVER_ERR_NAME_MAIL",
	"SERVER_ERR_ACCOUNT_LOCK",
	"SERVER_ERR_ACCOUNT_DEADLINE",
	"SERVER_ERR_ACCOUNT_NOT_ACTIVATED",
	"SERVER_ERR_VERSION_NOT_MATCH",
	"SERVER_ERR_OP_FAILED",
	"SERVER_ERR_SRV_STARTING",
	"SERVER_ERR_ACCOUNT_REGISTER_NOT_AVAILABLE",
	"SERVER_ERR_CANNOT_USE_MAIL",
	"SERVER_ERR_NOT_FOUND_ACCOUNT",
	"SERVER_ERR_DB"
};

}

#endif // KBE_SERVER_ERRORS_H
