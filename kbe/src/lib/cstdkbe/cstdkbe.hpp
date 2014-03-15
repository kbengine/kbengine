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
#ifndef __CSTDKBE__
#define __CSTDKBE__
#include "cstdkbe/platform.hpp"
#include "cstdkbe/singleton.hpp"
#include "cstdkbe/kbeversion.hpp"
#include "cstdkbe/kbemalloc.hpp"
#include "cstdkbe/stringconv.hpp"
namespace KBEngine{
/** 安全的释放一个指针内存 */
#define SAFE_RELEASE(i)										\
	if (i)													\
		{													\
			delete i;										\
			i = NULL;										\
		}

/** 安全的释放一个指针数组内存 */
#define SAFE_RELEASE_ARRAY(i)								\
	if (i)													\
		{													\
			delete[] i;										\
			i = NULL;										\
		}

#ifdef CODE_INLINE
    #define INLINE    inline
#else
    #define INLINE
#endif

/** kbe时间 */
extern GAME_TIME g_kbetime;

/** 账号的类别 */
enum ACCOUNT_TYPE
{
	ACCOUNT_TYPE_NORMAL = 1,	// 普通账号
	ACCOUNT_TYPE_MAIL = 2,		// email账号(需激活)
	ACCOUNT_TYPE_SMART = 3		// 智能识别
};

enum ACCOUNT_FLAGS
{
	ACCOUNT_FLAG_NORMAL = 0x00000000,
	ACCOUNT_FLAG_LOCK = 0x000000001,
	ACCOUNT_FLAG_NOT_ACTIVATED = 0x000000002
};

/** entity的mailbox类别 */
enum ENTITY_MAILBOX_TYPE
{
	MAILBOX_TYPE_CELL												= 0,
	MAILBOX_TYPE_BASE												= 1,
	MAILBOX_TYPE_CLIENT												= 2,
	MAILBOX_TYPE_CELL_VIA_BASE										= 3,
	MAILBOX_TYPE_BASE_VIA_CELL										= 4,
	MAILBOX_TYPE_CLIENT_VIA_CELL									= 5,
	MAILBOX_TYPE_CLIENT_VIA_BASE									= 6,
};

/** mailbox的类别对换为字符串名称 严格和ENTITY_MAILBOX_TYPE索引匹配 */
const char ENTITY_MAILBOX_TYPE_TO_NAME_TABLE[][8] = 
{
	"cell",
	"base",
	"client",
	"cell",
	"base",
	"client",
	"client",
};

/** 定义服务器各组件类别 */
enum COMPONENT_TYPE
{
	UNKNOWN_COMPONENT_TYPE	= 0,
	DBMGR_TYPE				= 1,
	LOGINAPP_TYPE			= 2,
	BASEAPPMGR_TYPE			= 3,
	CELLAPPMGR_TYPE			= 4,
	CELLAPP_TYPE			= 5,
	BASEAPP_TYPE			= 6,
	CLIENT_TYPE				= 7,
	MACHINE_TYPE			= 8,
	CENTER_TYPE				= 9,
	CONSOLE_TYPE			= 10,
	MESSAGELOG_TYPE			= 11,
	RESOURCEMGR_TYPE		= 12,
	BOTS_TYPE				= 13,
	WATCHER_TYPE			= 14,
	BILLING_TYPE			= 15,
	COMPONENT_END_TYPE		= 16,
};

/** 当前服务器组件类别和ID */
extern COMPONENT_TYPE g_componentType;
extern COMPONENT_ID g_componentID;

/** 定义服务器各组件名称 */
const char COMPONENT_NAME[][255] = {
	"unknown",
	"dbmgr",
	"loginapp",
	"baseappmgr",
	"cellappmgr",
	"cellapp",
	"baseapp",
	"client",
	"kbmachine",
	"kbcenter",
	"console",
	"messagelog",
	"resourcemgr",
	"bots",
	"watcher",
	"billing",
};

const char COMPONENT_NAME_1[][255] = {
	"unknown    ",
	"dbmgr      ",
	"loginapp   ",
	"baseappmgr ",
	"cellappmgr ",
	"cellapp    ",
	"baseapp    ",
	"client     ",
	"kbmachine  ",
	"kbcenter   ",
	"console    ",
	"messagelog ",
	"resourcemgr",
	"bots",
	"watcher",
	"billing",
};

inline const char* COMPONENT_NAME_EX(COMPONENT_TYPE CTYPE)
{									
	if(CTYPE < 0 || CTYPE >= COMPONENT_END_TYPE)
	{
		return COMPONENT_NAME[UNKNOWN_COMPONENT_TYPE];
	}

	return COMPONENT_NAME[CTYPE];
}

inline const char* COMPONENT_NAME_EX_1(COMPONENT_TYPE CTYPE)
{									
	if(CTYPE < 0 || CTYPE >= COMPONENT_END_TYPE)
	{
		return COMPONENT_NAME_1[UNKNOWN_COMPONENT_TYPE];
	}

	return COMPONENT_NAME_1[CTYPE];
}

inline COMPONENT_TYPE ComponentName2ComponentType(const char* name)
{
	for(int i=0; i<(int)COMPONENT_END_TYPE; i++)
	{
		if(kbe_stricmp(COMPONENT_NAME[i], name) == 0)
			return (COMPONENT_TYPE)i;
	}

	return UNKNOWN_COMPONENT_TYPE;
}

// 所有的组件列表
const COMPONENT_TYPE ALL_COMPONENT_TYPES[] = {BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, DBMGR_TYPE, CELLAPP_TYPE, 
						BASEAPP_TYPE, LOGINAPP_TYPE, MACHINE_TYPE, CONSOLE_TYPE, MESSAGELOG_TYPE, RESOURCEMGR_TYPE, 
						CENTER_TYPE, WATCHER_TYPE, BILLING_TYPE, BOTS_TYPE, UNKNOWN_COMPONENT_TYPE};

// 所有的后端组件列表
const COMPONENT_TYPE ALL_SERVER_COMPONENT_TYPES[] = {BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, DBMGR_TYPE, CELLAPP_TYPE, 
						BASEAPP_TYPE, LOGINAPP_TYPE, MACHINE_TYPE, MESSAGELOG_TYPE, RESOURCEMGR_TYPE, 
						CENTER_TYPE, WATCHER_TYPE, BILLING_TYPE, BOTS_TYPE, UNKNOWN_COMPONENT_TYPE};

// 所有的后端组件列表
const COMPONENT_TYPE ALL_GAME_SERVER_COMPONENT_TYPES[] = {BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, DBMGR_TYPE, CELLAPP_TYPE, 
						BASEAPP_TYPE, LOGINAPP_TYPE, BILLING_TYPE, UNKNOWN_COMPONENT_TYPE};

// 返回是否是一个有效的组件
#define VALID_COMPONENT(C_TYPE) ((C_TYPE) > 0 && (C_TYPE) < COMPONENT_END_TYPE)


// 前端应用的类别
enum COMPONENT_CLIENT_TYPE
{
	UNKNOWN_CLIENT_COMPONENT_TYPE	= 0,
	CLIENT_TYPE_MOBILE				= 1,	// 手机类
	CLIENT_TYPE_PC					= 2,	// pc， 一般都是exe客户端
	CLIENT_TYPE_BROWSER				= 3,	// web应用， html5，flash
	CLIENT_TYPE_BOTS				= 4,	// bots
	CLIENT_TYPE_END					= 5		// end
};

/** 定义前端应用的类别名称 */
const char COMPONENT_CLIENT_NAME[][255] = {
	"UNKNOWN_CLIENT_COMPONENT_TYPE",
	"CLIENT_TYPE_MOBILE",
	"CLIENT_TYPE_PC",
	"CLIENT_TYPE_BROWSER",
	"CLIENT_TYPE_BOTS",
};

// 所有前端应用的类别
const COMPONENT_CLIENT_TYPE ALL_CLIENT_TYPES[] = {CLIENT_TYPE_MOBILE, CLIENT_TYPE_PC, CLIENT_TYPE_BROWSER, 
												CLIENT_TYPE_BOTS, UNKNOWN_CLIENT_COMPONENT_TYPE};

typedef int8 CLIENT_CTYPE;

// 前端是否支持浮点数
// #define CLIENT_NO_FLOAT

// 一个cell的默认的边界或者最小大小
#define CELL_DEF_MIN_AREA_SIZE				500.0f

/** 一个空间的一个chunk大小 */
#define SPACE_CHUNK_SIZE					100


/** 检查用户名合法性 */
inline bool validName(const char* name, int size)
{
	if(size >= 256)
		return false;

	for(int i=0; i<size; i++)
	{
		char ch = name[i];
		if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (ch == '_'))
			continue;

		return false;
	}

	return true;
}

inline bool validName(const std::string& name)
{
	return validName(name.c_str(), name.size());
}

/** 检查email地址合法性 
严格匹配请用如下表达式
[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?
*/
#ifdef USE_REGEX
#include <regex>
#endif

inline bool email_isvalid(const char *address) 
{
#ifdef USE_REGEX
	std::tr1::regex _mail_pattern("([a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?)");
	return std::tr1::regex_match(accountName, _mail_pattern);
#endif
	int len = strlen(address);
	if(len <= 3)
		return false;

	char ch = address[len - 1];
	if(!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9')))
		return false;

	int        count = 0;
	const char *c, *domain;
	static const char *rfc822_specials = "()<>@,;:\\\"[]";

	/* first we validate the name portion (name@domain) */
	for (c = address;  *c;  c++) {
	if (*c == '\"' && (c == address || *(c - 1) == '.' || *(c - 1) == 
		'\"')) {
	  while (*++c) {
		if (*c == '\"') break;
		if (*c == '\\' && (*++c == ' ')) continue;
		if (*c <= ' ' || *c >= 127) return false;
	  }
	  if (!*c++) return false;
	  if (*c == '@') break;
	  if (*c != '.') return false;
	  continue;
	}
	if (*c == '@') break;
	if (*c <= ' ' || *c >= 127) return false;
	if (strchr(rfc822_specials, *c)) return false;
	}
	if (c == address || *(c - 1) == '.') return false;

	/* next we validate the domain portion (name@domain) */
	if (!*(domain = ++c)) return false;
	do {
	if (*c == '.') {
	  if (c == domain || *(c - 1) == '.') return false;
	  count++;
	}
	if (*c <= ' ' || *c >= 127) return false;
	if (strchr(rfc822_specials, *c)) return false;
	} while (*++c);

	return (count >= 1);
}

}
#endif // __CSTDKBE__
