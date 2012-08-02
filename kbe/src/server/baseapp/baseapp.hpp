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


#ifndef __BASEAPP_H__
#define __BASEAPP_H__
	
// common include	
#include "base.hpp"
#include "server/entity_app.hpp"
#include "server/pendingLoginmgr.hpp"
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

class Baseapp :	public EntityApp<Base>, 
				public Singleton<Baseapp>
{
public:
	enum TimeOutType
	{
		TIMEOUT_MAX = TIMEOUT_ENTITYAPP_MAX + 1
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
	
	/* 相关处理接口 */
	virtual void handleTimeout(TimerHandle handle, void * arg);
	virtual void handleGameTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool initializeEnd();
	void finalise();
	
	/* 网络接口
		dbmgr告知已经启动的其他baseapp或者cellapp的地址
		当前app需要主动的去与他们建立连接
	*/
	virtual void onGetEntityAppFromDbmgr(Mercury::Channel* pChannel, 
							int32 uid, 
							std::string& username, 
							int8 componentType, uint64 componentID, 
							uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport);

	virtual Base* onCreateEntityCommon(PyObject* pyEntity, ScriptModule* sm, ENTITY_ID eid);

	/* 创建一个entity */
	static PyObject* __py_createBase(PyObject* self, PyObject* args);
	static PyObject* __py_createBaseAnywhere(PyObject* self, PyObject* args);

	/** 创建一个新的space */
	void createInNewSpace(Base* base, PyObject* cell);

	/** 在一个负载较低的baseapp上创建一个baseEntity */
	void createBaseAnywhere(const char* entityType, PyObject* params, PyObject* pyCallback);

	/** 收到baseappmgr决定将某个baseapp要求createBaseAnywhere的请求在本baseapp上执行 
		@param entityType	: entity的类别， entities.xml中的定义的。
		@param strInitData	: 这个entity被创建后应该给他初始化的一些数据， 需要使用pickle.loads解包.
		@param componentID	: 请求创建entity的baseapp的组件ID
	*/
	void onCreateBaseAnywhere(Mercury::Channel* pChannel, MemoryStream& s);

	/** baseapp 的createBaseAnywhere的回调 */
	void onCreateBaseAnywhereCallback(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);
	void _onCreateBaseAnywhereCallback(Mercury::Channel* pChannel, CALLBACK_ID callbackID, 
		std::string& entityType, ENTITY_ID eid, COMPONENT_ID componentID);

	/** 为一个baseEntity在制定的cell上创建一个cellEntity */
	void createCellEntity(EntityMailboxAbstract* createToCellMailbox, Base* base);
	
	/** 网络接口
		createCellEntity的cell实体创建成功回调。
	*/
	void onEntityGetCell(Mercury::Channel* pChannel, ENTITY_ID id, COMPONENT_ID componentID);

	/** 通知客户端创建一个proxy对应的实体 */
	bool createClientProxyEntity(Mercury::Channel* pChannel, Base* base);

	/** 网络接口
		dbmgr发送初始信息
		startID: 初始分配ENTITY_ID 段起始位置
		endID: 初始分配ENTITY_ID 段结束位置
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
	*/
	void onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder);

	/** 网络接口
		dbmgr广播global数据的改变
	*/
	void onBroadcastGlobalBasesChange(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 网络接口
	注册将要登录的账号, 注册后则允许登录到此网关
	*/
	void registerPendingLogin(Mercury::Channel* pChannel, std::string& accountName, std::string& password);

	/** 网络接口
		新用户请求登录到网关上
	*/
	void loginGateway(Mercury::Channel* pChannel, std::string& accountName, std::string& password);

	/*
	   登录失败
	   @failedcode: 失败返回码 0=登录非法（超时或者非法侵入）, 
							   1=账号或者密码不正确
	*/
	void loginGatewayFailed(Mercury::Channel* pChannel, std::string& accountName, int8 failedcode);

protected:
	GlobalDataClient*					pGlobalBases_;								// globalBases

	// 记录登录到服务器但还未处理完毕的账号
	PendingLoginMgr pendingLoginMgr_;
};

}
#endif
