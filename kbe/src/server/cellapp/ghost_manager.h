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

#ifndef KBE_GHOST_MANAGER_HANDLER_H
#define KBE_GHOST_MANAGER_HANDLER_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{

namespace Network
{
class Bundle;
}

class Entity;

/*
	* cell1: entity(1) is real, 则在GhostManager中存放于entityIDs_进行检查  (向其他ghost更新)

	* cell2: entity(1) is ghost, 如果cell2被整体迁移走，则需要向ghost_route_临时设置一个路由地址， 路由在最后一次收包超过一定时间擦除。
	                    如果期间有一些包被转发过来， 那么找不到entity就查询路由表，并继续转发到ghostEntity(例如real销毁了要求立即销毁ghost)。

	* cell1: entity(1) is real, 如果被再迁移到cell3， 则需要向ghost_route_临时设置一个路由地址， 路由在最后一次收ghost请求包超过一定时间擦除。
	                    如果期间有一些ghost请求包被转发过来， 那么找不到entity就查询路由表，并继续转发到realEntity。
*/
class GhostManager : public TimerHandler
{
public:
	GhostManager();
	~GhostManager();

	void pushMessage(COMPONENT_ID componentID, Network::Bundle* pBundle);
	void pushRouteMessage(ENTITY_ID entityID, COMPONENT_ID componentID, Network::Bundle* pBundle);

	COMPONENT_ID getRoute(ENTITY_ID entityID);
	void addRoute(ENTITY_ID entityID, COMPONENT_ID componentID);

private:
	virtual void handleTimeout(TimerHandle handle, void * pUser);

	virtual void onRelease( TimerHandle handle, void * /*pUser*/ ){};

	void cancel();

	void start();

private:
	void syncMessages();
	void syncGhosts();

	void checkRoute();

	struct ROUTE_INFO
	{
		ROUTE_INFO():
		componentID(0),
		lastTime(0)
		{
		}

		COMPONENT_ID componentID;
		uint64 lastTime;
	};

private:
	// 所有存在ghost的相关entity
	std::map<ENTITY_ID, Entity*> 	realEntities_;
	
	// ghost路由， 分布式程序某些时候无法保证同步， 那么在本机上的某些entity被迁移走了的
	// 时候可能会还会收到一些网络消息， 因为其他app可能还无法立即得到迁移地址， 此时我们
	// 可以在当前app上将迁移走的entity指向缓存一下， 有网络消息过来我们可以继续转发到新的地址
	std::map<ENTITY_ID, ROUTE_INFO> ghost_route_;

	// 所有需要广播的事件消息
	std::map<COMPONENT_ID, std::vector< Network::Bundle* > > messages_;

	TimerHandle* pTimerHandle_;

	uint64 checkTime_;
};


}

#endif // KBE_GHOST_MANAGER_HANDLER_H
