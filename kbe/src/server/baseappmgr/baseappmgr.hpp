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


#ifndef __BASEAPPMGR_H__
#define __BASEAPPMGR_H__
	
// common include	
#include "server/kbemain.hpp"
#include "server/serverapp.hpp"
#include "server/idallocate.hpp"
#include "server/serverconfig.hpp"
#include "server/forward_messagebuffer.hpp"
#include "cstdkbe/timer.hpp"
#include "network/endpoint.hpp"

//#define NDEBUG
#include <map>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class Baseappmgr :	public ServerApp, 
					public Singleton<Baseappmgr>
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK = TIMEOUT_SERVERAPP_MAX + 1
	};
	
	Baseappmgr(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~Baseappmgr();
	
	bool run();
	
	void handleTimeout(TimerHandle handle, void * arg);
	void handleGameTick();

	/* 初始化相关接口 */
	bool initializeBegin();
	bool inInitialize();
	bool initializeEnd();
	void finalise();

	/** 网络接口
		收到baseapp::createBaseAnywhere请求在某个空闲的baseapp上创建一个baseEntity
		@param sp: 这个数据包中存储的是 entityType	: entity的类别， entities.xml中的定义的。
										strInitData	: 这个entity被创建后应该给他初始化的一些数据， 
													  需要使用pickle.loads解包.
										componentID	: 请求创建entity的baseapp的组件ID
	*/
	void reqCreateBaseAnywhere(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		消息转发， 由某个app想通过本app将消息转发给某个app。
	*/
	void forwardMessage(Mercury::Channel* pChannel, MemoryStream& s);

	/** 网络接口
		一个新登录的账号获得合法登入baseapp的权利， 现在需要将账号注册给baseapp
		使其允许在此baseapp上登录。
	*/
	void registerPendingAccountToBaseapp(Mercury::Channel* pChannel, 
								std::string& accountName, std::string& password);

	/** 网络接口
		baseapp将自己的地址发送给loginapp并转发给客户端。
	*/
	void onPendingAccountGetBaseappAddr(Mercury::Channel* pChannel, 
								  uint32 addr, uint16 port);
protected:
	TimerHandle					gameTimer_;
	Forward_MessageBuffer		forward_baseapp_messagebuffer_;
};

}
#endif
