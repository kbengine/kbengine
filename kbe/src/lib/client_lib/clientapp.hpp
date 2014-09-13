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


#ifndef KBE_CLIENT_APP_HPP
#define KBE_CLIENT_APP_HPP

#include "clientobjectbase.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "helper/script_loglevel.hpp"
#include "xmlplus/xmlplus.hpp"	
#include "cstdkbe/singleton.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/timer.hpp"
#include "network/interfaces.hpp"
#include "network/encryption_filter.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "thread/threadpool.hpp"
#include "pyscript/script.hpp"
#include "resmgr/resmgr.hpp"
	
namespace KBEngine{

namespace Mercury
{
class Channel;
class TCPPacketReceiver;
}

class ClientApp : 
	public Singleton<ClientApp>,
	public ClientObjectBase,
	public TimerHandler, 
	public Mercury::ChannelTimeOutHandler,
	public Mercury::ChannelDeregisterHandler
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK
	};

	enum C_STATE
	{
		C_STATE_INIT = 0,
		C_STATE_INITLOGINAPP_CHANNEL = 1,
		C_STATE_LOGIN = 2,
		C_STATE_LOGIN_GATEWAY_CHANNEL = 3,
		C_STATE_LOGIN_GATEWAY = 4,
		C_STATE_PLAY = 5,
	};
public:
	ClientApp(Mercury::EventDispatcher& dispatcher, 
			Mercury::NetworkInterface& ninterface, 
			COMPONENT_TYPE componentType,
			COMPONENT_ID componentID);

	~ClientApp();

	virtual bool initialize();
	virtual bool initializeBegin();
	virtual bool inInitialize(){ return true; }
	virtual bool initializeEnd();
	virtual void finalise();
	virtual bool run();
	
	virtual void reset(void);

	virtual int processOnce(bool shouldIdle = false);

	virtual bool installPyModules();
	virtual void onInstallPyModules(){};
	virtual bool uninstallPyModules();
	virtual bool uninstallPyScript();
	virtual bool installEntityDef();

	void registerScript(PyTypeObject*);
	int registerPyObjectToScript(const char* attrName, PyObject* pyObj);
	int unregisterPyObjectToScript(const char* attrName);

	PyObjectPtr getEntryScript(){ return entryScript_; }

	const char* name(){return COMPONENT_NAME_EX(componentType_);}
	
	virtual void handleTimeout(TimerHandle, void * pUser);
	virtual void handleGameTick();

	void shutDown();

	bool login(std::string accountName, std::string passwd, 
		std::string ip, KBEngine::uint32 port);

	GAME_TIME time() const { return g_kbetime; }
	double gameTimeInSeconds() const;

	Mercury::EventDispatcher & mainDispatcher()				{ return mainDispatcher_; }
	Mercury::NetworkInterface & networkInterface()			{ return networkInterface_; }

	COMPONENT_ID componentID()const	{ return componentID_; }
	COMPONENT_TYPE componentType()const	{ return componentType_; }
		
	KBEngine::script::Script& getScript(){ return *pScript_; }
	void setScript(KBEngine::script::Script* p){ pScript_ = p; }

	static PyObject* __py_getAppPublish(PyObject* self, PyObject* args);
	static PyObject* __py_getPlayer(PyObject* self, PyObject* args);
	static PyObject* __py_fireEvent(PyObject* self, PyObject* args);

	virtual void onServerClosed();

	/**
		设置脚本输出类型前缀
	*/
	static PyObject* __py_setScriptLogType(PyObject* self, PyObject* args);

	virtual void onChannelTimeOut(Mercury::Channel * pChannel);
	virtual void onChannelDeregister(Mercury::Channel * pChannel);

	virtual void onHelloCB_(Mercury::Channel* pChannel, const std::string& verInfo, 
		const std::string& scriptVerInfo, COMPONENT_TYPE componentType);

	/** 网络接口
		和服务端的版本不匹配
	*/
	virtual void onVersionNotMatch(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		和服务端的脚本层版本不匹配
	*/
	virtual void onScriptVersionNotMatch(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
	   登录成功
	   @ip: 服务器ip地址
	   @port: 服务器端口
	*/
	virtual void onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s);

	/** 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_SRV_OVERLOAD:服务器负载过重, 
									MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	virtual void onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s);

	/** 网络接口
	   登录失败回调
	   @failedcode: 失败返回码 MERCURY_ERR_SRV_NO_READY:服务器没有准备好, 
									MERCURY_ERR_ILLEGAL_LOGIN:非法登录, 
									MERCURY_ERR_NAME_PASSWORD:用户名或者密码不正确
	*/
	virtual void onLoginGatewayFailed(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode);

	/** 网络接口
	   重登陆baseapp成功
	*/
	virtual void onReLoginGatewaySuccessfully(Mercury::Channel * pChannel);

	virtual void onTargetChanged();

	/** 
		服务端添加了某个space的几何映射
	*/
	virtual void onAddSpaceGeometryMapping(SPACE_ID spaceID, std::string& respath);

	static PyObject* __py_GetSpaceData(PyObject *self, PyObject* args)
	{
		return ClientObjectBase::__py_GetSpaceData(&ClientApp::getSingleton(), args);	
	}

	static PyObject* __py_callback(PyObject *self, PyObject* args)
	{
		return ClientObjectBase::__py_callback(&ClientApp::getSingleton(), args);	
	}

	static PyObject* __py_cancelCallback(PyObject *self, PyObject* args)
	{
		return ClientObjectBase::__py_cancelCallback(&ClientApp::getSingleton(), args);	
	}

	static PyObject* __py_getWatcher(PyObject *self, PyObject* args)
	{
		return ClientObjectBase::__py_getWatcher(&ClientApp::getSingleton(), args);	
	}

	static PyObject* __py_getWatcherDir(PyObject *self, PyObject* args)
	{
		return ClientObjectBase::__py_getWatcherDir(&ClientApp::getSingleton(), args);	
	}

	static PyObject* __py_disconnect(PyObject *self, PyObject* args)
	{
		return ClientObjectBase::__py_disconnect(&ClientApp::getSingleton(), args);	
	}
protected:
	KBEngine::script::Script*								pScript_;
	std::vector<PyTypeObject*>								scriptBaseTypes_;

	TimerHandle												gameTimer_;

	COMPONENT_TYPE											componentType_;

	// 本组件的ID
	COMPONENT_ID											componentID_;									

	Mercury::EventDispatcher& 								mainDispatcher_;
	Mercury::NetworkInterface&								networkInterface_;
	
	Mercury::TCPPacketReceiver*								pTCPPacketReceiver_;
	Mercury::BlowfishFilter*								pBlowfishFilter_;

	// 线程池
	thread::ThreadPool										threadPool_;

	PyObjectPtr												entryScript_;

	C_STATE													state_;
};

}
#endif
