/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __OPCODES_H__
#define __OPCODES_H__

// common include
#include "cstdkbe/cstdkbe.hpp"
// #define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <iostream>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine{
	
/** 操作码类别 */
typedef	uint16 OPCODE_TYPE;

/** 操作码标志 */
#define OP_FLAG_UNKNOWN											0x00000001						// 未知或者不支持的操作码
#define OP_FLAG_NORMAL											0x00000002						// 正常使用的操作码
#define OP_FLAG_URGENT											0x00000004						// 紧急操作 （要求立刻执行）

/** 网络操作码 */
enum Opcodes
{
	OP_UNKNOWN													= 0x000,						// 未知或者不支持操作
	OP_ENGINE_COMPONENT_HANDSHAKE								= 0x001,						// 引擎握手操作
	OP_SCRIPT_EXEC_COMMAND										= 0x002,						// 执行脚本命令
	OP_BROADCAST_GLOBALDATA_CHANGE								= 0x003,						// 广播globalData的改变
	OP_BROADCAST_GLOBALBASES_CHANGE								= 0x004,						// 广播globalBases的改变
	OP_BROADCAST_CELLAPPDATA_CHANGE								= 0x005,						// 广播cellAppData的改变
	OP_CELLAPP_BROADCAST_MSG									= 0x006,						// cellapp消息广播
	OP_REQUEST_ID_RANGE_ALLOC									= 0x007,						// 请求ID数据段的分配
	OP_INIT_APP													= 0x008,						// baeappmgr初始化baseapp 或者 cellapp
	OP_CREATE_IN_NEW_SPACE										= 0x009,						// 一个entity请求创建在一个新的space中
	OP_ENTITY_CELL_CREATE_COMPLETE								= 0x010,						// 一个entity的cell部分实体创建完成
	OP_CELLAPP_OR_BASEAPP_STARTUP								= 0x011,						// 一个新的cellapp或者baseapp组件启动了 广播给其他baseapp和cellapp
	OP_CREATE_BASE_ANY_WHERE									= 0x012,						// baseapp的createBaseAnywhere相关操作
	OP_CREATE_BASE_ANY_WHERE_CALLBACK							= 0x013,						// baseapp的createBaseAnywhere的回调
	OP_CREATE_CELL_ENTITY										= 0x014,						// createCellEntity的操作包
	OP_CLIENT_LOGIN												= 0x015,						// 帐号登陆操作
	OP_QUERY_ACCOUNT_PASSWORD									= 0x016,						// dbmgr查询到了密码返回给loginapp
	OP_UPDATE_COMPONENT_STATE									= 0x017,						// 某个组件向另一个组件更新它自身的状态信息 (比如：服务器上的人数)
	OP_ALLOC_CLIENT_TO_BASEAPP									= 0x018,						// 分配一个客户端给baseapp
	OP_CREATE_CLIENT_ENTITY										= 0x019,						// 在某个客户端上创建一个entity
	OP_CREATE_CLIENT_PROXY										= 0x020,						// 在某个客户端上创建一个ProxyEntity
	OP_DESTROY_CLIENT_ENTITY									= 0x021,						// 在某个客户端上销毁一个Entity
	OP_ENTITY_ENTER_WORLD										= 0x022,						// 一个entity进入了游戏(cell实体进入了世界)
	OP_ENTITY_LOSE_CELL											= 0x023,						// 一个entity的cell部分丢失(或者说是被销毁了 通知base部分)
	OP_ENTITY_DESTROY_CELL_ENTITY								= 0x024,						// 销毁一个entity的cell
	OP_ENTITY_CLIENT_GET_CELL									= 0x025,						// 告诉baseentity,entity的客户端获得了cell
	OP_ENTITY_MAIL												= 0x026,						// 一个邮件数据包, 它就是一个邮寄的socketPacket 主要是mailbox机制使用
	OP_NTP														= 0x027,						// 游戏时间同步协议
	OP_DELTATIME_TEST											= 0x028,						// 网络延时测试
	OP_CLIENT_SET_DELTATIME										= 0x029,						// 客户端设置自己的网络延时(客户端已经和服务器进行了相关测试)
	OP_TYPE_TOTAL_NUMBER										= 0x030							// 注意:这个始终确保在最后一个
};

/** 帐号登陆相关操作码 */
#define ALOPC_SUCCESS											0x00000000						// 帐号登陆成功
#define ALOPC_ACCOUNT_OR_PASSWORD_ERROR							0x00000001						// 帐号或者密码错误




}
#endif
