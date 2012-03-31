/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __KBENGINE_DEF_COMMON_H__
#define __KBENGINE_DEF_COMMON_H__
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)

// common include	
#include "cstdkbe/cstdkbe.hpp"
//#define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>	
#include <map>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
#define LIB_DLLAPI  __declspec(dllexport)

#ifdef __cplusplus  
extern "C" {  
#endif  

#ifdef __cplusplus  
}
#endif 

namespace KBEngine{

/** entity的数据传输特性标记 */
enum EntityDataFlags
{
	ED_FLAG_CELL_PUBLIC												= 0x00000000, // 相关所有cell广播
	ED_FLAG_CELL_PRIVATE											= 0x00000001, // 当前cell
	ED_FLAG_ALL_CLIENTS												= 0x00000002, // cell广播与所有客户端
	ED_FLAG_CELL_PUBLIC_AND_OWN										= 0x00000004, // cell广播与自己的客户端
	ED_FLAG_OWN_CLIENT												= 0x00000008, // 当前cell和客户端
	ED_FLAG_BASE_AND_CLIENT											= 0x00000010, // base和客户端
	ED_FLAG_BASE													= 0x00000020, // 当前base
	ED_FLAG_OTHER_CLIENTS											= 0x00000040, // cell广播和其他客户端
};

/** 相当于对entity数据传输类别的一个总体的定义 */
enum EntityDataFlagRelation
{
	// 所有与baseapp有关系的标志
	ENTITY_BASE_DATA_FLAGS											= ED_FLAG_BASE | ED_FLAG_BASE_AND_CLIENT,
	// 所有与cellapp有关系的标志
	ENTITY_CELL_DATA_FLAGS											= ED_FLAG_CELL_PUBLIC | ED_FLAG_CELL_PRIVATE | ED_FLAG_ALL_CLIENTS | ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OTHER_CLIENTS | ED_FLAG_OWN_CLIENT,
	// 所有与client有关系的标志
	ENTITY_CLIENT_DATA_FLAGS										= ED_FLAG_BASE_AND_CLIENT | ED_FLAG_ALL_CLIENTS | ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OTHER_CLIENTS | ED_FLAG_OWN_CLIENT,
	// 所有需要广播给其他cellapp的标志
	ENTITY_BROADCAST_CELL_FLAGS										= ED_FLAG_CELL_PUBLIC | ED_FLAG_ALL_CLIENTS | ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OTHER_CLIENTS,
	// 所有需要广播给其他客户端(不包括自己的)的标志
	ENTITY_BROADCAST_OTHER_CLIENT_FLAGS								= ED_FLAG_OTHER_CLIENTS | ED_FLAG_ALL_CLIENTS,
	// 所有需要广播给自己的客户端的标志
	ENTITY_BROADCAST_OWN_CLIENT_FLAGS								= ED_FLAG_ALL_CLIENTS | ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OWN_CLIENT | ED_FLAG_BASE_AND_CLIENT,
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

/** mailbox类别所对应的组件类别映射，  这个表的索引个严格匹配ENTITY_MAILBOX_TYPE的值 */
const COMPONENT_TYPE ENTITY_MAILBOX_COMPONENT_TYPE_MAPPING[] = 
{
	CELLAPP_TYPE,
	BASEAPP_TYPE,
	CLIENT_TYPE,
	BASEAPP_TYPE,
	CELLAPP_TYPE,
	CELLAPP_TYPE,
	BASEAPP_TYPE,
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

/** mailbox 所投递的mail类别 */
#define MAIL_TYPE_REMOTE_CALL										0x00000001	// 远程呼叫一个方法
#define MAIL_TYPE_LOST_VIEW_ENTITY									0x00000002	// 当前这个entity视野范围内的一个entity丢失了
#define MAIL_TYPE_ENTITY_MOVE_TO_POINT								0x00000003	// 当前这个entity视野范围内的一个entity移动到某个点
#define MAIL_TYPE_ENTER_SPACE										0x00000004	// entity进入一个新的space
#define MAIL_TYPE_LEAVE_SPACE										0x00000005	// entity将要离开当前space
#define MAIL_TYPE_UPDATE_PROPERTYS									0x00000006	// 更新某个entity的一些属性到客户端
#define MAIL_TYPE_UPDATE_PROPERTY									0x00000007	// 更新某个entity的某个属性到客户端
#define MAIL_TYPE_SEEK												0x00000008	// 某个player的客户端请求移动player


typedef std::map<std::string, EntityDataFlags> ENTITYFLAGMAP;
extern ENTITYFLAGMAP g_entityFlagMapping;										// entity 的flag字符串映射表
}
#endif
