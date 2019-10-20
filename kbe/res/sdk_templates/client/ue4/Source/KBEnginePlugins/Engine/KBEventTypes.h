#pragma once

#include "KBECommon.h"

namespace KBEngine
{

struct KBENGINEPLUGINS_API KBEventTypes
{
	// ------------------------------------账号相关------------------------------------

	// Create new account.
	// <para> param1(string): accountName</para>
	// <para> param2(string): password</para>
	// <para> param2(bytes): datas. // If you use third-party account system, the system may fill some of the third-party additional datas. </para>
	static const FString createAccount;

	// Create account feedback results.
	// <para> param1(uint16): retcode. // server_errors</para>
	// <para> param2(bytes): datas. // If you use third-party account system, the system may fill some of the third-party additional datas. </para>
	static const FString onCreateAccountResult;
	
	// Request server binding account Email.
	// <para> param1(string): emailAddress</para>
	static const FString bindAccountEmail;

	// Response from binding account Email request.
	// <para> param1(uint16): retcode. // server_errors</para>
	static const FString onBindAccountEmail;

	// Request to set up a new password for the account. Note: account must be online.
	// <para> param1(string): old_password</para>
	// <para> param2(string): new_password</para>
	static const FString newPassword;

	// Response from a new password request.
	// <para> param1(uint16): retcode. // server_errors</para>
	static const FString onNewPassword;

	// Request to reset password for the account. Note: account must be online.
	// <para> param1(string): username</para>
	static const FString resetPassword;

	// Response from a reset password request.
	// <para> param1(uint16): retcode. // server_errors</para>
	static const FString onResetPassword;

	// ------------------------------------连接相关------------------------------------

	// Kicked of the current server.
	// <para> param1(uint16): retcode. // server_errors</para>
	static const FString onKicked;

	// Disconnected from the server.
	static const FString onDisconnected;

	// Status of connection server.
	// <para> param1(bool): success or fail</para>
	static const FString onConnectionState;

	// ------------------------------------logon相关------------------------------------

	// Login to server.
	// <para> param1(string): accountName</para>
	// <para> param2(string): password</para>
	// <para> param3(bytes): datas // Datas by user defined. Data will be recorded into the KBE account database, you can access the datas through the script layer. If you use third-party account system, datas will be submitted to the third-party system.</para>
	static const FString login;

	// Relogin to baseapp.
	static const FString logout;

	// Relogin to baseapp.
	static const FString reloginBaseapp;

	// Engine version mismatch.
	// <para> param1(string): clientVersion
	// <para> param2(string): serverVersion
	static const FString onVersionNotMatch;

	// script version mismatch.
	// <para> param1(string): clientScriptVersion
	// <para> param2(string): serverScriptVersion
	static const FString onScriptVersionNotMatch;

	// Login failed.
	// <para> param1(uint16): retcode. // server_errors</para>
	static const FString onLoginFailed;

	// Login to baseapp.
	static const FString onLoginBaseapp;

	// Login baseapp failed.
	// <para> param1(uint16): retcode. // server_errors</para>
	static const FString onLoginBaseappFailed;

	// Relogin to baseapp.
	static const FString onReloginBaseapp;

	// Relogin baseapp success.
	static const FString onReloginBaseappSuccessfully;

	// Relogin baseapp failed.
	// <para> param1(uint16): retcode. // server_errors</para>
	static const FString onReloginBaseappFailed;

	// ------------------------------------实体cell相关事件------------------------------------

	// Entity enter the client-world.
	// <para> param1: Entity</para>
	static const FString onEnterWorld;

	// Entity leave the client-world.
	// <para> param1: Entity</para>
	static const FString onLeaveWorld;

	// Player enter the new space.
	// <para> param1: Entity</para>
	static const FString onEnterSpace;

	// Player leave the space.
	// <para> param1: Entity</para>
	static const FString onLeaveSpace;

	// Sets the current position of the entity.
	// <para> param1: Entity</para>
	static const FString set_position;

	// Sets the current direction of the entity.
	// <para> param1: Entity</para>
	static const FString set_direction;

	// The entity position is updated, you can smooth the moving entity to new location.
	// <para> param1: Entity</para>
	static const FString updatePosition;

	// The current space is specified by the geometry mapping.
	// Popular said is to load the specified Map Resources.
	// <para> param1(string): resPath</para>
	static const FString addSpaceGeometryMapping;

	// Server spaceData set data.
	// <para> param1(int32): spaceID</para>
	// <para> param2(string): key</para>
	// <para> param3(string): value</para>
	static const FString onSetSpaceData;

	// Start downloading data.
	// <para> param1(int32): rspaceID</para>
	// <para> param2(string): key</para>
	static const FString onDelSpaceData;

	// Triggered when the entity is controlled or out of control.
	// <para> param1: Entity</para>
	// <para> param2(bool): isControlled</para>
	static const FString onControlled;

	// Lose controlled entity.
	// <para> param1: Entity</para>
	static const FString onLoseControlledEntity;

	// ------------------------------------数据下载相关------------------------------------

	// Start downloading data.
	// <para> param1(uint16): resouce id</para>
	// <para> param2(uint32): data size</para>
	// <para> param3(string): description</para>
	static const FString onStreamDataStarted;

	// Receive data.
	// <para> param1(uint16): resouce id</para>
	// <para> param2(bytes): datas</para>
	static const FString onStreamDataRecv;

	// The downloaded data is completed.
	// <para> param1(uint16): resouce id</para>
	static const FString onStreamDataCompleted;

	// ------------------------------------SDK更新相关-----------------------------------
	static const FString onImportClientSDKSuccessfully;
	static const FString onDownloadSDK;

};

}