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

#ifndef __ENGINE_COMPONENT_MGR_H__
#define __ENGINE_COMPONENT_MGR_H__
	
// common include
//#define NDEBUG
#include "cstdkbe/timer.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "thread/threadmutex.hpp"
#include "thread/threadguard.hpp"
#include "server/common.hpp"

// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{

namespace Mercury
{
class Channel;
class Address;
class NetworkInterface;
}

// ComponentInfos.flags标志
#define COMPONENT_FLAG_NORMAL 0x00000000
#define COMPONENT_FLAG_SHUTTINGDOWN 0x00000001

class Components : public Singleton<Components>
{
public:
	static int32 ANY_UID; 

	struct ComponentInfos
	{
		ComponentInfos()
		{
			uid = 0;
			flags = COMPONENT_FLAG_NORMAL;
			cid = 0;
			groupOrderid = 0;
			globalOrderid = 0;
			pChannel = NULL;
		}

		KBEShared_ptr<Mercury::Address > pIntAddr, pExtAddr; // 内部和外部地址
		int32 uid;
		COMPONENT_ID cid;
		COMPONENT_ORDER groupOrderid, globalOrderid;
		char username[MAX_NAME + 1];
		Mercury::Channel* pChannel;
		COMPONENT_TYPE componentType;
		uint32 flags;
	};

	typedef std::vector<ComponentInfos> COMPONENTS;

	/** 组件添加删除handler */
	class ComponentsNotificationHandler
	{
	public:
		virtual ~ComponentsNotificationHandler() {};
		virtual void onAddComponent(const Components::ComponentInfos*) = 0;
		virtual void onRemoveComponent(const Components::ComponentInfos*) = 0;
	};
public:
	Components();
	~Components();

	INLINE void pNetworkInterface(Mercury::NetworkInterface * networkInterface)
	{ 
		KBE_ASSERT(networkInterface != NULL); 
		_pNetworkInterface = networkInterface; 
	}

	INLINE Mercury::NetworkInterface* pNetworkInterface()
	{ 
		return _pNetworkInterface;
	}

	void addComponent(int32 uid, const char* username, 
		COMPONENT_TYPE componentType, COMPONENT_ID componentID, int8 globalorderid, int8 grouporderid,
		uint32 intaddr, uint16 intport, 
		uint32 extaddr, uint16 extport, 
		Mercury::Channel* pChannel = NULL);

	void delComponent(int32 uid, COMPONENT_TYPE componentType, 
		COMPONENT_ID componentID, bool ignoreComponentID = false, bool shouldShowLog = true);

	void removeComponentFromChannel(Mercury::Channel * pChannel);

	void clear(int32 uid = -1, bool shouldShowLog = true);

	Components::COMPONENTS& getComponents(COMPONENT_TYPE componentType);

	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(COMPONENT_ID componentID);
	Components::ComponentInfos* findComponent(Mercury::Channel * pChannel);
	//const Components::ComponentInfos findComponent(COMPONENT_TYPE componentType, int32 uid);

	int connectComponent(COMPONENT_TYPE componentType, int32 uid, COMPONENT_ID componentID);

	typedef std::map<int32/*uid*/, int32/*lastorder*/> ORDER_LOG;
	ORDER_LOG& getGlobalOrderLog(){ return _globalOrderLog; }
	ORDER_LOG& getBaseappGroupOrderLog(){ return _baseappGrouplOrderLog; }
	ORDER_LOG& getCellappGroupOrderLog(){ return _cellappGrouplOrderLog; }
	ORDER_LOG& getLoginappGroupOrderLog(){ return _loginappGrouplOrderLog; }
	
	/** 
		检查所有的组件， 防止有重复的uuid， 此时应该报错.
	*/
	bool checkComponents(int32 uid, COMPONENT_ID componentID);

	void pHandler(ComponentsNotificationHandler* ph){ _pHandler = ph; };

	/** 
		检查某个组件是否有效.
	*/
	bool checkComponentUsable(const Components::ComponentInfos* info);

	Components::ComponentInfos* getBaseappmgr();
	Components::ComponentInfos* getCellappmgr();
	Components::ComponentInfos* getDbmgr();
	Components::ComponentInfos* getResourcemgr();
	Components::ComponentInfos* getMessagelog();
	Components::ComponentInfos* getBillings();

	Mercury::Channel* getBaseappmgrChannel();
	Mercury::Channel* getCellappmgrChannel();
	Mercury::Channel* getDbmgrChannel();
	Mercury::Channel* getResourcemgrChannel();
	Mercury::Channel* getMessagelogChannel();

private:
	COMPONENTS								_baseapps;
	COMPONENTS								_cellapps;
	COMPONENTS								_dbmgrs;
	COMPONENTS								_loginapps;
	COMPONENTS								_cellappmgrs;
	COMPONENTS								_baseappmgrs;
	COMPONENTS								_machines;
	COMPONENTS								_messagelogs;
	COMPONENTS								_resourcemgrs;
	COMPONENTS								_billings;
	COMPONENTS								_bots;
	COMPONENTS								_consoles;

	Mercury::NetworkInterface*				_pNetworkInterface;
	
	// 组件的全局启动次序log和组(相同的组件为一组， 如：所有baseapp为一个组)启动次序log
	// 注意:中途有死掉的app组件这里log并不去做减操作, 从使用意图来看也没有必要做这个匹配。
	ORDER_LOG								_globalOrderLog;
	ORDER_LOG								_baseappGrouplOrderLog;
	ORDER_LOG								_cellappGrouplOrderLog;
	ORDER_LOG								_loginappGrouplOrderLog;

	ComponentsNotificationHandler*			_pHandler;
};

}
#endif
