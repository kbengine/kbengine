// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CLIENTAPP_ENTITY_H
#define KBE_CLIENTAPP_ENTITY_H

#include "entity_aspect.h"
#include "client_lib/profile.h"
#include "common/timer.h"
#include "common/common.h"
#include "helper/debug_helper.h"
#include "entitydef/entity_call.h"
#include "entitydef/entity_component.h"
#include "pyscript/math.h"
#include "pyscript/scriptobject.h"
#include "entitydef/datatypes.h"	
#include "entitydef/entitydef.h"	
#include "entitydef/scriptdef_module.h"
#include "entitydef/entity_macro.h"	
#include "server/script_timers.h"	
	
namespace KBEngine{
class EntityCall;
class ClientObjectBase;
class EntityComponent;

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
	Entity(ENTITY_ID id, const ScriptDefModule* pScriptModule, EntityCall* base, EntityCall* cell);
	~Entity();
	
	/** 
		定义属性数据被改变了 
	*/
	void onDefDataChanged(EntityComponent* pEntityComponent, const PropertyDescription* propertyDescription,
			PyObject* pyData);
	
	/** 
		entityCall section
	*/
	INLINE EntityCall* baseEntityCall() const;
	DECLARE_PY_GET_MOTHOD(pyGetBaseEntityCall);
	INLINE void baseEntityCall(EntityCall* entityCall);
	
	INLINE EntityCall* cellEntityCall() const;
	DECLARE_PY_GET_MOTHOD(pyGetCellEntityCall);
	INLINE void cellEntityCall(EntityCall* entityCall);

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
		实体客户端的位置和朝向
	*/
	INLINE Position3D& clientPos();
	INLINE void clientPos(const Position3D& pos);
	INLINE void clientPos(float x, float y, float z);

	INLINE Direction3D& clientDir();
	INLINE void clientDir(const Direction3D& dir);
	INLINE void clientDir(float roll, float pitch, float yaw);

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
	
	/**
	    用于Entity的数据第一次设置时，决定是否要回调脚本层的set_*方法
	*/
	void callPropertysSetMethods();

	bool inWorld() const{ return enterworld_; }

	void onBecomePlayer();
	void onBecomeNonPlayer();
	
	bool isOnGround() const { return isOnGround_;}
	void isOnGround(bool v) { isOnGround_ = v;}

	INLINE bool isInited();
	INLINE void isInited(bool status);

    bool isControlled() { return isControlled_; }
    void onControlled(bool p_controlled);

	bool isPlayer();
	DECLARE_PY_MOTHOD_ARG0(pyIsPlayer);

protected:
	EntityCall*								cellEntityCall_;					// 这个entity的cell-entityCall
	EntityCall*								baseEntityCall_;					// 这个entity的base-entityCall

	Position3D								position_, serverPosition_;			// entity的当前位置
	Direction3D								direction_;							// entity的当前方向

	Position3D								clientPos_;							// 客户端位置，如果实体被客户端控制用于向服务器同步位置
	Direction3D								clientDir_;							// 客户端朝向，如果实体被客户端控制用于向服务器同步朝向

	ClientObjectBase*						pClientApp_;

	EntityAspect							aspect_;

	float									velocity_;

	bool									enterworld_;						// 是否已经enterworld了， restore时有用
	
	bool									isOnGround_;

	ScriptID								pMoveHandlerID_;
	
	bool									inited_;							// __init__调用之后设置为true

    bool                                    isControlled_;                      // 是否被控制
};																										

}
}

#ifdef CODE_INLINE
#include "entity.inl"
#endif
#endif
