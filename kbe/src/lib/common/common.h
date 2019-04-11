// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#ifndef KBE_COMMON_H
#define KBE_COMMON_H
#include "common/platform.h"
#include "common/singleton.h"
#include "common/kbeversion.h"
#include "common/kbemalloc.h"
#include "common/stringconv.h"
#include "fmt/format.h"

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

/** 定义服务器各组件状态 */
enum COMPONENT_STATE
{
	// 初始状态
	COMPONENT_STATE_INIT = 0,

	// 进程正在运行中
	COMPONENT_STATE_RUN = 1,

	// 进程开始关闭
	COMPONENT_STATE_SHUTTINGDOWN_BEGIN = 2,

	// 进程正在关闭
	COMPONENT_STATE_SHUTTINGDOWN_RUNNING = 3,

	// 进程关闭完成了
	COMPONENT_STATE_STOP = 4
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
	CONSOLE_TYPE			= 9,
	LOGGER_TYPE				= 10,
	BOTS_TYPE				= 11,
	WATCHER_TYPE			= 12,
	INTERFACES_TYPE			= 13,
	TOOL_TYPE				= 14,
	COMPONENT_END_TYPE		= 15,
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
	"machine",
	"console",
	"logger",
	"bots",
	"watcher",
	"interfaces",
	"tool",
};

const char COMPONENT_NAME_1[][255] = {
	"unknown   ",
	"dbmgr     ",
	"loginapp  ",
	"baseappmgr",
	"cellappmgr",
	"cellapp   ",
	"baseapp   ",
	"client    ",
	"machine   ",
	"console   ",
	"logger    ",
	"bots      ",
	"watcher   ",
	"interfaces",
	"tool      ",
};

const char COMPONENT_NAME_2[][255] = {
	"   unknown",
	"     dbmgr",
	"  loginapp",
	"baseappmgr",
	"cellappmgr",
	"   cellapp",
	"   baseapp",
	"    client",
	"   machine",
	"   console",
	"    logger",
	"      bots",
	"   watcher",
	"interfaces",
	"      tool",
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

inline const char* COMPONENT_NAME_EX_2(COMPONENT_TYPE CTYPE)
{									
	if(CTYPE < 0 || CTYPE >= COMPONENT_END_TYPE)
	{
		return COMPONENT_NAME_2[UNKNOWN_COMPONENT_TYPE];
	}

	return COMPONENT_NAME_2[CTYPE];
}

inline COMPONENT_TYPE ComponentName2ComponentType(const char* name)
{
	for(int i=0; i<(int)COMPONENT_END_TYPE; ++i)
	{
		if(kbe_stricmp(COMPONENT_NAME[i], name) == 0)
			return (COMPONENT_TYPE)i;
	}

	return UNKNOWN_COMPONENT_TYPE;
}

// 所有的组件列表
const COMPONENT_TYPE ALL_COMPONENT_TYPES[] = {BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, DBMGR_TYPE, CELLAPP_TYPE, 
						BASEAPP_TYPE, LOGINAPP_TYPE, MACHINE_TYPE, CONSOLE_TYPE, TOOL_TYPE, LOGGER_TYPE,
						WATCHER_TYPE, INTERFACES_TYPE, BOTS_TYPE, UNKNOWN_COMPONENT_TYPE};

// 所有的后端组件列表
const COMPONENT_TYPE ALL_SERVER_COMPONENT_TYPES[] = {BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, DBMGR_TYPE, CELLAPP_TYPE, 
						BASEAPP_TYPE, LOGINAPP_TYPE, MACHINE_TYPE, LOGGER_TYPE, 
						WATCHER_TYPE, INTERFACES_TYPE, BOTS_TYPE, UNKNOWN_COMPONENT_TYPE};

// 所有的后端组件列表
const COMPONENT_TYPE ALL_GAME_SERVER_COMPONENT_TYPES[] = {BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, DBMGR_TYPE, CELLAPP_TYPE, 
						BASEAPP_TYPE, LOGINAPP_TYPE, INTERFACES_TYPE, UNKNOWN_COMPONENT_TYPE};

// 所有的辅助性组件
const COMPONENT_TYPE ALL_HELPER_COMPONENT_TYPE[] = {LOGGER_TYPE, UNKNOWN_COMPONENT_TYPE};

// 返回是否是一个有效的组件
#define VALID_COMPONENT(C_TYPE) ((C_TYPE) > 0 && (C_TYPE) < COMPONENT_END_TYPE)

/** 检查是否为一个游戏服务端组件类别 */
inline bool isGameServerComponentType(COMPONENT_TYPE componentType)
{
	int i = 0;
	while(true)
	{
		COMPONENT_TYPE t = ALL_GAME_SERVER_COMPONENT_TYPES[i++];
		if(t == UNKNOWN_COMPONENT_TYPE)
			break;

		if(t == componentType)
			return true;
	}

	return false;
}

// 前端应用的类别, All client type
enum COMPONENT_CLIENT_TYPE
{
	UNKNOWN_CLIENT_COMPONENT_TYPE	= 0,

	// 移动类，手机，平板电脑
	// Mobile, Phone, Pad
	CLIENT_TYPE_MOBILE				= 1,

	// 独立的Windows应用程序
	// Windows Application program
	CLIENT_TYPE_WIN					= 2,

	// 独立的Linux应用程序
	// Linux Application program
	CLIENT_TYPE_LINUX				= 3,
		
	// Mac应用程序
	// Mac Application program
	CLIENT_TYPE_MAC					= 4,
				
	// Web, HTML5, Flash
	CLIENT_TYPE_BROWSER				= 5,

	// bots
	CLIENT_TYPE_BOTS				= 6,

	// 轻端类
	CLIENT_TYPE_MINI				= 7,

	// End
	CLIENT_TYPE_END					= 8
};

/** 定义前端应用的类别名称 */
const char COMPONENT_CLIENT_NAME[][255] = {
	"UNKNOWN_CLIENT_COMPONENT_TYPE",
	"CLIENT_TYPE_MOBILE",
	"CLIENT_TYPE_WIN",
	"CLIENT_TYPE_LINUX",
	"CLIENT_TYPE_MAC",
	"CLIENT_TYPE_BROWSER",
	"CLIENT_TYPE_BOTS",
	"CLIENT_TYPE_MINI",
};

// 所有前端应用的类别
const COMPONENT_CLIENT_TYPE ALL_CLIENT_TYPES[] = {CLIENT_TYPE_MOBILE, CLIENT_TYPE_WIN, CLIENT_TYPE_LINUX, CLIENT_TYPE_MAC, 
												CLIENT_TYPE_BROWSER, CLIENT_TYPE_BOTS, CLIENT_TYPE_MINI, UNKNOWN_CLIENT_COMPONENT_TYPE};

typedef int8 CLIENT_CTYPE;

/** entity的entityCall类别 */
enum ENTITYCALL_TYPE
{
	ENTITYCALL_TYPE_CELL = 0,
	ENTITYCALL_TYPE_BASE = 1,
	ENTITYCALL_TYPE_CLIENT = 2,
	ENTITYCALL_TYPE_CELL_VIA_BASE = 3,
	ENTITYCALL_TYPE_BASE_VIA_CELL = 4,
	ENTITYCALL_TYPE_CLIENT_VIA_CELL = 5,
	ENTITYCALL_TYPE_CLIENT_VIA_BASE = 6,
};

/** 通过entityCall的类别获得该entity对应的组件类型 */
inline COMPONENT_TYPE entityCallType2ComponentType(ENTITYCALL_TYPE type)
{
	switch (type)
	{
	case ENTITYCALL_TYPE_CELL:
		return CELLAPP_TYPE;
	case ENTITYCALL_TYPE_BASE:
		return BASEAPP_TYPE;
	case ENTITYCALL_TYPE_CLIENT:
		return CLIENT_TYPE;
	case ENTITYCALL_TYPE_CELL_VIA_BASE:
		return CELLAPP_TYPE;
	case ENTITYCALL_TYPE_BASE_VIA_CELL:
		return BASEAPP_TYPE;
	case ENTITYCALL_TYPE_CLIENT_VIA_CELL:
		return CLIENT_TYPE;
	case ENTITYCALL_TYPE_CLIENT_VIA_BASE:
		return CLIENT_TYPE;
	default:
		break;
	};

	return UNKNOWN_COMPONENT_TYPE;
};

/** entityCall的类别对换为字符串名称 严格和ENTITYCALL_TYPE索引匹配 */
const char ENTITYCALL_TYPE_TO_NAME_TABLE[][8] =
{
	"cell",
	"base",
	"client",
	"cell",
	"base",
	"client",
	"client",
};

/** entityCall的类别对换为字符串名称 严格和ENTITYCALL_TYPE索引匹配 */
const char ENTITYCALL_TYPE_TO_NAME_TABLE_EX[][14] =
{
	"cell",
	"base",
	"client",
	"cellViaBase",
	"baseViaCell",
	"clientViaCell",
	"clientViaBase",
};

/*
 APP设置的标志
*/
// 默认的(未设置标记)
#define APP_FLAGS_NONE								0x00000000
// 不参与负载均衡
#define APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING	0x00000001

// 建立一个通过标记值得到名称的map，提供初始化python暴露给脚本使用
inline std::map<uint32, std::string> createAppFlagsMaps()
{
	std::map<uint32, std::string> datas;
	datas[APP_FLAGS_NONE] = "APP_FLAGS_NONE";
	datas[APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING] = "APP_FLAGS_NOT_PARTCIPATING_LOAD_BALANCING";
	return datas;
}

// 前端是否支持浮点数
// #define CLIENT_NO_FLOAT

// 一个cell的默认的边界或者最小大小
#define CELL_DEF_MIN_AREA_SIZE						500.0f

/** 一个空间的一个chunk大小 */
#define SPACE_CHUNK_SIZE							100


/** 检查用户名合法性 */
inline bool validName(const char* name, int size)
{
	if(size >= 256)
		return false;

	for(int i=0; i<size; ++i)
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
	return validName(name.c_str(), (int)name.size());
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
	std::tr1::regex _mail_pattern("^[_a-z0-9-]+(\\.[_a-z0-9-]+)*@[a-z0-9-]+(\\.[a-z0-9-]+)*(\\.[a-z]{2,4})$");
	return std::tr1::regex_match(address, _mail_pattern);
#endif
	int len = (int)strlen(address);
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
	if (c == address || *(c - 1) == '.' || *c == '\0') return false;

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

//组件ID的扩展倍数
#define COMPONENT_ID_MULTIPLE	1000000000 

}
#endif // KBE_COMMON_H
