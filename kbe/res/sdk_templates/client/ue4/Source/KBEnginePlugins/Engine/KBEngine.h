// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "KBECommon.h"
#include "ServerErrorDescrs.h"
#include "Interfaces.h"
#include "KBETicker.h"

namespace KBEngine
{

class KBEngineArgs;
class Entity;
class NetworkInterfaceBase;
class MemoryStream;
class EncryptionFilter;

/*
	这是KBEngine插件的核心模块
	包括网络创建、持久化协议、entities的管理、以及引起对外可调用接口。

	一些可以参考的地方:
	http://www.kbengine.org/docs/programming/clientsdkprogramming.html
	http://www.kbengine.org/docs/programming/kbe_message_format.html
*/
class KBENGINEPLUGINS_API KBEngineApp : public InterfaceConnect
{
public:
	KBEngineApp();
	KBEngineApp(KBEngineArgs* pArgs);
	virtual ~KBEngineApp();
	
public:
	static KBEngineApp& getSingleton();
	static void destroyKBEngineApp();
public:
	bool isInitialized() const {
		return pArgs_ != NULL;
	}

	bool initialize(KBEngineArgs* pArgs);
	void destroy();
	void reset();
	
	void installUKBETicker();
	void uninstallUKBETicker();

	NetworkInterfaceBase* pNetworkInterface() const {
		return pNetworkInterface_;
	}

	void _closeNetwork();

	const TArray<uint8>& serverdatas() const
	{
		return serverdatas_;
	}

	void entityServerPos(const FVector& pos)
	{
		entityServerPos_ = pos;
	}

	KBEngineArgs* getInitArgs() const
	{
		return pArgs_;
	}

	void installEvents();

	void resetMessages();

	static bool validEmail(const FString& strEmail);

	/*
		通过错误id得到错误描述
	*/
	FString serverErr(uint16 id);

	Entity* player();
	Entity* findEntity(int32 entityID);

	/*
		服务端错误描述导入了
	*/
	void Client_onImportServerErrorsDescr(MemoryStream& stream)
	{
		// 无需实现，已由插件生成静态代码
	}

	/*
		从服务端返回的二进制流导入客户端消息协议
	*/
	void Client_onImportClientMessages(MemoryStream& stream)
	{
		// 无需实现，已由插件生成静态代码
	}

	/*
		从服务端返回的二进制流导入客户端消息协议
	*/
	void Client_onImportClientEntityDef(MemoryStream& stream)
	{
		// 无需实现，已由插件生成静态代码
	}

	void Client_onImportClientSDK(MemoryStream& stream);

	/**
		插件的主循环处理函数
	*/
	void process();

	/*
		向服务端发送心跳以及同步角色信息到服务端
	*/
	void sendTick();

	/**
		登录到服务端，必须登录完成loginapp与网关(baseapp)，登录流程才算完毕
	*/
	bool login(const FString& username, const FString& password, const TArray<uint8>& datas);
	virtual void onConnectCallback(FString ip, uint16 port, bool success, int userdata) override;

	/**
		登录出baseapp
	*/
	bool logout();

	/*
		账号创建返回结果
	*/
	void Client_onCreateAccountResult(MemoryStream& stream);

	/*
		创建账号
	*/
	bool createAccount(const FString& username, const FString& password, const TArray<uint8>& datas);

	/*
		重置密码, 通过loginapp
	*/
	void resetPassword(const FString& username);
	void onOpenLoginapp_resetpassword();
	void onConnectTo_resetpassword_callback(FString ip, uint16 port, bool success);
	void Client_onReqAccountResetPasswordCB(uint16 failcode);

	/*
		绑定Email，通过baseapp
	*/
	void bindAccountEmail(const FString& emailAddress);
	void Client_onReqAccountBindEmailCB(uint16 failcode);

	/*
		设置新密码，通过baseapp， 必须玩家登录在线操作所以是baseapp。
	*/
	void newPassword(const FString& old_password, const FString& new_password);
	void Client_onReqAccountNewPasswordCB(uint16 failcode);

	/*
	重登录到网关(baseapp)
	一些移动类应用容易掉线，可以使用该功能快速的重新与服务端建立通信
	*/
	void reloginBaseapp();
	void onReloginTo_baseapp_callback(FString ip, uint16 port, bool success);

	/*
		登录loginapp失败了
	*/
	void Client_onLoginFailed(MemoryStream& stream);

	/*
		登录loginapp成功了
	*/
	void Client_onLoginSuccessfully(MemoryStream& stream);

	/*
		登录baseapp失败了
	*/
	void Client_onLoginBaseappFailed(uint16 failedcode);

	/*
		重登录baseapp失败了
	*/
	void Client_onReloginBaseappFailed(uint16 failedcode);

	/*
		登录baseapp成功了
	*/
	void Client_onReloginBaseappSuccessfully(MemoryStream& stream);

	void hello();
	void Client_onHelloCB(MemoryStream& stream);

	void Client_onVersionNotMatch(MemoryStream& stream);
	void Client_onScriptVersionNotMatch(MemoryStream& stream);

	/*
		被服务端踢出
	*/
	void Client_onKicked(uint16 failedcode);

	/*
		服务器心跳回调
	*/
	void Client_onAppActiveTickCB();

	/*
		服务端通知创建一个角色
	*/
	void Client_onCreatedProxies(uint64 rndUUID, int32 eid, FString& entityType);

	/*
		服务端通知强制销毁一个实体
	*/
	void Client_onEntityDestroyed(int32 eid);

	/*
		服务端使用优化的方式更新实体属性数据
	*/
	void Client_onUpdatePropertysOptimized(MemoryStream& stream);

	/*
		服务端更新实体属性数据
	*/
	void Client_onUpdatePropertys(MemoryStream& stream);

	/*
		服务端使用优化的方式调用实体方法
	*/
	void Client_onRemoteMethodCallOptimized(MemoryStream& stream);

	/*
		服务端调用实体方法
	*/
	void Client_onRemoteMethodCall(MemoryStream& stream);

	/*
		告诉客户端：你当前负责（或取消）控制谁的位移同步
	*/
	void Client_onControlEntity(ENTITY_ID eid, int8 isControlled);

	/*
		服务端初始化客户端的spacedata， spacedata请参考API
	*/
	void Client_initSpaceData(MemoryStream& stream);
	FString getSpaceData(const FString& key);

	/*
		服务端初始化客户端的spacedata， spacedata请参考API
	*/
	void Client_setSpaceData(uint32 spaceID, const FString& key, const FString& value);

	/*
		服务端删除客户端的spacedata， spacedata请参考API
	*/
	void Client_delSpaceData(uint32 spaceID, const FString& key);

	/*
		服务端通知流数据下载开始
		请参考API手册关于onStreamDataStarted
	*/
	void Client_onStreamDataStarted(int16 id, uint32 datasize, FString descr);
	void Client_onStreamDataRecv(MemoryStream& stream);
	void Client_onStreamDataCompleted(int16 id);

	/*
		服务端通知一个实体进入了世界(如果实体是当前玩家则玩家第一次在一个space中创建了， 如果是其他实体则是其他实体进入了玩家的View)
	*/
	void Client_onEntityEnterWorld(MemoryStream& stream);

	/*
		服务端使用优化的方式通知一个实体离开了世界(如果实体是当前玩家则玩家离开了space， 如果是其他实体则是其他实体离开了玩家的View)
	*/
	void Client_onEntityLeaveWorldOptimized(MemoryStream& stream);

	/*
		服务端通知一个实体离开了世界(如果实体是当前玩家则玩家离开了space， 如果是其他实体则是其他实体离开了玩家的View)
	*/
	void Client_onEntityLeaveWorld(ENTITY_ID eid);

	/*
		服务端通知当前玩家进入了一个新的space
	*/
	void Client_onEntityEnterSpace(MemoryStream& stream);

	/*
		服务端通知当前玩家离开了space
	*/
	void Client_onEntityLeaveSpace(ENTITY_ID eid);

	/*
		服务端更新玩家的基础位置， 客户端以这个基础位置加上便宜值计算出玩家周围实体的坐标
	*/
	void Client_onUpdateBasePos(float x, float y, float z);
	void Client_onUpdateBasePosXZ(float x, float z);
	void Client_onUpdateBaseDir(MemoryStream& stream);
	void Client_onUpdateData(MemoryStream& stream);

	/*
		服务端强制设置了玩家的坐标
		例如：在服务端使用avatar.position=(0,0,0), 或者玩家位置与速度异常时会强制拉回到一个位置
	*/
	void Client_onSetEntityPosAndDir(MemoryStream& stream);

	void Client_onUpdateData_ypr(MemoryStream& stream);
	void Client_onUpdateData_yp(MemoryStream& stream);
	void Client_onUpdateData_yr(MemoryStream& stream);
	void Client_onUpdateData_pr(MemoryStream& stream);
	void Client_onUpdateData_y(MemoryStream& stream);
	void Client_onUpdateData_p(MemoryStream& stream);
	void Client_onUpdateData_r(MemoryStream& stream);
	void Client_onUpdateData_xz(MemoryStream& stream);
	void Client_onUpdateData_xz_ypr(MemoryStream& stream);
	void Client_onUpdateData_xz_yp(MemoryStream& stream);
	void Client_onUpdateData_xz_yr(MemoryStream& stream);
	void Client_onUpdateData_xz_pr(MemoryStream& stream);
	void Client_onUpdateData_xz_y(MemoryStream& stream);
	void Client_onUpdateData_xz_p(MemoryStream& stream);
	void Client_onUpdateData_xz_r(MemoryStream& stream);
	void Client_onUpdateData_xyz(MemoryStream& stream);
	void Client_onUpdateData_xyz_ypr(MemoryStream& stream);
	void Client_onUpdateData_xyz_yp(MemoryStream& stream);
	void Client_onUpdateData_xyz_yr(MemoryStream& stream);
	void Client_onUpdateData_xyz_pr(MemoryStream& stream);
	void Client_onUpdateData_xyz_y(MemoryStream& stream);
	void Client_onUpdateData_xyz_p(MemoryStream& stream);
	void Client_onUpdateData_xyz_r(MemoryStream& stream);

	void Client_onUpdateData_ypr_optimized(MemoryStream& stream);
	void Client_onUpdateData_yp_optimized(MemoryStream& stream);
	void Client_onUpdateData_yr_optimized(MemoryStream& stream);
	void Client_onUpdateData_pr_optimized(MemoryStream& stream);
	void Client_onUpdateData_y_optimized(MemoryStream& stream);
	void Client_onUpdateData_p_optimized(MemoryStream& stream);
	void Client_onUpdateData_r_optimized(MemoryStream& stream);
	void Client_onUpdateData_xz_optimized(MemoryStream& stream);
	void Client_onUpdateData_xz_ypr_optimized(MemoryStream& stream);
	void Client_onUpdateData_xz_yp_optimized(MemoryStream& stream);
	void Client_onUpdateData_xz_yr_optimized(MemoryStream& stream);
	void Client_onUpdateData_xz_pr_optimized(MemoryStream& stream);
	void Client_onUpdateData_xz_y_optimized(MemoryStream& stream);
	void Client_onUpdateData_xz_p_optimized(MemoryStream& stream);
	void Client_onUpdateData_xz_r_optimized(MemoryStream& stream);
	void Client_onUpdateData_xyz_optimized(MemoryStream& stream);
	void Client_onUpdateData_xyz_ypr_optimized(MemoryStream& stream);
	void Client_onUpdateData_xyz_yp_optimized(MemoryStream& stream);
	void Client_onUpdateData_xyz_yr_optimized(MemoryStream& stream);
	void Client_onUpdateData_xyz_pr_optimized(MemoryStream& stream);
	void Client_onUpdateData_xyz_y_optimized(MemoryStream& stream);
	void Client_onUpdateData_xyz_p_optimized(MemoryStream& stream);
	void Client_onUpdateData_xyz_r_optimized(MemoryStream& stream);

private:
	void _updateVolatileData(ENTITY_ID entityID, float x, float y, float z, float yaw, float pitch, float roll, int8 isOnGround, bool isOptimized);

	bool initNetwork();

	void login_loginapp(bool noconnect);
	void onConnectTo_loginapp_login_callback(FString ip, uint16 port, bool success);
	void onLogin_loginapp();

	void login_baseapp(bool noconnect);
	void onConnectTo_baseapp_callback(FString ip, uint16 port, bool success);
	void onLogin_baseapp();


	void clearSpace(bool isall);
	void clearEntities(bool isall);


	void updatePlayerToServer();

	void onServerDigest();

	void addSpaceGeometryMapping(uint32 uspaceID, const FString& respath);

	void resetpassword_loginapp(bool noconnect);

	void createAccount_loginapp(bool noconnect);
	void onOpenLoginapp_createAccount();
	void onConnectTo_loginapp_create_callback(FString ip, uint16 port, bool success);

	/*
		通过流数据获得View实体的ID
	*/
	ENTITY_ID getViewEntityIDFromStream(MemoryStream& stream);

	/*
	服务端更新实体属性数据
	*/
	void onUpdatePropertys_(ENTITY_ID eid, MemoryStream& stream);

	void onRemoteMethodCall_(ENTITY_ID eid, MemoryStream& stream);

public:
	SPACE_ID spaceID() const {
		return spaceID_;
	}

	ENTITY_ID entity_id() const {
		return entity_id_;
	}

	uint64 entity_uuid() const {
		return entity_uuid_;
	}

	const FString& entity_type() const {
		return entity_type_;
	}

	const FString& serverVersion() const {
		return serverVersion_;
	}

	const FString& clientVersion() const {
		return clientVersion_;
	}

	const FString& serverScriptVersion() const {
		return serverScriptVersion_;
	}

	const FString& clientScriptVersion() const {
		return clientScriptVersion_;
	}

	const FString& component() const {
		return component_;
	}

	const FString& currserver() const {
		return currserver_;
	}

	const FString& currstate() const {
		return currstate_;
	}

	typedef TMap<ENTITY_ID, Entity*> ENTITIES_MAP;
	ENTITIES_MAP& entities() {
		return entities_;
	}

protected:
	KBEngineArgs* pArgs_;
	NetworkInterfaceBase* pNetworkInterface_;

	FString username_;
	FString password_;

	// 服务端分配的baseapp地址
	FString baseappIP_;
	uint16 baseappTcpPort_;
	uint16 baseappUdpPort_;

	// 当前状态
	FString currserver_;
	FString currstate_;

	// 服务端下行以及客户端上行用于登录时处理的账号绑定的二进制信息
	// 该信息由用户自己进行扩展
	TArray<uint8> serverdatas_;
	TArray<uint8> clientdatas_;

	// 通信协议加密，blowfish协议
	TArray<uint8> encryptedKey_;

	// 服务端与客户端的版本号以及协议MD5
	FString serverVersion_;
	FString clientVersion_;
	FString serverScriptVersion_;
	FString clientScriptVersion_;
	FString serverProtocolMD5_;
	FString serverEntitydefMD5_;

	// 当前玩家的实体id与实体类别
	uint64 entity_uuid_;
	ENTITY_ID entity_id_;
	FString entity_type_;
	
	// 这个参数的选择必须与kbengine_defs.xml::cellapp/aliasEntityID的参数保持一致
	bool useAliasEntityID_;

	TArray<Entity*> controlledEntities_;

	// 当前服务端最后一次同步过来的玩家位置
	FVector entityServerPos_;

	// space的数据，具体看API手册关于spaceData
	// https://github.com/kbengine/kbengine/tree/master/docs/api
	TMap<FString, FString> spacedatas_;
	
	// 所有实体都保存于这里， 请参看API手册关于entities部分
	// https://github.com/kbengine/kbengine/tree/master/docs/api
	ENTITIES_MAP entities_;

	// 在玩家View范围小于256个实体时我们可以通过一字节索引来找到entity
	TArray<ENTITY_ID> entityIDAliasIDList_;
	TMap<ENTITY_ID, MemoryStream*> bufferedCreateEntityMessages_;

	// 所有服务端错误码对应的错误描述
	static ServerErrorDescrs serverErrs_;

	double lastTickTime_;
	double lastTickCBTime_;
	double lastUpdateToServerTime_;

	// 玩家当前所在空间的id， 以及空间对应的资源
	SPACE_ID spaceID_;
	FString spaceResPath_;
	bool isLoadedGeometry_;

	// 按照标准，每个客户端部分都应该包含这个属性
	FString component_;

	EncryptionFilter *pFilter_;
	UKBETicker *pUKBETicker_;

};

}
