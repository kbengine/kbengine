// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_LOGINAPP_H
#define KBE_LOGINAPP_H
	
// common include	
#include "server/kbemain.h"
#include "server/serverapp.h"
#include "server/idallocate.h"
#include "server/serverconfig.h"
#include "server/pendingLoginmgr.h"
#include "server/python_app.h"
#include "common/timer.h"
#include "network/endpoint.h"
	
namespace KBEngine{

class HTTPCBHandler;
class TelnetServer;

class Loginapp :	public PythonApp, 
					public Singleton<Loginapp>
{
public:
	enum TimeOutType
	{
		TIMEOUT_TICK = TIMEOUT_PYTHONAPP_MAX + 1
	};

	Loginapp(Network::EventDispatcher& dispatcher, 
		Network::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Loginapp();
	
	bool run();
	
	virtual void onChannelDeregister(Network::Channel * pChannel);

	virtual void handleTimeout(TimerHandle handle, void * arg);
	void handleMainTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	void onInstallPyModules();
	
	virtual void onShutdownBegin();
	virtual void onShutdownEnd();

	/** 信号处理
	*/
	virtual bool installSignals();
	virtual void onSignalled(int sigNum);

	virtual void onHello(Network::Channel* pChannel, 
		const std::string& verInfo, 
		const std::string& scriptVerInfo, 
		const std::string& encryptedKey);

	/** 网络接口
		某个client向本app告知处于活动状态。
	*/
	void onClientActiveTick(Network::Channel* pChannel);

	/** 网络接口
		创建账号
	*/
	bool _createAccount(Network::Channel* pChannel, std::string& accountName, 
		std::string& password, std::string& datas, ACCOUNT_TYPE type = ACCOUNT_TYPE_NORMAL);
	void reqCreateAccount(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		创建email账号
	*/
	void reqCreateMailAccount(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		创建账号
	*/
	void onReqCreateAccountResult(Network::Channel* pChannel, MemoryStream& s);
	void onReqCreateMailAccountResult(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		重置账号密码申请（忘记密码?）
	*/
	void reqAccountResetPassword(Network::Channel* pChannel, std::string& accountName);
	void onReqAccountResetPasswordCB(Network::Channel* pChannel, std::string& accountName, std::string& email,
		SERVER_ERROR_CODE failedcode, std::string& code);

	/** 网络接口
		dbmgr账号激活返回
	*/
	void onAccountActivated(Network::Channel* pChannel, std::string& code, bool success);

	/** 网络接口
		dbmgr账号绑定email返回
	*/
	void onAccountBindedEmail(Network::Channel* pChannel, std::string& code, bool success);

	/** 网络接口
		dbmgr账号重设密码返回
	*/
	void onAccountResetPassword(Network::Channel* pChannel, std::string& code, bool success);

	/** 网络接口
	baseapp请求绑定email（返回时需要找到loginapp的地址）
	*/
	void onReqAccountBindEmailAllocCallbackLoginapp(Network::Channel* pChannel, COMPONENT_ID reqBaseappID, ENTITY_ID entityID, std::string& accountName, std::string& email,
		SERVER_ERROR_CODE failedcode, std::string& code);

	/** 网络接口
		用户登录服务器
		clientType[COMPONENT_CLIENT_TYPE]: 前端类别(手机， web， pcexe端)
		clientData[str]: 前端附带数据(可以是任意的， 比如附带手机型号， 浏览器类型等)
		accountName[str]: 帐号名
		password[str]: 密码
	*/
	void login(Network::Channel* pChannel, MemoryStream& s);

	/*
		登录失败
		failedcode: 失败返回码 NETWORK_ERR_SRV_NO_READY:服务器没有准备好, 
									NETWORK_ERR_SRV_OVERLOAD:服务器负载过重, 
									NETWORK_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	void _loginFailed(Network::Channel* pChannel, std::string& loginName, 
		SERVER_ERROR_CODE failedcode, std::string& datas, bool force = false);
	
	/** 网络接口
		dbmgr返回的登录账号检测结果
	*/
	void onLoginAccountQueryResultFromDbmgr(Network::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		baseappmgr返回的登录网关地址
	*/
	void onLoginAccountQueryBaseappAddrFromBaseappmgr(Network::Channel* pChannel, std::string& loginName, 
		std::string& accountName, std::string& addr, uint16 tcp_port, uint16 udp_port);


	/** 网络接口
		dbmgr发送初始信息
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
	*/
	void onDbmgrInitCompleted(Network::Channel* pChannel, COMPONENT_ORDER startGlobalOrder, 
		COMPONENT_ORDER startGroupOrder, const std::string& digest);

	/** 网络接口
		客户端协议导出
	*/
	void importClientMessages(Network::Channel* pChannel);

	/** 网络接口
		错误码描述导出
	*/
	void importServerErrorsDescr(Network::Channel* pChannel);

	/** 网络接口
	客户端SDK导出
	*/
	void importClientSDK(Network::Channel* pChannel, MemoryStream& s);

	// 引擎版本不匹配
	virtual void onVersionNotMatch(Network::Channel* pChannel);

	// 引擎脚本层版本不匹配
	virtual void onScriptVersionNotMatch(Network::Channel* pChannel);

	/** 网络接口
		baseapp同步自己的初始化信息
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
	*/
	void onBaseappInitProgress(Network::Channel* pChannel, float progress);

protected:
	TimerHandle							mainProcessTimer_;

	// 记录注册账号还未登陆的请求
	PendingLoginMgr						pendingCreateMgr_;

	// 记录登录到服务器但还未处理完毕的账号
	PendingLoginMgr						pendingLoginMgr_;

	std::string							digest_;

	HTTPCBHandler*						pHttpCBHandler;

	float								initProgress_;
	
	TelnetServer*						pTelnetServer_;
};

}

#endif // KBE_LOGINAPP_H
