//
//  KNetworkInterface.h
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#ifndef __libKBEClient__KBECCNetworkInterface__
#define __libKBEClient__KBECCNetworkInterface__

#include "KBEClientcore.h"
#include "KClient_interface_marcos.h"
#include <unordered_map>
#include "KDataTypes.h"
#include "KEntity.h"
#include "KMailbox.h"
#include "KEntitydef.h"
#include "KMemoryStream.h"


namespace KBEngineClient
{
	void KBE_init();	
	void KBE_run();
	typedef	std::unordered_map<std::string,std::string>	SPACE_DATA;
	typedef int8 CLIENT_CTYPE;
	/**
	服务器错误， 主要是服务器返回给客户端用的。
*/
	
typedef uint16 SERVER_ERROR_CODE;								// 错误码类别


#define SERVER_SUCCESS								0			// 成功。
#define SERVER_ERR_SRV_NO_READY						1			// 服务器没有准备好。
#define SERVER_ERR_SRV_OVERLOAD						2			// 服务器负载过重。
#define SERVER_ERR_ILLEGAL_LOGIN					3			// 非法登录。
#define SERVER_ERR_NAME_PASSWORD					4			// 用户名或者密码不正确。
#define SERVER_ERR_NAME								5			// 用户名不正确。
#define SERVER_ERR_PASSWORD							6			// 密码不正确。
#define SERVER_ERR_ACCOUNT_CREATE					7			// 创建账号失败（已经存在一个相同的账号）。
#define SERVER_ERR_BUSY								8			// 操作过于繁忙(例如：在服务器前一次请求未执行完毕的情况下连续N次创建账号)。
#define SERVER_ERR_ANOTHER_LOGON					9			// 当前账号在另一处登录了。
#define SERVER_ERR_ACCOUNT_ONLINE					10			// 你已经登录了， 服务器拒绝再次登录。
#define SERVER_ERR_PROXY_DESTROYED					11			// 与客户端关联的proxy在服务器上已经销毁。
#define SERVER_ERR_DIGEST							12			// defmd5不匹配。
#define SERVER_ERR_SHUTTINGDOWN						13			// 服务器正在关闭中
#define SERVER_ERR_NAME_MAIL						14			// email地址错误。
#define SERVER_ERR_ACCOUNT_LOCK						15			// 账号被冻结。
#define SERVER_ERR_ACCOUNT_DEADLINE					16			// 账号已过期。
#define SERVER_ERR_ACCOUNT_NOT_ACTIVATED			17			// 账号未激活。
#define SERVER_ERR_VERSION_NOT_MATCH				18			// 账号未激活。
#define SERVER_ERR_FAILED							19			// 操作失败。

const char SERVER_ERR_STR[][256] = {
	"SERVER_SUCCESS",
	"SERVER_ERR_SRV_NO_READY",
	"SERVER_ERR_SRV_OVERLOAD",
	"SERVER_ERR_ILLEGAL_LOGIN",
	"SERVER_ERR_NAME_PASSWORD",
	"SERVER_ERR_NAME",
	"SERVER_ERR_PASSWORD",
	"SERVER_ERR_ACCOUNT_CREATE",
	"SERVER_ERR_BUSY",
	"SERVER_ERR_ANOTHER_LOGON",
	"SERVER_ERR_ACCOUNT_ONLINE",
	"SERVER_ERR_PROXY_DESTROYED",
	"SERVER_ERR_DIGEST",
	"SERVER_ERR_SHUTTINGDOWN",
	"SERVER_ERR_NAME_MAIL",
	"SERVER_ERR_ACCOUNT_LOCK",
	"SERVER_ERR_ACCOUNT_DEADLINE",
	"SERVER_ERR_ACCOUNT_NOT_ACTIVATED",
	"SERVER_ERR_VERSION_NOT_MATCH",
	"SERVER_ERR_FAILED"
};

#define KBE_PI									3.1415926535898

	
#define ENTITIES_MAP  std::map<ENTITY_ID,KBEngineClient::Entity*>

	class ClientObjectBase
	{
		public:
			ClientObjectBase(){	 pServerChannel_ = new KNetworkInterface(); pEntities_ = &entities; }
			~ClientObjectBase();
			//Entities<client::Entity>* pEntities()const{ return pEntities_; }
			std::map<ENTITY_ID,Entity*> entities;
			/**
			创建一个entity 
			*/
		
			KBEngineClient::Entity* createEntityCommon(const char* entityType, void* params,
				bool isInitializeScript = true, ENTITY_ID eid = 0, bool initProperty = true, MailBox* base = NULL, MailBox* cell = NULL);
	
			/**
			通过entityID销毁一个entity 
				*/
			virtual bool destroyEntity(ENTITY_ID entityID, bool callScript){ return false ;};
			bool createAccount();
			bool login();
			bool loginGateWay();  
			const char* name(){ return name_.c_str(); }
			ENTITY_ID entityID(){ return entityID_; }
			DBID dbid(){ return dbid_; }

			//bool registerEventHandle(EventHandle* pEventHandle);
			//bool deregisterEventHandle(EventHandle* pEventHandle);
	
			//void fireEvent(const EventData* pEventData);
			//EventHandler& eventHandler(){ return eventHandler_; }
		
		/**
			如果entitiessize小于256
			通过索引位置来获取entityID
			否则直接取ID
			*/
		
			ENTITY_ID readEntityIDFromStream(MemoryStream& s);

			/**
				由mailbox来尝试获取一个channel的实例
			*/
			//virtual Mercury::Channel* findChannelByMailbox(EntityMailbox& mailbox);

			/** 网络接口
				客户端与服务端第一次建立交互, 服务端返回
			*/
			virtual void onHelloCB_(const std::string& verInfo,COMPONENT_TYPE componentType);

			virtual void onHelloCB(MemoryStream& s);

			/** 网络接口
				和服务端的版本不匹配
			*/
			virtual void onVersionNotMatch(MemoryStream& s);
	

			/** 网络接口
				创建账号成功和失败回调
			   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
											MERCURY_ERR_ACCOUNT_CREATE:创建失败（已经存在）, 
											MERCURY_SUCCESS:账号创建成功

											SERVER_ERROR_CODE failedcode;
				@二进制附带数据:二进制额外数据: uint32长度 + bytearray
			*/
			virtual void onCreateAccountResult(MemoryStream& s);

			/** 网络接口
			   登录失败回调
			   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
											MERCURY_ERR_SRV_OVERLOAD:服务器负载过重, 
											MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
			*/
			virtual void onLoginFailed(MemoryStream& s);

			/** 网络接口
			   登录成功
			   @ip: 服务器ip地址
			   @port: 服务器端口
			*/
			virtual void onLoginSuccessfully(MemoryStream& s);

			/** 网络接口
			   登录失败回调
			   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
											MERCURY_ERR_ILLEGAL_LOGIN:非法登录, 
											MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
			*/
			virtual void onLoginGatewayFailed(SERVER_ERROR_CODE failedcode);

			/** 网络接口
				服务器端已经创建了一个与客户端关联的代理Entity
			   在登录时也可表达成功回调
			   @datas: 账号entity的信息
			*/
			virtual void onCreatedProxies(uint64 rndUUID, ENTITY_ID eid, std::string& entityType);

			/** 网络接口
				服务器上的entity已经进入游戏世界了
			*/
			virtual void onEntityEnterWorld(MemoryStream& s);

			/** 网络接口
				服务器上的entity已经离开游戏世界了
			*/
			virtual void onEntityLeaveWorld( ENTITY_ID eid);
			virtual void onEntityLeaveWorldOptimized( MemoryStream& s);

			/** 网络接口
				告诉客户端某个entity销毁了， 此类entity通常是还未onEntityEnterWorld
			*/
			virtual void onEntityDestroyed( ENTITY_ID eid);

			/** 网络接口
				服务器上的entity已经进入space了
			*/
			virtual void onEntityEnterSpace( SPACE_ID spaceID, ENTITY_ID eid);

			/** 网络接口
				服务器上的entity已经离开space了
			*/
			virtual void onEntityLeaveSpace( SPACE_ID spaceID, ENTITY_ID eid);

			/** 网络接口
				远程调用entity的方法 
			*/
			virtual void onRemoteMethodCall(MemoryStream& s);
			virtual void onRemoteMethodCallOptimized(MemoryStream& s);
			void onRemoteMethodCall_(ENTITY_ID eid, MemoryStream& s);

			/** 网络接口
			   被踢出服务器
			*/
			virtual void onKicked(SERVER_ERROR_CODE failedcode) {};

			/** 网络接口
				服务器更新entity属性
			*/
			virtual void onUpdatePropertys( MemoryStream& s);
			virtual void onUpdatePropertysOptimized( MemoryStream& s);
			void onUpdatePropertys_(ENTITY_ID eid, MemoryStream& s);

			/** 网络接口
				服务器强制设置entity的位置与朝向
			*/
			virtual void onSetEntityPosAndDir( MemoryStream& s);

			/** 网络接口
				服务器更新avatar基础位置
			*/
			virtual void onUpdateBasePos( MemoryStream& s);
			virtual void onUpdateBasePosXZ( MemoryStream& s);

			/** 网络接口
				服务器更新VolatileData
			*/
			virtual void onUpdateData( MemoryStream& s);

			virtual void onUpdateData_ypr( MemoryStream& s);
			virtual void onUpdateData_yp( MemoryStream& s);
			virtual void onUpdateData_yr(MemoryStream& s);
			virtual void onUpdateData_pr( MemoryStream& s);
			virtual void onUpdateData_y( MemoryStream& s);
			virtual void onUpdateData_p( MemoryStream& s);
			virtual void onUpdateData_r( MemoryStream& s);

			virtual void onUpdateData_xz( MemoryStream& s);
			virtual void onUpdateData_xz_ypr( MemoryStream& s);
			virtual void onUpdateData_xz_yp( MemoryStream& s);
			virtual void onUpdateData_xz_yr( MemoryStream& s);
			virtual void onUpdateData_xz_pr( MemoryStream& s);
			virtual void onUpdateData_xz_y( MemoryStream& s);
			virtual void onUpdateData_xz_p( MemoryStream& s);
			virtual void onUpdateData_xz_r( MemoryStream& s);

			virtual void onUpdateData_xyz( MemoryStream& s);
			virtual void onUpdateData_xyz_ypr( MemoryStream& s);
			virtual void onUpdateData_xyz_yp( MemoryStream& s);
			virtual void onUpdateData_xyz_yr( MemoryStream& s);
			virtual void onUpdateData_xyz_pr( MemoryStream& s);
			virtual void onUpdateData_xyz_y( MemoryStream& s);
			virtual void onUpdateData_xyz_p( MemoryStream& s);
			virtual void onUpdateData_xyz_r( MemoryStream& s);
	
			void _updateVolatileData(ENTITY_ID entityID, float x, float y, float z, float roll, float pitch, float yaw);

			/** 
				更新玩家到服务端 
			*/
			virtual void updatePlayerToServer();

			/** 网络接口
				download stream开始了 
			*/
			virtual void onStreamDataStarted(int16 id, uint32 datasize, std::string& descr);

			/** 网络接口
				接收到streamData
			*/
			virtual void onStreamDataRecv( MemoryStream& s);

			/** 网络接口
				download stream完成了 
			*/
			virtual void onStreamDataCompleted(int16 id);

			/** 网络接口
				接收到ClientMessages(通常是web等才会应用到)
			*/
			virtual void onImportClientMessages( MemoryStream& s){}

			/** 网络接口
				接收到entitydef(通常是web等才会应用到)
			*/
			virtual void onImportClientEntityDef(MemoryStream& s){}
	
			/** 网络接口
				错误码描述导出(通常是web等才会应用到)
			*/
			virtual void onImportMercuryErrorsDescr(MemoryStream& s){}

			/** 网络接口
				重置账号密码请求返回
			*/
			virtual void onReqAccountResetPasswordCB(SERVER_ERROR_CODE failedcode){}

			/** 网络接口
				请求绑定邮箱返回
			*/
			virtual void onReqAccountBindEmailCB( SERVER_ERROR_CODE failedcode){}

			/** 网络接口
				请求修改密码返回
			*/
			virtual void onReqAccountNewPasswordCB( SERVER_ERROR_CODE failedcode){}

			/** 
				获得player实例
			*/
			KBEngineClient::Entity* pPlayer();

			void setPlayerPosition(float x, float y, float z){  entityPos_.x = x; entityPos_.y = y; entityPos_.z = z; /*entityPos_ = Vector3(x, y, z);*/ }
			void setPlayerDirection(float roll, float pitch, float yaw){ ;/*entityDir_ = Vector3(roll, pitch, yaw); */}

			void setTargetID(ENTITY_ID id){ 
				targetID_ = id; 
				onTargetChanged();
			}
			ENTITY_ID getTargetID()const{ return targetID_; }
			virtual void onTargetChanged(){}

			ENTITY_ID getAoiEntityID(ENTITY_ID id);
			ENTITY_ID getAoiEntityIDFromStream(MemoryStream& s);
			ENTITY_ID getAoiEntityIDByAliasID(uint8 id);

			/** 
				space相关操作接口
				服务端添加了某个space的几何映射
			*/
			void addSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath);
			virtual void onAddSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath){}
			virtual void onLoadedSpaceGeometryMapping(SPACE_ID spaceID){
				isLoadedGeometry_ = true;
			}

			const std::string& getGeometryPath();
	
			void initSpaceData( MemoryStream& s);
			void setSpaceData( SPACE_ID spaceID, const std::string& key, const std::string& value);
			void delSpaceData( SPACE_ID spaceID, const std::string& key);
			bool hasSpaceData(const std::string& key);
			const std::string& getSpaceData(const std::string& key);
	protected:				
			int32													appID_;

			// 服务端网络通道
			//Mercury::Channel*										pServerChannel_;
			KNetworkInterface*										pServerChannel_;

			// 存储所有的entity的容器
			//Entities<client::Entity>*								pEntities_;	
			std::map<ENTITY_ID,KBEngineClient::Entity*>				*pEntities_;
			std::vector<ENTITY_ID>									pEntityIDAliasIDList_;

			//PY_CALLBACKMGR											pyCallbackMgr_;

			ENTITY_ID												entityID_;
			SPACE_ID												spaceID_;
			std::string												entity_type_;

			//Position3D												entityPos_;
			//Direction3D												entityDir_;
		 
			Vector3													entityPos_;
			Vector3													entityDir_;

			DBID													dbid_;

			std::string												ip_;
			uint16													port_;

			uint64													lastSentActiveTickTime_;
			uint64													lastSentUpdateDataTime_;

			bool													connectedGateway_;
			bool													canReset_;

			std::string												name_;
			std::string												password_;
			std::string												extradatas_;

			CLIENT_CTYPE											typeClient_;

			//typedef std::map<ENTITY_ID, KBEShared_ptr<MemoryStream> > BUFFEREDMESSAGE;
			//BUFFEREDMESSAGE											bufferedCreateEntityMessage_;
			std::map<ENTITY_ID, MemoryStream>					bufferedCreateEntityMessage_;
	
			//EventHandler											eventHandler_;

			//Mercury::NetworkInterface&								ninterface_;
			KNetworkInterface*										networkInterface_;

			// 当前客户端所选择的目标
			ENTITY_ID												targetID_;

			// 是否加载过地形数据
			bool													isLoadedGeometry_;

			SPACE_DATA												spacedatas_;
			
			//
		};

		///////////////////////////////////////////////////////////////////////////////////////////

		class ClientApp : 
			public ClientObjectBase
		{
			public:	
				static ClientApp& getInstance()
				{
					static ClientApp instance;
					return instance;
				}
	
	
		};

}

#endif /* defined(__libKBEClient__KNetworkInterface__) */
