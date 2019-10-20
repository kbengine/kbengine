#include "KBEventTypes.h"

namespace KBEngine
{

// ------------------------------------账号相关------------------------------------
const FString KBEventTypes::createAccount = "createAccount";
const FString KBEventTypes::onCreateAccountResult = "onCreateAccountResult";
const FString KBEventTypes::bindAccountEmail = "bindAccountEmail";
const FString KBEventTypes::onBindAccountEmail = "onBindAccountEmail";
const FString KBEventTypes::newPassword = "newPassword";
const FString KBEventTypes::onNewPassword = "onNewPassword";
const FString KBEventTypes::resetPassword = "resetPassword";
const FString KBEventTypes::onResetPassword = "onResetPassword";

// ------------------------------------连接相关------------------------------------
const FString KBEventTypes::onKicked = "onKicked";
const FString KBEventTypes::onDisconnected = "onDisconnected";
const FString KBEventTypes::onConnectionState = "onConnectionState";

// ------------------------------------logon相关------------------------------------
const FString KBEventTypes::login = "login";
const FString KBEventTypes::logout = "logout";
const FString KBEventTypes::reloginBaseapp = "reloginBaseapp";
const FString KBEventTypes::onVersionNotMatch = "onVersionNotMatch";
const FString KBEventTypes::onScriptVersionNotMatch = "onScriptVersionNotMatch";
const FString KBEventTypes::onLoginFailed = "onLoginFailed";
const FString KBEventTypes::onLoginBaseapp = "onLoginBaseapp";
const FString KBEventTypes::onLoginBaseappFailed = "onLoginBaseappFailed";
const FString KBEventTypes::onReloginBaseapp = "onReloginBaseapp";
const FString KBEventTypes::onReloginBaseappSuccessfully = "onReloginBaseappSuccessfully";
const FString KBEventTypes::onReloginBaseappFailed = "onReloginBaseappFailed";

// ------------------------------------实体cell相关事件------------------------------------
const FString KBEventTypes::onEnterWorld = "onEnterWorld";
const FString KBEventTypes::onLeaveWorld = "onLeaveWorld";
const FString KBEventTypes::onEnterSpace = "onEnterSpace";
const FString KBEventTypes::onLeaveSpace = "onLeaveSpace";
const FString KBEventTypes::set_position = "set_position";
const FString KBEventTypes::set_direction = "set_direction";
const FString KBEventTypes::updatePosition = "updatePosition";
const FString KBEventTypes::addSpaceGeometryMapping = "addSpaceGeometryMapping";
const FString KBEventTypes::onSetSpaceData = "onSetSpaceData";
const FString KBEventTypes::onDelSpaceData = "onDelSpaceData";
const FString KBEventTypes::onControlled = "onControlled";
const FString KBEventTypes::onLoseControlledEntity = "onLoseControlledEntity";

// ------------------------------------数据下载相关------------------------------------
const FString KBEventTypes::onStreamDataStarted = "onStreamDataStarted";
const FString KBEventTypes::onStreamDataRecv = "onStreamDataRecv";
const FString KBEventTypes::onStreamDataCompleted = "onStreamDataCompleted";

// ------------------------------------SDK更新相关-------------------------------------
const FString KBEventTypes::onImportClientSDKSuccessfully = "onImportClientSDKSuccessfully";
const FString KBEventTypes::onDownloadSDK = "onDownloadSDK";

}