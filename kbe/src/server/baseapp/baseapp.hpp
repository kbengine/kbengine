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

protected:
	GlobalDataClient*					pGlobalBases_;								// globalBases
};

}
#endif
