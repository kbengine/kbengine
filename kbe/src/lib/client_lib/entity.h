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

#ifndef KBE_CLIENTAPP_ENTITY_H
#define KBE_CLIENTAPP_ENTITY_H

#include "entity_aspect.h"
#include "client_lib/profile.h"
#include "common/timer.h"
#include "common/common.h"
#include "helper/debug_helper.h"
#include "entitydef/entity_mailbox.h"
#include "pyscript/math.h"
#include "pyscript/scriptobject.h"
#include "entitydef/datatypes.h"	
#include "entitydef/entitydef.h"	
#include "entitydef/scriptdef_module.h"
#include "entitydef/entity_macro.h"	
#include "server/script_timers.h"	
	
namespace KBEngine{
class EntityMailbox;
class ClientObjectBase;

namespace Network
{
class Channel;
}

namespace client
{

class Entity : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	BASE_SCRIPT_HREADER(Entity, ScriptObject)	
	ENTITY_HEADER(Entity)
		
public:
	Entity(ENTITY_ID id, const ScriptDefModule* scriptModule, EntityMailbox* base, EntityMailbox* cell);
	~Entity();
	
	/** 
		定义属性数据被改变了 
	*/
	void onDefDataChanged(const PropertyDescription* propertyDescription, 
			PyObject* pyData);
	
	/** 
		mailbox section
	*/
	INLINE EntityMailbox* baseMailbox() const;
	DECLARE_PY_GET_MOTHOD(pyGetBaseMailbox);
	INLINE void baseMailbox(EntityMailbox* mailbox);
	
	INLINE EntityMailbox* cellMailbox() const;
	DECLARE_PY_GET_MOTHOD(pyGetCellMailbox);
	INLINE void cellMailbox(EntityMailbox* mailbox);

	/** 
		脚本获取和设置entity的position 
	*/
	INLINE Position3D& position();
	INLINE Position3D& serverPosition();
	INLINE void position(const Position3D& pos);
	INLINE void serverPosition(const Position3D& pos);
	void onPositionChanged();
	DECLARE_PY_GETSET_MOTHOD(pyGetPosition, pySetPosition);

	/** 
		脚本获取和设置entity的方向 
	*/
	INLINE Direction3D& direction();
	INLINE void direction(const Direction3D& dir);
	void onDirectionChanged();
	DECLARE_PY_GETSET_MOTHOD(pyGetDirection, pySetDirection);
	
	/**
		移动速度
	*/
	INLINE void moveSpeed(float speed);
	INLINE float moveSpeed() const;
	void onMoveSpeedChanged();
	DECLARE_PY_GETSET_MOTHOD(pyGetMoveSpeed, pySetMoveSpeed);

	/** 
		pClientApp section
	*/
	DECLARE_PY_GET_MOTHOD(pyGetClientApp);
	void pClientApp(ClientObjectBase* p);
	INLINE ClientObjectBase* pClientApp() const;
	
	const EntityAspect* getAspect() const{ return &aspect_; }

	/** 
		entity移动到某个点 
	*/
	uint32 moveToPoint(const Position3D& destination, float velocity, float distance,
			PyObject* userData, bool faceMovement, bool moveVertically);
	
	DECLARE_PY_MOTHOD_ARG6(pyMoveToPoint, PyObject_ptr, float, float, PyObject_ptr, int32, int32);

	/** 
		停止任何移动行为
	*/
	bool stopMove();

	/** 
		entity的一次移动完成 
	*/
	void onMove(uint32 controllerId, int layer, const Position3D& oldPos, PyObject* userarg);

	/** 
		entity的移动完成 
	*/
	void onMoveOver(uint32 controllerId, int layer, const Position3D& oldPos, PyObject* userarg);

	/** 
		entity移动失败
	*/
	void onMoveFailure(uint32 controllerId, PyObject* userarg);

	/** 
		删除一个控制器  
	*/
	void cancelController(uint32 id);
	static PyObject* __py_pyCancelController(PyObject* self, PyObject* args);

	/** 
		销毁这个entity 
	*/
	void onDestroy(bool callScript){};

	void onEnterWorld();
	void onLeaveWorld();

	void onEnterSpace();
	void onLeaveSpace();

	/**
		远程呼叫本entity的方法 
	*/
	void onRemoteMethodCall(Network::Channel* pChannel, MemoryStream& s);

	/**
		服务器更新entity属性
	*/
	void onUpdatePropertys(MemoryStream& s);

	bool inWorld() const{ return enterworld_; }

	void onBecomePlayer();
	void onBecomeNonPlayer();
	
	bool isOnGound() const { return isOnGound_;}
	void isOnGound(bool v) { isOnGound_ = v;}

protected:
	EntityMailbox*							cellMailbox_;						// 这个entity的cell-mailbox
	EntityMailbox*							baseMailbox_;						// 这个entity的base-mailbox

	Position3D								position_, serverPosition_;			// entity的当前位置
	Direction3D								direction_;							// entity的当前方向

	ClientObjectBase*						pClientApp_;

	EntityAspect							aspect_;

	float									velocity_;

	bool									enterworld_;						// 是否已经enterworld了， restore时有用
	
	bool									isOnGound_;

	ScriptID								pMoveHandlerID_;
};																										

}
}

#ifdef CODE_INLINE
#include "entity.inl"
#endif
#endif
