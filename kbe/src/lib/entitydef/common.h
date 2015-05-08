﻿/*
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


#ifndef KBENGINE_DEF_COMMON_H
#define KBENGINE_DEF_COMMON_H

#include "common/common.h"
#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4910)
#pragma warning (disable : 4251)
#endif


namespace KBEngine{

/** entity的数据传输特性标记 */
enum EntityDataFlags
{
	ED_FLAG_UNKOWN													= 0x00000000, // 未定义
	ED_FLAG_CELL_PUBLIC												= 0x00000001, // 相关所有cell广播
	ED_FLAG_CELL_PRIVATE											= 0x00000002, // 当前cell
	ED_FLAG_ALL_CLIENTS												= 0x00000004, // cell广播与所有客户端
	ED_FLAG_CELL_PUBLIC_AND_OWN										= 0x00000008, // cell广播与自己的客户端
	ED_FLAG_OWN_CLIENT												= 0x00000010, // 当前cell和客户端
	ED_FLAG_BASE_AND_CLIENT											= 0x00000020, // base和客户端
	ED_FLAG_BASE													= 0x00000040, // 当前base
	ED_FLAG_OTHER_CLIENTS											= 0x00000080, // cell广播和其他客户端
};

std::string entityDataFlagsToString(uint32 flags);

#define ED_FLAG_ALL  ED_FLAG_CELL_PUBLIC | ED_FLAG_CELL_PRIVATE | ED_FLAG_ALL_CLIENTS \
	| ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OWN_CLIENT |	\
	ED_FLAG_BASE_AND_CLIENT | ED_FLAG_BASE | ED_FLAG_OTHER_CLIENTS

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

/** 属性的lod广播级别范围的定义 */
typedef uint8 DETAIL_TYPE;
#define DETAIL_LEVEL_NEAR													0	// lod级别：近						
#define DETAIL_LEVEL_MEDIUM													1	// lod级别：中
#define DETAIL_LEVEL_FAR													2	// lod级别：远	

typedef std::map<std::string, EntityDataFlags> ENTITYFLAGMAP;
/// entity 的Flags字符串到值映射表
/// 
/// \code
/// g_entityFlagMapping["CELL_PUBLIC"]			= ED_FLAG_CELL_PUBLIC;
/// g_entityFlagMapping["CELL_PRIVATE"]         = ED_FLAG_CELL_PRIVATE;
/// g_entityFlagMapping["ALL_CLIENTS"]          = ED_FLAG_ALL_CLIENTS;
/// g_entityFlagMapping["CELL_PUBLIC_AND_OWN"]  = ED_FLAG_CELL_PUBLIC_AND_OWN;
/// g_entityFlagMapping["BASE_AND_CLIENT"]      = ED_FLAG_BASE_AND_CLIENT;
/// g_entityFlagMapping["BASE"]                 = ED_FLAG_BASE;
/// g_entityFlagMapping["OTHER_CLIENTS"]        = ED_FLAG_OTHER_CLIENTS;
/// g_entityFlagMapping["OWN_CLIENT"]           = ED_FLAG_OWN_CLIENT;
/// \endcode
extern ENTITYFLAGMAP g_entityFlagMapping;

// 属性和方法的UID类别
typedef uint16 ENTITY_PROPERTY_UID;
typedef uint16 ENTITY_METHOD_UID;
typedef uint16 ENTITY_SCRIPT_UID;
typedef uint16 DATATYPE_UID;
typedef uint8  DATATYPE;
typedef uint8  ENTITY_DEF_ALIASID;

#define DATA_TYPE_UNKONWN		0
#define DATA_TYPE_FIXEDARRAY	1
#define DATA_TYPE_FIXEDDICT		2
#define DATA_TYPE_STRING		3
#define DATA_TYPE_DIGIT			4
#define DATA_TYPE_BLOB			5
#define DATA_TYPE_PYTHON		6
#define DATA_TYPE_VECTOR		7
#define DATA_TYPE_UNICODE		8
#define DATA_TYPE_MAILBOX		9
#define DATA_TYPE_PYDICT		10
#define DATA_TYPE_PYTUPLE		11
#define DATA_TYPE_PYLIST		12

// 对entity的一些系统级别的可变属性进行编号以便网络传输时进行辨别
enum ENTITY_BASE_PROPERTY_UTYPE
{
	ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ					= 1,
	ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW		= 2,
	ENTITY_BASE_PROPERTY_UTYPE_SPACEID						= 3,
};

// 对entity的一些系统级别的可变属性进行编号以便网络传输时进行辨别
enum ENTITY_BASE_PROPERTY_ALIASID
{
	ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ				= 0,
	ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW	= 1,
	ENTITY_BASE_PROPERTY_ALIASID_SPACEID					= 2,
	ENTITY_BASE_PROPERTY_ALIASID_MAX						= 3,
};

// 被限制的系统属性，def中不允许定义
const char ENTITY_LIMITED_PROPERTYS[][32] =
{
	"id",
	"position",
	"direction",
	"spaceID",
	"autoLoad",
	"cell",
	"base",
	"client",
	"cell",
	"className",
	"databaseID",
	"isDestroyed",
	"shouldAutoArchive",
	"shouldAutoBackup",
	"__ACCOUNT_NAME__",
	"__ACCOUNT_PASSWORD__",
	"clientAddr",
	"entitiesEnabled",
	"hasClient",
	"roundTripTime",
	"timeSinceHeardFromClient",
	"allClients",
	"hasWitness",
	"isWitnessed",
	"otherClients",
	"topSpeed",
	"topSpeedY",
	"",
};

}
#endif // KBENGINE_DEF_COMMON_H

