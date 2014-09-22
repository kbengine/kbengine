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

#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_CLIENT_INTERFACE_HPP
#endif


#ifndef KBE_CLIENT_INTERFACE_HPP
#define KBE_CLIENT_INTERFACE_HPP

// common include	
#if defined(CLIENT)
#include "clientapp.hpp"
#endif
#include "client_interface_macros.hpp"
#include "network/interface_defs.hpp"
#include "server/server_errors.hpp"
#include "entitydef/common.hpp"
#include "common.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

/**
	CLIENT所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(ClientInterface)
	// 服务端hello返回。
	CLIENT_MESSAGE_DECLARE_STREAM(onHelloCB,								MERCURY_VARIABLE_MESSAGE)

	// 和服务端的版本不匹配
	CLIENT_MESSAGE_DECLARE_STREAM(onVersionNotMatch,						MERCURY_VARIABLE_MESSAGE)

	// 和服务端的脚本层版本不匹配
	CLIENT_MESSAGE_DECLARE_STREAM(onScriptVersionNotMatch,					MERCURY_VARIABLE_MESSAGE)

	// 创建账号失败。
	CLIENT_MESSAGE_DECLARE_STREAM(onCreateAccountResult,					MERCURY_VARIABLE_MESSAGE)

	// 登录成功。
	CLIENT_MESSAGE_DECLARE_STREAM(onLoginSuccessfully,						MERCURY_VARIABLE_MESSAGE)

	// 登录失败。
	CLIENT_MESSAGE_DECLARE_STREAM(onLoginFailed,							MERCURY_VARIABLE_MESSAGE)

	// 服务器端已经创建了一个与客户端关联的代理Entity || 登录网关成功。
	CLIENT_MESSAGE_DECLARE_ARGS3(onCreatedProxies,							MERCURY_VARIABLE_MESSAGE,
									uint64,									rndUUID,
									ENTITY_ID,								eid,
									std::string,							entityType)

	// 登录网关失败。
	CLIENT_MESSAGE_DECLARE_ARGS1(onLoginGatewayFailed,						MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// 登录网关失败。
	CLIENT_MESSAGE_DECLARE_ARGS1(onReLoginGatewayFailed,					MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// 服务器上的entity已经进入游戏世界了。
	CLIENT_MESSAGE_DECLARE_STREAM(onEntityEnterWorld,						MERCURY_VARIABLE_MESSAGE)

	// 服务器上的entity已经离开游戏世界了。
	CLIENT_MESSAGE_DECLARE_ARGS1(onEntityLeaveWorld,						MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid)

	// 服务器上的entity已经离开游戏世界了。
	CLIENT_MESSAGE_DECLARE_STREAM(onEntityLeaveWorldOptimized,				MERCURY_VARIABLE_MESSAGE)

	// 告诉客户端某个entity销毁了， 此类entity通常是还未onEntityEnterWorld。
	CLIENT_MESSAGE_DECLARE_ARGS1(onEntityDestroyed,							MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid)

	// 服务器上的entity已经进入space了。
	CLIENT_MESSAGE_DECLARE_STREAM(onEntityEnterSpace,						MERCURY_VARIABLE_MESSAGE)

	// 服务器上的entity已经离开space了。
	CLIENT_MESSAGE_DECLARE_ARGS1(onEntityLeaveSpace,						MERCURY_FIXED_MESSAGE,
									ENTITY_ID,								eid)

	// 远程呼叫entity方法
	CLIENT_MESSAGE_DECLARE_STREAM(onRemoteMethodCall,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onRemoteMethodCallOptimized,				MERCURY_VARIABLE_MESSAGE)

	// 被踢出服务器
	CLIENT_MESSAGE_DECLARE_ARGS1(onKicked,									MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// 服务器更新entity属性
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdatePropertys,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdatePropertysOptimized,				MERCURY_VARIABLE_MESSAGE)

	// 服务器强制设置entity的位置与朝向
	CLIENT_MESSAGE_DECLARE_STREAM(onSetEntityPosAndDir,						MERCURY_VARIABLE_MESSAGE)

	// 服务器更新包
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateBasePos,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateBasePosXZ,						MERCURY_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData,								MERCURY_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_ypr,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_yp,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_yr,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_pr,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_y,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_p,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_r,							MERCURY_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_ypr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_yp,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_yr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_pr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_y,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_p,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_r,						MERCURY_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz,							MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_ypr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_yp,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_yr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_pr,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_y,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_p,						MERCURY_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_r,						MERCURY_VARIABLE_MESSAGE)

	// download stream开始了 
	CLIENT_MESSAGE_DECLARE_ARGS3(onStreamDataStarted,						MERCURY_VARIABLE_MESSAGE,
									int16,									id,
									uint32,									datasize,
									std::string,							descr)

	// 接收到streamData
	CLIENT_MESSAGE_DECLARE_STREAM(onStreamDataRecv,							MERCURY_VARIABLE_MESSAGE)

	// download stream完成了 
	CLIENT_MESSAGE_DECLARE_ARGS1(onStreamDataCompleted,						MERCURY_FIXED_MESSAGE,
									int16,									id)

	// 导入协议
	CLIENT_MESSAGE_DECLARE_STREAM(onImportClientMessages,					MERCURY_VARIABLE_MESSAGE)
	
	// 导入entitydef
	CLIENT_MESSAGE_DECLARE_STREAM(onImportClientEntityDef,					MERCURY_VARIABLE_MESSAGE)

	// 错误码描述导出
	CLIENT_MESSAGE_DECLARE_STREAM(onImportServerErrorsDescr,				MERCURY_VARIABLE_MESSAGE)

	// 服务端初始化spacedata
	CLIENT_MESSAGE_DECLARE_STREAM(initSpaceData,							MERCURY_VARIABLE_MESSAGE)

	// 服务端设置了spacedata
	CLIENT_MESSAGE_DECLARE_ARGS3(setSpaceData,								MERCURY_VARIABLE_MESSAGE,
									SPACE_ID,								spaceID,
									std::string,							key,
									std::string,							valye)

	// 服务端删除了spacedata
	CLIENT_MESSAGE_DECLARE_ARGS2(delSpaceData,								MERCURY_VARIABLE_MESSAGE,
									SPACE_ID,								spaceID,
									std::string,							key)

	// 重置账号密码请求返回
	CLIENT_MESSAGE_DECLARE_ARGS1(onReqAccountResetPasswordCB,				MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// 重置账号密码请求返回
	CLIENT_MESSAGE_DECLARE_ARGS1(onReqAccountBindEmailCB,					MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// 重置账号密码请求返回
	CLIENT_MESSAGE_DECLARE_ARGS1(onReqAccountNewPasswordCB,					MERCURY_FIXED_MESSAGE,
									SERVER_ERROR_CODE,						failedcode)

	// 重登陆网关成功 
	CLIENT_MESSAGE_DECLARE_STREAM(onReLoginGatewaySuccessfully,				MERCURY_VARIABLE_MESSAGE)
									
NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}

#endif // KBE_CLIENT_INTERFACE_HPP
