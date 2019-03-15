// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#if defined(DEFINE_IN_INTERFACE)
	#undef KBE_CLIENT_INTERFACE_H
#endif


#ifndef KBE_CLIENT_INTERFACE_H
#define KBE_CLIENT_INTERFACE_H

// common include	
#if defined(CLIENT)
#include "clientapp.h"
#endif
#include "client_interface_macros.h"
#include "network/interface_defs.h"
#include "server/server_errors.h"
#include "entitydef/common.h"
#include "common.h"
	
namespace KBEngine{

/**
	CLIENT所有消息接口在此定义
*/
NETWORK_INTERFACE_DECLARE_BEGIN(ClientInterface)
	// 服务端hello返回。
	CLIENT_MESSAGE_DECLARE_STREAM(onHelloCB,								NETWORK_VARIABLE_MESSAGE)

	// 和服务端的版本不匹配
	CLIENT_MESSAGE_DECLARE_STREAM(onVersionNotMatch,						NETWORK_VARIABLE_MESSAGE)

	// 和服务端的脚本层版本不匹配
	CLIENT_MESSAGE_DECLARE_STREAM(onScriptVersionNotMatch,					NETWORK_VARIABLE_MESSAGE)

	// 创建账号失败。
	CLIENT_MESSAGE_DECLARE_STREAM(onCreateAccountResult,					NETWORK_VARIABLE_MESSAGE)

	// 登录成功。
	CLIENT_MESSAGE_DECLARE_STREAM(onLoginSuccessfully,						NETWORK_VARIABLE_MESSAGE)

	// 登录失败。
	CLIENT_MESSAGE_DECLARE_STREAM(onLoginFailed,							NETWORK_VARIABLE_MESSAGE)

	// 服务器端已经创建了一个与客户端关联的代理Entity || 登录网关成功。
	CLIENT_MESSAGE_DECLARE_ARGS3(onCreatedProxies,							NETWORK_VARIABLE_MESSAGE,
								uint64,										rndUUID,
								ENTITY_ID,									eid,
								std::string,								entityType)

	// 登录网关失败。
	CLIENT_MESSAGE_DECLARE_ARGS1(onLoginBaseappFailed,						NETWORK_FIXED_MESSAGE,
								SERVER_ERROR_CODE,							failedcode)

	// 登录网关失败。
	CLIENT_MESSAGE_DECLARE_ARGS1(onReloginBaseappFailed,					NETWORK_FIXED_MESSAGE,
								SERVER_ERROR_CODE,							failedcode)

	// 服务器上的entity已经进入游戏世界了。
	CLIENT_MESSAGE_DECLARE_STREAM(onEntityEnterWorld,						NETWORK_VARIABLE_MESSAGE)

	// 服务器上的entity已经离开游戏世界了。
	CLIENT_MESSAGE_DECLARE_ARGS1(onEntityLeaveWorld,						NETWORK_FIXED_MESSAGE,
								ENTITY_ID,									eid)

	// 服务器上的entity已经离开游戏世界了。
	CLIENT_MESSAGE_DECLARE_STREAM(onEntityLeaveWorldOptimized,				NETWORK_VARIABLE_MESSAGE)

	// 告诉客户端某个entity销毁了， 此类entity通常是还未onEntityEnterWorld。
	CLIENT_MESSAGE_DECLARE_ARGS1(onEntityDestroyed,							NETWORK_FIXED_MESSAGE,
								ENTITY_ID,									eid)

	// 服务器上的entity已经进入space了。
	CLIENT_MESSAGE_DECLARE_STREAM(onEntityEnterSpace,						NETWORK_VARIABLE_MESSAGE)

	// 服务器上的entity已经离开space了。
	CLIENT_MESSAGE_DECLARE_ARGS1(onEntityLeaveSpace,						NETWORK_FIXED_MESSAGE,
								ENTITY_ID,									eid)

	// 远程呼叫entity方法
	CLIENT_MESSAGE_DECLARE_STREAM(onRemoteMethodCall,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onRemoteMethodCallOptimized,				NETWORK_VARIABLE_MESSAGE)

	// 被踢出服务器
	CLIENT_MESSAGE_DECLARE_ARGS1(onKicked,									NETWORK_FIXED_MESSAGE,
								SERVER_ERROR_CODE,							failedcode)

	// 服务器更新entity属性
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdatePropertys,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdatePropertysOptimized,				NETWORK_VARIABLE_MESSAGE)

	// 服务器强制设置entity的位置与朝向
	CLIENT_MESSAGE_DECLARE_STREAM(onSetEntityPosAndDir,						NETWORK_VARIABLE_MESSAGE)

	// 服务器更新包
	CLIENT_MESSAGE_DECLARE_ARGS3(onUpdateBasePos,							NETWORK_FIXED_MESSAGE,
								float,										x,
								float,										y,
								float,										z)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateBaseDir,							NETWORK_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_ARGS2(onUpdateBasePosXZ,							NETWORK_FIXED_MESSAGE,
								float,										x,
								float,										z)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData,								NETWORK_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_ypr,							NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_yp,							NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_yr,							NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_pr,							NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_y,							NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_p,							NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_r,							NETWORK_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz,							NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_ypr,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_yp,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_yr,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_pr,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_y,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_p,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_r,						NETWORK_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz,							NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_ypr,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_yp,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_yr,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_pr,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_y,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_p,						NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_r,						NETWORK_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_ypr_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_yp_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_yr_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_pr_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_y_optimized,					NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_p_optimized,					NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_r_optimized,					NETWORK_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_ypr_optimized,			NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_yp_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_yr_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_pr_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_y_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_p_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xz_r_optimized,				NETWORK_VARIABLE_MESSAGE)

	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_ypr_optimized,			NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_yp_optimized,			NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_yr_optimized,			NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_pr_optimized,			NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_y_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_p_optimized,				NETWORK_VARIABLE_MESSAGE)
	CLIENT_MESSAGE_DECLARE_STREAM(onUpdateData_xyz_r_optimized,				NETWORK_VARIABLE_MESSAGE)

	// download stream开始了 
	CLIENT_MESSAGE_DECLARE_ARGS3(onStreamDataStarted,						NETWORK_VARIABLE_MESSAGE,
								int16,										id,
								uint32,										datasize,
								std::string,								descr)

	// 接收到streamData
	CLIENT_MESSAGE_DECLARE_STREAM(onStreamDataRecv,							NETWORK_VARIABLE_MESSAGE)

	// download stream完成了 
	CLIENT_MESSAGE_DECLARE_ARGS1(onStreamDataCompleted,						NETWORK_FIXED_MESSAGE,
								int16,										id)

	// 导入协议
	CLIENT_MESSAGE_DECLARE_STREAM(onImportClientMessages,					NETWORK_VARIABLE_MESSAGE)
	
	// 导入entitydef
	CLIENT_MESSAGE_DECLARE_STREAM(onImportClientEntityDef,					NETWORK_VARIABLE_MESSAGE)

	// 错误码描述导出
	CLIENT_MESSAGE_DECLARE_STREAM(onImportServerErrorsDescr,				NETWORK_VARIABLE_MESSAGE)

	// 接收导入sdk消息
	CLIENT_MESSAGE_DECLARE_STREAM(onImportClientSDK,						NETWORK_VARIABLE_MESSAGE)

	// 服务端初始化spacedata
	CLIENT_MESSAGE_DECLARE_STREAM(initSpaceData,							NETWORK_VARIABLE_MESSAGE)

	// 服务端设置了spacedata
	CLIENT_MESSAGE_DECLARE_ARGS3(setSpaceData,								NETWORK_VARIABLE_MESSAGE,
								SPACE_ID,									spaceID,
								std::string,								key,
								std::string,								val)

	// 服务端删除了spacedata
	CLIENT_MESSAGE_DECLARE_ARGS2(delSpaceData,								NETWORK_VARIABLE_MESSAGE,
								SPACE_ID,									spaceID,
								std::string,								key)

	// 重置账号密码请求返回
	CLIENT_MESSAGE_DECLARE_ARGS1(onReqAccountResetPasswordCB,				NETWORK_FIXED_MESSAGE,
								SERVER_ERROR_CODE,							failedcode)

	// 重置账号密码请求返回
	CLIENT_MESSAGE_DECLARE_ARGS1(onReqAccountBindEmailCB,					NETWORK_FIXED_MESSAGE,
								SERVER_ERROR_CODE,							failedcode)

	// 重置账号密码请求返回
	CLIENT_MESSAGE_DECLARE_ARGS1(onReqAccountNewPasswordCB,					NETWORK_FIXED_MESSAGE,
								SERVER_ERROR_CODE,							failedcode)

	// 重登陆网关成功 
	CLIENT_MESSAGE_DECLARE_STREAM(onReloginBaseappSuccessfully,				NETWORK_VARIABLE_MESSAGE)
									
	// 告诉客户端：你当前负责（或取消）控制谁的位移同步
	CLIENT_MESSAGE_DECLARE_ARGS2(onControlEntity,							NETWORK_FIXED_MESSAGE,
									ENTITY_ID,								eid,
									int8,									isControlled)

	// 服务器心跳回调
	CLIENT_MESSAGE_DECLARE_ARGS0(onAppActiveTickCB,							NETWORK_FIXED_MESSAGE)

	NETWORK_INTERFACE_DECLARE_END()

#ifdef DEFINE_IN_INTERFACE
	#undef DEFINE_IN_INTERFACE
#endif

}

#endif // KBE_CLIENT_INTERFACE_H
