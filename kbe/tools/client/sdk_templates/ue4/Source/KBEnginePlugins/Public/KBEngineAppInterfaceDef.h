// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.



#include "KBECommon.h"
#include "NetworkInterfaceDef.h"


NETWORK_INTERFACE_DECLARE_BEGIN(KBEngineApp)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onHelloCB,									NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onScriptVersionNotMatch,						NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onVersionNotMatch,							NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onImportClientMessages,						NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onImportServerErrorsDescr,					NETWORK_VARIABLE_MESSAGE)
	
	NETWORK_MESSAGE_HANDLER_ARGS0(Client_onAppActiveTickCB,								NETWORK_FIXED_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onLoginFailed,								NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_ARGS1(Client_onLoginBaseappFailed,							NETWORK_FIXED_MESSAGE,
									uint16,												failedcode)

	NETWORK_MESSAGE_HANDLER_ARGS1(Client_onReloginBaseappFailed,						NETWORK_FIXED_MESSAGE,
									uint16,												failedcode)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onReloginBaseappSuccessfully,					NETWORK_VARIABLE_MESSAGE)
	
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onLoginSuccessfully,							NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onImportClientEntityDef,						NETWORK_VARIABLE_MESSAGE)
	
	NETWORK_MESSAGE_HANDLER_ARGS1(Client_onKicked,										NETWORK_FIXED_MESSAGE,
									uint16,												failedcode)

	NETWORK_MESSAGE_HANDLER_ARGS1(Client_onEntityDestroyed,								NETWORK_FIXED_MESSAGE,
									int32,												eid)

	NETWORK_MESSAGE_HANDLER_ARGS3(Client_onCreatedProxies,								NETWORK_VARIABLE_MESSAGE,
									uint64,												rndUUID,
									int32,												eid,
									FString,											entityType)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdatePropertysOptimized,					NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdatePropertys,							NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onRemoteMethodCallOptimized,					NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onRemoteMethodCall,							NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onCreateAccountResult,						NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_ARGS1(Client_onReqAccountResetPasswordCB,					NETWORK_FIXED_MESSAGE,
									uint16,												failedcode)

	NETWORK_MESSAGE_HANDLER_ARGS1(Client_onReqAccountBindEmailCB,						NETWORK_FIXED_MESSAGE,
									uint16,												failedcode)

	NETWORK_MESSAGE_HANDLER_ARGS1(Client_onReqAccountNewPasswordCB,						NETWORK_FIXED_MESSAGE,
									uint16,												failedcode)

	NETWORK_MESSAGE_HANDLER_ARGS2(Client_onControlEntity,								NETWORK_FIXED_MESSAGE,
									ENTITY_ID,											eid,
									int8,												isControlled)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_initSpaceData,								NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_ARGS3(Client_setSpaceData,									NETWORK_VARIABLE_MESSAGE,
									uint32,												spaceID,
									FString,											key,
									FString,											value)

	NETWORK_MESSAGE_HANDLER_ARGS2(Client_delSpaceData,									NETWORK_VARIABLE_MESSAGE,
									uint32,												spaceID,
									FString,											key)

	NETWORK_MESSAGE_HANDLER_ARGS3(Client_onStreamDataStarted,							NETWORK_VARIABLE_MESSAGE,
									int16,												_id,
									uint32,												datasize,
									FString,											descr)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onStreamDataRecv,								NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_ARGS1(Client_onStreamDataCompleted,							NETWORK_FIXED_MESSAGE,
									int16,												_id)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onEntityEnterWorld,							NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onEntityLeaveWorldOptimized,					NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_ARGS1(Client_onEntityLeaveWorld,							NETWORK_FIXED_MESSAGE,
									ENTITY_ID,											eid)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onEntityEnterSpace,							NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_ARGS1(Client_onEntityLeaveSpace,							NETWORK_FIXED_MESSAGE,
									ENTITY_ID,											eid)

	NETWORK_MESSAGE_HANDLER_ARGS3(Client_onUpdateBasePos,								NETWORK_FIXED_MESSAGE,
									float,												x,
									float,												y,
									float,												z)

	NETWORK_MESSAGE_HANDLER_ARGS2(Client_onUpdateBasePosXZ,								NETWORK_FIXED_MESSAGE,
									float,												x,
									float,												z)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateBaseDir,								NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData,									NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onSetEntityPosAndDir,							NETWORK_VARIABLE_MESSAGE)

	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_ypr,								NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_yp,								NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_yr,								NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_pr,								NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_y,								NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_p,								NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_r,								NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xz,								NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xz_ypr,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xz_yp,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xz_yr,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xz_pr,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xz_y,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xz_p,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xz_r,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xyz,								NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xyz_ypr,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xyz_yp,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xyz_yr,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xyz_pr,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xyz_y,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xyz_p,							NETWORK_VARIABLE_MESSAGE)
	NETWORK_MESSAGE_HANDLER_STREAM(Client_onUpdateData_xyz_r,							NETWORK_VARIABLE_MESSAGE)

NETWORK_INTERFACE_DECLARE_END()

