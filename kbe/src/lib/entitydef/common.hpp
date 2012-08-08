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


#ifndef __KBENGINE_DEF_COMMON_H__
#define __KBENGINE_DEF_COMMON_H__
#include "cstdkbe/cstdkbe.hpp"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif
// common include	
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

/** 属性的lod广播级别范围的定义 */
#define DETAIL_LEVEL_NEAR													0	// lod级别：近						
#define DETAIL_LEVEL_MEDIUM													1	// lod级别：中
#define DETAIL_LEVEL_FAR													2	// lod级别：远	
#define DETAIL_LEVEL_UNKNOW													3	// lod级别：非常的远 (通常在这个级别内的entity不会对他广播任何属性（不包括位置方向等）)	

typedef std::map<std::string, EntityDataFlags> ENTITYFLAGMAP;
extern ENTITYFLAGMAP g_entityFlagMapping;										// entity 的flag字符串映射表

// 属性和方法的UID类别
typedef uint16 ENTITY_PROPERTY_UID;
typedef uint16 ENTITY_METHOD_UID;

// 对entity的一些系统级别的可变属性进行编号以便网络传输时进行辨别
enum ENTITY_BASE_PROPERTY_UTYPE
{
	ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ					= 1,
	ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW		= 2,
	ENTITY_BASE_PROPERTY_UTYPE_SPACEID						= 3,
};

}
#endif
