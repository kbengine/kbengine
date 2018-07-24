// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_INTERFACES_TOOL_H
#define KBE_INTERFACES_TOOL_H

#include "server/kbemain.h"
#include "server/python_app.h"
#include "server/serverconfig.h"
#include "common/timer.h"
#include "network/endpoint.h"
#include "resmgr/resmgr.h"
#include "thread/threadpool.h"
	
namespace KBEngine{

class DBInterface;
class Orders;
class CreateAccountTask;
class LoginAccountTask;
class TelnetServer;


class Interfaces :	public PythonApp, 
					public Singleton<Interfaces>
{
public:
	enum TimeOutType
	{
		TIMEOUT_TICK = TIMEOUT_PYTHONAPP_MAX + 1
	};

	Interfaces(Network::EventDispatcher& dispatcher, 
		Network::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Interfaces();
	
	bool run();
	
	void handleTimeout(TimerHandle handle, void * arg);
	void handleMainTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	void onInstallPyModules();
	
	bool initDB();

	virtual void onShutdownBegin();
	virtual void onShutdownEnd();

	/** 网络接口
		请求创建账号
	*/
	void reqCreateAccount(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		一个新用户登录， 需要检查合法性
	*/
	void onAccountLogin(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
		请求擦除客户端请求任务
	*/
	void eraseClientReq(Network::Channel* pChannel, std::string& logkey);

	/** 网络接口
		请求充值
	*/
	void charge(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** Python回调接口
	    充值响应
	*/
	void chargeResponse(std::string orderID, std::string extraDatas, KBEngine::SERVER_ERROR_CODE errorCode);
	static PyObject* __py_chargeResponse(PyObject* self, PyObject* args);

	/** Python回调接口
	    请求登录账号的响应
	*/
	void accountLoginResponse(std::string commitName, std::string realAccountName, 
		std::string extraDatas, KBEngine::SERVER_ERROR_CODE errorCode);
	static PyObject* __py_accountLoginResponse(PyObject* self, PyObject* args);

	/** Python回调接口
	    请求创建账号的响应
	*/
	void createAccountResponse(std::string commitName, std::string realAccountName, 
		std::string extraDatas, KBEngine::SERVER_ERROR_CODE errorCode);
	static PyObject* __py_createAccountResponse(PyObject* self, PyObject* args);

	typedef KBEUnordered_map<std::string, KBEShared_ptr<Orders> > ORDERS;
	Interfaces::ORDERS& orders(){ return orders_; }

	typedef KBEUnordered_map<std::string, CreateAccountTask*> REQCREATE_MAP;
	typedef KBEUnordered_map<std::string, LoginAccountTask*> REQLOGIN_MAP;
	REQCREATE_MAP& reqCreateAccount_requests(){ return reqCreateAccount_requests_; }
	REQLOGIN_MAP& reqAccountLogin_requests(){ return reqAccountLogin_requests_; }

	void eraseOrders(std::string ordersid);
	bool hasOrders(std::string ordersid);
	
protected:
	TimerHandle																mainProcessTimer_;

	// 订单
	ORDERS																	orders_;

	// 所有的请求记录， 避免某类重复性请求。
	REQCREATE_MAP															reqCreateAccount_requests_;
	REQLOGIN_MAP															reqAccountLogin_requests_;

	TelnetServer*															pTelnetServer_;

};

}

#endif // KBE_INTERFACES_TOOL_H

