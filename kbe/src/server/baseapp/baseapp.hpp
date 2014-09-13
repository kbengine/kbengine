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


#ifndef KBE_BASEAPP_HPP
#define KBE_BASEAPP_HPP
	
// common include	
#include "base.hpp"
#include "proxy.hpp"
#include "profile.hpp"
#include "server/entity_app.hpp"
#include "server/pendingLoginmgr.hpp"
#include "server/forward_messagebuffer.hpp"
#include "network/endpoint.hpp"

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

namespace Mercury{
	class Channel;
}

class Proxy;
class Backuper;
class Archiver;
class TelnetServer;
class RestoreEntityHandler;

class Baseapp :	public EntityApp<Base>, 
				public Singleton<Baseapp>
{
public:
	enum TimeOutType
	{
		TIMEOUT_CHECK_STATUS = TIMEOUT_ENTITYAPP_MAX + 1,
		TIMEOUT_MAX
	};
	
	Baseapp(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Baseapp();
	
	virtual bool installPyModules();
	virtual void onInstallPyModules();
	virtual bool uninstallPyModules();

	bool run();
	
	/** 
		相关处理接口 
	*/
	virtual void handleTimeout(TimerHandle handle, void * arg);
	virtual void handleGameTick();
	void handleCheckStatusTick();
	void handleBackup();
	void handleArchive();

	/** 
		初始化相关接口 
	*/
	bool initializeBegin();
	bool initializeEnd();
	void finalise();
	
	virtual bool canShutdown();
	virtual void onShutdownBegin();
	virtual void onShutdown(bool first);
	virtual void onShutdownEnd();

	virtual bool initializeWatcher();

	float getLoad()const { return load_; }
	
	void updateLoad();

	static uint64 checkTickPeriod();

	static int quantumPassedPercent(uint64 curr = timestamp());
	static PyObject* __py_quantumPassedPercent(PyObject* self, PyObject* args);

	virtual void onChannelDeregister(Mercury::Channel * pChannel);

	/**
		一个cellapp死亡
	*/
	void onCellAppDeath(Mercury::Channel * pChannel);

	/** 网络接口
		dbmgr告知已经启动的其他baseapp或者cellapp的地址
		当前app需要主动的去与他们建立连接
	*/
	virtual void onGetEntityAppFromDbmgr(Mercury::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							int8 componentType, uint64 componentID, int8 globalorderID, int8 grouporderID,
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx);
	
	/** 网络接口
		某个client向本app告知处于活动状态。
	*/
	void onClientActiveTick(Mercury::Channel* pChannel);

	/** 
		创建了一个entity回调
	*/
	virtual Base* onCreateEntityCommon(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid);

	/** 
		创建一个entity 
	*/
	static PyObject* __py_createBase(PyObject* self, PyObject* args);
	static PyObject* __py_createBaseAnywhere(PyObject* self, PyObject* args);
	static PyObject* __py_createBaseFromDBID(PyObject* self, PyObject* args);
	static PyObject* __py_createBaseAnywhereFromDBID(PyObject* self, PyObject* args);
	
	/**
		创建一个新的space 
	*/
	void createInNewSpace(Base* base, PyObject* cell);

	/**
		恢复一个space 
	*/
	void restoreSpaceInCell(Base* base);

	/** 
		在一个负载较低的baseapp上创建一个baseEntity 
	*/
	void createBaseAnywhere(const char* entityType, PyObject* params, PyObject* pyCallback);

	/** 收到baseappmgr决定将某个baseapp要求createBaseAnywhere的请求在本baseapp上执行 
		@param entityType	: entity的类别， entities.xml中的定义的。
		@param strInitData	: 这个entity被创建后应该给他初始化的一些数据， 需要使用pickle.loads解包.
		@param componentID	: 请求创建entity的baseapp的组件ID
	*/
	void onCreateBaseAnywhere(Mercury::Channel* pChannel, MemoryStream& s);

	/** 
		从db获取信息创建一个entity
	*/
	void createBaseFromDBID(const char* entityType, DBID dbid, PyObject* pyCallback);

	/** 网络接口
		createBaseFromDBID的回调。
	*/
	void onCreateBaseFromDBIDCallback(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 
		从db获取信息创建一个entity
	*/
	void createBaseAnywhereFromDBID(const char* entityType, DBID dbid, PyObject* pyCallback);

	/** 网络接口
		createBaseFromDBID的回调。
	*/
	// 从数据库来的回调
	void onCreateBaseAnywhereFromDBIDCallback(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	// 请求在这个进程上创建这个entity
	void createBaseAnywhereFromDBIDOtherBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	// 创建完毕后的回调
	void onCreateBaseAnywhereFromDBIDOtherBaseappCallback(Mercury::Channel* pChannel, COMPONENT_ID createByBaseappID, 
							std::string entityType, ENTITY_ID createdEntityID, CALLBACK_ID callbackID, DBID dbid);
	

	/** 
		baseapp 的createBaseAnywhere的回调 
	*/
	void onCreateBaseAnywhereCallback(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	void _onCreateBaseAnywhereCallback(Mercury::Channel* pChannel, CALLBACK_ID callbackID, 
		std::string& entityType, ENTITY_ID eid, COMPONENT_ID componentID);

	/** 
		为一个baseEntity在指定的cell上创建一个cellEntity 
	*/
	void createCellEntity(EntityMailboxAbstract* createToCellMailbox, Base* base);
	
	/** 网络接口
		createCellEntity失败的回调。
	*/
	void onCreateCellFailure(Mercury::Channel* pChannel, ENTITY_ID entityID);

	/** 网络接口
		createCellEntity的cell实体创建成功回调。
	*/
	void onEntityGetCell(Mercury::Channel* pChannel, ENTITY_ID id, COMPONENT_ID componentID, SPACE_ID spaceID);

	/** 
		通知客户端创建一个proxy对应的实体 
	*/
	bool createClientProxies(Proxy* base, bool reload = false);

	/** 
		向dbmgr请求执行一个数据库命令
	*/
	static PyObject* __py_executeRawDatabaseCommand(PyObject* self, PyObject* args);
	void executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback, ENTITY_ID eid);
	void onExecuteRawDatabaseCommandCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		dbmgr发送初始信息
		startID: 初始分配ENTITY_ID 段起始位置
		endID: 初始分配ENTITY_ID 段结束位置
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
	*/
	void onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		GAME_TIME gametime, ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder, const std::string& digest);

	/** 网络接口
		dbmgr广播global数据的改变
	*/
	void onBroadcastBaseAppDataChanged(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		注册将要登录的账号, 注册后则允许登录到此网关
	*/
	void registerPendingLogin(Mercury::Channel* pChannel, std::string& loginName, std::string& accountName, 
		std::string& password, ENTITY_ID entityID, DBID entityDBID, uint32 flags, uint64 deadline, COMPONENT_TYPE componentType);

	/** 网络接口
		新用户请求登录到网关上
	*/
	void loginGateway(Mercury::Channel* pChannel, std::string& accountName, std::string& password);

	/**
		踢出一个Channel
	*/
	void kickChannel(Mercury::Channel* pChannel, SERVER_ERROR_CODE failedcode);

	/** 网络接口
		重新登录 快速与网关建立交互关系(前提是之前已经登录了， 
		之后断开在服务器判定该前端的Entity未超时销毁的前提下可以快速与服务器建立连接并达到操控该entity的目的)
	*/
	void reLoginGateway(Mercury::Channel* pChannel, std::string& accountName, 
		std::string& password, uint64 key, ENTITY_ID entityID);

	/**
	   登录失败
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_ILLEGAL_LOGIN:非法登录, 
									MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	void loginGatewayFailed(Mercury::Channel* pChannel, std::string& accountName, SERVER_ERROR_CODE failedcode);

	/** 网络接口
		从dbmgr获取到账号Entity信息
	*/
	void onQueryAccountCBFromDbmgr(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/**
		客户端自身进入世界了
	*/
	void onClientEntityEnterWorld(Proxy* base, COMPONENT_ID componentID);

	/** 网络接口
		entity收到一封mail, 由某个app上的mailbox发起(只限与服务器内部使用， 客户端的mailbox调用方法走
		onRemoteCellMethodCallFromClient)
	*/
	void onEntityMail(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/** 网络接口
		client访问entity的cell方法
	*/
	void onRemoteCallCellMethodFromClient(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		client更新数据
	*/
	void onUpdateDataFromClient(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);


	/** 网络接口
		cellapp备份entity的cell数据
	*/
	void onBackupEntityCellData(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		cellapp writeToDB完成
	*/
	void onCellWriteToDBCompleted(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		cellapp转发entity消息给client
	*/
	void forwardMessageToClientFromCellapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		cellapp转发entity消息给某个baseEntity的cellEntity
	*/
	void forwardMessageToCellappFromCellapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	
	/**
		获取游戏时间
	*/
	static PyObject* __py_gametime(PyObject* self, PyObject* args);

	/** 网络接口
		写entity到db回调
	*/
	void onWriteToDBCallback(Mercury::Channel* pChannel, ENTITY_ID eid, DBID entityDBID, CALLBACK_ID callbackID, bool success);

	/**
		增加proxices计数
	*/
	void incProxicesCount(){ ++numProxices_; }

	/**
		减少proxices计数
	*/
	void decProxicesCount(){ --numProxices_; }

	/**
		获得proxices计数
	*/
	int32 numProxices()const{ return numProxices_; }

	/**
		获得numClients计数
	*/
	int32 numClients(){ return this->networkInterface().numExtChannels(); }
	
	/** 
		请求充值
	*/
	static PyObject* __py_charge(PyObject* self, PyObject* args);
	void charge(std::string chargeID, DBID dbid, const std::string& datas, PyObject* pycallback);
	void onChargeCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/**
		hook mailboxcall
	*/
	RemoteEntityMethod* createMailboxCallEntityRemoteMethod(MethodDescription* md, EntityMailbox* pMailbox);

	virtual void onHello(Mercury::Channel* pChannel, 
		const std::string& verInfo, 
		const std::string& scriptVerInfo, 
		const std::string& encryptedKey);

	// 引擎版本不匹配
	virtual void onVersionNotMatch(Mercury::Channel* pChannel);

	// 引擎脚本层版本不匹配
	virtual void onScriptVersionNotMatch(Mercury::Channel* pChannel);

	/**
		一个cell的entity都恢复完毕
	*/
	void onRestoreEntitiesOver(RestoreEntityHandler* pRestoreEntityHandler);

	/** 网络接口
		某个baseapp上的space恢复了cell， 判断当前baseapp是否有相关entity需要恢复cell
	*/
	void onRestoreSpaceCellFromOtherBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		某个app请求查看该app
	*/
	virtual void lookApp(Mercury::Channel* pChannel);

	/** 网络接口
		客户端协议导出
	*/
	void importClientMessages(Mercury::Channel* pChannel);

	/** 网络接口
		客户端entitydef导出
	*/
	void importClientEntityDef(Mercury::Channel* pChannel);

	/**
		重新导入所有的脚本
	*/
	static PyObject* __py_reloadScript(PyObject* self, PyObject* args);
	virtual void reloadScript(bool fullReload);
	virtual void onReloadScript(bool fullReload);

	/**
		获取进程是否正在关闭中
	*/
	static PyObject* __py_isShuttingDown(PyObject* self, PyObject* args);

	/**
		获取进程内部网络地址
	*/
	static PyObject* __py_address(PyObject* self, PyObject* args);

	/**
		通过dbid从数据库中删除一个实体

		从数据库删除实体， 如果实体不在线则可以直接删除回调返回true， 如果在线则回调返回的是entity的mailbox， 其他任何原因都返回false.
	*/
	static PyObject* __py_deleteBaseByDBID(PyObject* self, PyObject* args);

	/** 网络接口
		通过dbid从数据库中删除一个实体的回调
	*/
	void deleteBaseByDBIDCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		请求绑定email
	*/
	void reqAccountBindEmail(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& password, std::string& email);

	void onReqAccountBindEmailCB(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& email,
		SERVER_ERROR_CODE failedcode, std::string& code);

	/** 网络接口
		请求绑定email
	*/
	void reqAccountNewPassword(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& oldpassworld, std::string& newpassword);

	void onReqAccountNewPasswordCB(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName,
		SERVER_ERROR_CODE failedcode);
protected:
	TimerHandle												loopCheckTimerHandle_;

	GlobalDataClient*										pBaseAppData_;								// globalBases

	// 记录登录到服务器但还未处理完毕的账号
	PendingLoginMgr											pendingLoginMgr_;

	ForwardComponent_MessageBuffer							forward_messagebuffer_;

	// 备份存档相关
	KBEShared_ptr< Backuper >								pBackuper_;	
	KBEShared_ptr< Archiver >								pArchiver_;	

	float													load_;

	static uint64											_g_lastTimestamp;

	int32													numProxices_;

	TelnetServer*											pTelnetServer_;

	std::vector< KBEShared_ptr< RestoreEntityHandler > >	pRestoreEntityHandlers_;

	TimerHandle												pResmgrTimerHandle_;
};

}

#endif // KBE_BASEAPP_HPP
