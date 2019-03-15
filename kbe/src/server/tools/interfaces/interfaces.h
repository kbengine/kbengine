// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_INTERFACES_TOOL_H
#define KBE_INTERFACES_TOOL_H

#include "server/kbemain.h"
#include "server/python_app.h"
#include "server/serverconfig.h"
#include "server/callbackmgr.h"	
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

	/* ��ʼ����ؽӿ� */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();
	void onInstallPyModules();
	
	bool initDB();

	virtual void onShutdownBegin();
	virtual void onShutdownEnd();


	/** ����ӿ�
	ע��һ���¼����baseapp����cellapp����dbmgr
	ͨ����һ���µ�app�������ˣ� ����Ҫ��ĳЩ���ע���Լ���
	*/
	virtual void onRegisterNewApp(Network::Channel* pChannel,
		int32 uid,
		std::string& username,
		COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderID, COMPONENT_ORDER grouporderID,
		uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx);

	/** ����ӿ�
		���󴴽��˺�
	*/
	void reqCreateAccount(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		һ�����û���¼�� ��Ҫ���Ϸ���
	*/
	void onAccountLogin(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** ����ӿ�
		��������ͻ�����������
	*/
	void eraseClientReq(Network::Channel* pChannel, std::string& logkey);

	/** ����ӿ�
		�����ֵ
	*/
	void charge(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	/** Python�ص��ӿ�
	    ��ֵ��Ӧ
	*/
	void chargeResponse(std::string orderID, std::string extraDatas, KBEngine::SERVER_ERROR_CODE errorCode);
	static PyObject* __py_chargeResponse(PyObject* self, PyObject* args);

	/** Python�ص��ӿ�
	    �����¼�˺ŵ���Ӧ
	*/
	void accountLoginResponse(std::string commitName, std::string realAccountName, 
		std::string extraDatas, KBEngine::SERVER_ERROR_CODE errorCode);
	static PyObject* __py_accountLoginResponse(PyObject* self, PyObject* args);

	/** Python�ص��ӿ�
	    ���󴴽��˺ŵ���Ӧ
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
	
	/**
		��dbmgr����ִ��һ�����ݿ�����
	*/
	static PyObject* __py_executeRawDatabaseCommand(PyObject* self, PyObject* args);
	void executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback, ENTITY_ID eid, const std::string& dbInterfaceName);
	void onExecuteRawDatabaseCommandCB(Network::Channel* pChannel, KBEngine::MemoryStream& s);

	PY_CALLBACKMGR& callbackMgr() { return pyCallbackMgr_; }

protected:
	TimerHandle																mainProcessTimer_;

	// ����
	ORDERS																	orders_;

	// ���е������¼�� ����ĳ���ظ�������
	REQCREATE_MAP															reqCreateAccount_requests_;
	REQLOGIN_MAP															reqAccountLogin_requests_;

	TelnetServer*															pTelnetServer_;

	PY_CALLBACKMGR															pyCallbackMgr_;

};

}

#endif // KBE_INTERFACES_TOOL_H

