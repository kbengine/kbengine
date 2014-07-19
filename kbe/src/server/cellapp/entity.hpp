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

#ifndef __ENTITY_H__
#define __ENTITY_H__
	
// common include
//#include "entitymovecontroller.hpp"
#include "profile.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "helper/debug_helper.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "pyscript/math.hpp"
#include "pyscript/scriptobject.hpp"
#include "entitydef/datatypes.hpp"	
#include "entitydef/entitydef.hpp"	
#include "entitydef/scriptdef_module.hpp"
#include "entitydef/entity_macro.hpp"	
#include "server/script_timers.hpp"	

//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

class Chunk;
class Entity;
class EntityMailbox;
class Cellapp;
class Witness;
class AllClients;
class CoordinateSystem;
class EntityCoordinateNode;
class Controller;
class Controllers;
class Space;

namespace Mercury
{
class Channel;
class Bundle;
}

typedef SmartPointer<Entity> EntityPtr;
typedef std::vector<EntityPtr> SPACE_ENTITIES;

class Entity : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	BASE_SCRIPT_HREADER(Entity, ScriptObject)	
	ENTITY_HEADER(Entity)
public:
	Entity(ENTITY_ID id, const ScriptDefModule* scriptModule);
	~Entity();
	
	/** 
		销毁这个entity 
	*/
	void onDestroy(bool callScript);
	
	/**
		销毁场景
	*/
	DECLARE_PY_MOTHOD_ARG0(pyDestroySpace);
	void destroySpace(void);

	/** 
		判断自身是否是一个realEntity 
	*/
	INLINE bool isReal(void)const;

	/** 
		定义属性数据被改变了 
	*/
	void onDefDataChanged(const PropertyDescription* propertyDescription, 
			PyObject* pyData);
	
	/** 
		该entity通信通道
	*/
	INLINE void pChannel(Mercury::Channel* pchannel);
	INLINE Mercury::Channel* pChannel(void)const ;
public:
	/** 
		mailbox section
	*/
	INLINE EntityMailbox* getBaseMailbox()const;
	DECLARE_PY_GET_MOTHOD(pyGetBaseMailbox);
	INLINE void setBaseMailbox(EntityMailbox* mailbox);
	
	INLINE EntityMailbox* getClientMailbox()const;
	DECLARE_PY_GET_MOTHOD(pyGetClientMailbox);
	INLINE void setClientMailbox(EntityMailbox* mailbox);

	/**
		all_clients
	*/
	INLINE AllClients* getAllClients()const;
	DECLARE_PY_GET_MOTHOD(pyGetAllClients);
	INLINE void setAllClients(AllClients* clients);

	/**
		other_clients
	*/
	INLINE AllClients* getOtherClients()const;
	DECLARE_PY_GET_MOTHOD(pyGetOtherClients);
	INLINE void setOtherClients(AllClients* clients);

	/** 
		脚本获取和设置entity的position 
	*/
	INLINE Position3D& getPosition();
	INLINE void setPosition(const Position3D& pos);
	DECLARE_PY_GETSET_MOTHOD(pyGetPosition, pySetPosition);

	/** 
		脚本获取和设置entity的方向 
	*/
	INLINE Direction3D& getDirection();
	INLINE void setDirection(const Direction3D& dir);
	DECLARE_PY_GETSET_MOTHOD(pyGetDirection, pySetDirection);
	

	/**
		是否在地面上
	*/
	INLINE void isOnGround(bool v);
	INLINE bool isOnGround()const;
	DECLARE_PY_GET_MOTHOD(pyGetIsOnGround);

	/** 
		设置entity方向和位置 
	*/
	void setPositionAndDirection(const Position3D& position, 
		const Direction3D& direction);
	
	void onPositionChanged();
	void onDirectionChanged();
	
	void onPyPositionChanged();
	void onPyDirectionChanged();
	
	void updateLastPos();

	bool checkMoveForTopSpeed(const Position3D& position);

	/** 网络接口
		客户端设置新位置
	*/
	void setPosition_XZ_int(Mercury::Channel* pChannel, int32 x, int32 z);

	/** 网络接口
		客户端设置新位置
	*/
	void setPosition_XYZ_int(Mercury::Channel* pChannel, int32 x, int32 y, int32 z);

	/** 网络接口
		客户端设置位置
	*/
	void setPosition_XZ_float(Mercury::Channel* pChannel, float x, float z);

	/** 网络接口
		客户端设置位置
	*/
	void setPosition_XYZ_float(Mercury::Channel* pChannel, float x, float y, float z);

	/** 网络接口
		entity传送
		@cellAppID: 要传送到的目的cellappID
		@targetEntityID：要传送到这个entity的space中
		@sourceBaseAppID: 有可能是由某个baseapp上的base请求teleport的， 如果为0则为cellEntity发起
	*/
	void teleportFromBaseapp(Mercury::Channel* pChannel, COMPONENT_ID cellAppID, ENTITY_ID targetEntityID, COMPONENT_ID sourceBaseAppID);

	/**
		cell上的传送方法
	*/
	DECLARE_PY_MOTHOD_ARG3(pyTeleport, PyObject_ptr, PyObject_ptr, PyObject_ptr);
	void teleport(PyObject_ptr nearbyMBRef, Position3D& pos, Direction3D& dir);
	void teleportLocal(PyObject_ptr nearbyMBRef, Position3D& pos, Direction3D& dir);
	void teleportRefEntity(Entity* entity, Position3D& pos, Direction3D& dir);
	void teleportRefMailbox(EntityMailbox* nearbyMBRef, Position3D& pos, Direction3D& dir);
	void onTeleportRefMailbox(EntityMailbox* nearbyMBRef, Position3D& pos, Direction3D& dir);

	/**
		传送成功和失败相关回调
	*/
	void onTeleport();
	void onTeleportFailure();
	void onTeleportSuccess(PyObject* nearbyEntity, SPACE_ID lastSpaceID);
	void onReqTeleportOtherAck(Mercury::Channel* pChannel, ENTITY_ID nearbyMBRefID, SPACE_ID destSpaceID);

	/**
		进入离开cell等回调
	*/
	void onEnteredCell();
	void onEnteringCell();
	void onLeavingCell();
	void onLeftCell();
	
	/**
		进入离开space等回调
	*/
	void onEnterSpace(Space* pSpace);
	void onLeaveSpace(Space* pSpace);

	/** 
		当cellapp意外终止后， baseapp如果能找到合适的cellapp则将其恢复后
		会调用此方法
	*/
	void onRestore();

	/**
		脚本调试aoi
	*/
	void debugAOI();
	DECLARE_PY_MOTHOD_ARG0(pyDebugAOI);

	/** 
		当前entity设置自身的Aoi半径范围 
	*/
	int32 setAoiRadius(float radius, float hyst);
	float getAoiRadius(void)const;
	float getAoiHystArea(void)const;
	DECLARE_PY_MOTHOD_ARG2(pySetAoiRadius, float, float);
	
	/** 
		当前entity是否为real 
	*/
	DECLARE_PY_MOTHOD_ARG0(pyIsReal);
	
	/** 
		向baseapp发送备份数据
	*/
	void backupCellData();

	/** 
		将要保存到数据库之前的通知 
	*/
	void onWriteToDB();

	/** 
		脚本获取和设置entity的position 
	*/
	INLINE int8 layer()const;
	DECLARE_PY_GETSET_MOTHOD(pyGetLayer, pySetLayer);

	/** 
		射线 
	*/
	int raycast(int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPos);
	static PyObject* __py_pyRaycast(PyObject* self, PyObject* args);

	/** 
		entity移动导航 
	*/
	bool canNavigate();
	uint32 navigate(const Position3D& destination, float velocity, float range,
					float maxMoveDistance, float maxDistance, 
					bool faceMovement, float girth, PyObject* userData);


	DECLARE_PY_MOTHOD_ARG0(pycanNavigate);
	DECLARE_PY_MOTHOD_ARG8(pyNavigate, PyObject_ptr, float, float, float, float, int8, float, PyObject_ptr);

	/** 
		entity移动到某个点 
	*/
	uint32 moveToPoint(const Position3D& destination, float velocity, 
			PyObject* userData, bool faceMovement, bool moveVertically);
	
	DECLARE_PY_MOTHOD_ARG5(pyMoveToPoint, PyObject_ptr, float, PyObject_ptr, int32, int32);

	/** 
		entity移动到某个entity 
	*/
	uint32 moveToEntity(ENTITY_ID targetID, float velocity, float range,
			PyObject* userData, bool faceMovement, bool moveVertically);
	
	DECLARE_PY_MOTHOD_ARG6(pyMoveToEntity, int32, float, float, PyObject_ptr, int32, int32);

	/** 
		脚本获取和设置entity的最高xz移动速度 
	*/
	float getTopSpeed()const{ return topSpeed_; }
	INLINE void setTopSpeed(float speed);
	DECLARE_PY_GETSET_MOTHOD(pyGetTopSpeed, pySetTopSpeed);
	
	/** 
		脚本获取和设置entity的最高y移动速度 
	*/
	INLINE float getTopSpeedY()const;
	INLINE void setTopSpeedY(float speed);
	DECLARE_PY_GETSET_MOTHOD(pyGetTopSpeedY, pySetTopSpeedY);
	
	/** 
		脚本请求获得一定范围内的某种类型的entities 
	*/
	static PyObject* __py_pyEntitiesInRange(PyObject* self, PyObject* args);

	/** 
		脚本请求获得AOI范围内的entities 
	*/
	DECLARE_PY_MOTHOD_ARG0(pyEntitiesInAOI);

	/**
		设置获取是否自动备份
	*/
	INLINE int8 shouldAutoBackup()const;
	INLINE void shouldAutoBackup(int8 v);
	DECLARE_PY_GETSET_MOTHOD(pyGetShouldAutoBackup, pySetShouldAutoBackup);

	/** 网络接口
		远程呼叫本entity的方法 
	*/
	void onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s);
	void onRemoteCallMethodFromClient(Mercury::Channel* pChannel, MemoryStream& s);
	void onRemoteMethodCall_(MethodDescription* md, MemoryStream& s);

	/**
		观察者
	*/
	INLINE Witness* pWitness()const;
	INLINE void pWitness(Witness* w);

	/** 
		是否被任何proxy监视到, 如果这个entity没有客户端， 则这个值有效 
	*/
	INLINE bool isWitnessed(void)const;
	DECLARE_PY_GET_MOTHOD(pyIsWitnessed);

	/** 
		entity是否是一个观察者 
	*/
	INLINE bool hasWitness(void)const;
	DECLARE_PY_GET_MOTHOD(pyHasWitness);

	/** 
		自身被一个观察者观察到了 
	*/
	void addWitnessed(Entity* entity);

	/** 
		移除一个观察自身的观察者 
	*/
	void delWitnessed(Entity* entity);
	void onDelWitnessed();

	INLINE const std::list<ENTITY_ID>&	witnesses();

	/** 网络接口
		entity绑定了一个观察者(客户端)

	*/
	void setWitness(Witness* pWitness);
	void onGetWitness(Mercury::Channel* pChannel);

	/** 网络接口
		entity丢失了一个观察者(客户端)

	*/
	void onLoseWitness(Mercury::Channel* pChannel);

	/** 网络接口
		entity丢失了一个观察者(客户端)

	*/
	void onResetWitness(Mercury::Channel* pChannel);

	/** 
		client更新数据
	*/
	void onUpdateDataFromClient(KBEngine::MemoryStream& s);

	/** 
		添加一个范围触发器  
	*/
	uint32 addProximity(float range_xz, float range_y, int32 userarg);
	DECLARE_PY_MOTHOD_ARG3(pyAddProximity, float, float, int32);

	/** 
		添加一个范围触发器  
	*/
	DECLARE_PY_MOTHOD_ARG1(pyClientEntity, ENTITY_ID);

	/** 
		恢复所有的范围触发器 
		在teleport时会出现这样的情况
	*/
	void restoreProximitys();

	/** 
		删除一个范围触发器  
	*/
	void cancelController(uint32 id);
	static PyObject* __py_pyCancelController(PyObject* self, PyObject* args);

	/** 
		一个entity进入了这个entity的某个范围触发器  
	*/
	void onEnterTrap(Entity* entity, float range_xz, float range_y, 
							uint32 controllerID, int32 userarg);

	/** 
		一个entity离开了这个entity的某个范围触发器  
	*/
	void onLeaveTrap(Entity* entity, float range_xz, float range_y, 
							uint32 controllerID, int32 userarg);

	/** 
		当entity跳到一个新的space上去后，离开范围触发器事件将触发这个接口 
	*/
	void onLeaveTrapID(ENTITY_ID entityID, 
							float range_xz, float range_y, 
							uint32 controllerID, int32 userarg);

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
		获取自身在space的entities中的位置
	*/
	INLINE SPACE_ENTITIES::size_type spaceEntityIdx()const;
	INLINE void spaceEntityIdx(SPACE_ENTITIES::size_type idx);

	/**
		获取entity所在节点
	*/
	INLINE EntityCoordinateNode* pEntityCoordinateNode()const;
	INLINE void pEntityCoordinateNode(EntityCoordinateNode* pNode);

	/**
		安装卸载节点
	*/
	void installCoordinateNodes(CoordinateSystem* pCoordinateSystem);
	void uninstallCoordinateNodes(CoordinateSystem* pCoordinateSystem);

	/**
		获取entity位置朝向在某时间是否改变过
	*/
	INLINE GAME_TIME posChangedTime()const;
	INLINE GAME_TIME dirChangedTime()const;
private:
	/** 
		发送teleport结果到base端
	*/
	void _sendBaseTeleportResult(ENTITY_ID sourceEntityID, COMPONENT_ID sourceBaseAppID, 
		SPACE_ID spaceID, SPACE_ID lastSpaceID, bool fromCellTeleport);
protected:
	// 这个entity的客户端mailbox
	EntityMailbox*											clientMailbox_;						

	// 这个entity的baseapp mailbox
	EntityMailbox*											baseMailbox_;						

	// entity的当前位置
	Position3D												lastpos_;	
	Position3D												position_;							
	script::ScriptVector3*									pPyPosition_;	

	// entity的当前方向
	Direction3D												direction_;		
	script::ScriptVector3*									pPyDirection_;	

	// entity位置朝向在某时间是否改变过
	// 此属性可用于如:决定在某期间是否要高度同步该entity
	GAME_TIME												posChangedTime_;
	GAME_TIME												dirChangedTime_;

	// 自己是否是一个realEntity
	bool													isReal_;	

	// 是否在地面上
	bool													isOnGround_;						

	// entity x,z轴最高移动速度
	float													topSpeed_;							

	// entity y轴最高移动速度
	float													topSpeedY_;							

	// 自身在space的entities中的位置
	SPACE_ENTITIES::size_type								spaceEntityIdx_;					

	// 是否被任何观察者监视到
	std::list<ENTITY_ID>									witnesses_;

	// 观察者对象
	Witness*												pWitness_;							

	AllClients*												allClients_;
	AllClients*												otherClients_;

	// entity节点
	EntityCoordinateNode*									pEntityCoordinateNode_;					

	// 控制器管理器
	Controllers*											pControllers_;						
	Controller*												pMoveController_;

	// 是否进行自动备份 <= 0为false, 1为true, KBE_NEXT_ONLY为执行一次后自动为false
	int8													shouldAutoBackup_;

	script::ScriptVector3::PYVector3ChangedCallback			pyPositionChangedCallback_;
	script::ScriptVector3::PYVector3ChangedCallback			pyDirectionChangedCallback_;
	
	// entity层， 可以做任意表示， 基于tile的游戏可以表示为海陆空等层， 纯3d也可以表示各种层
	// 在脚本层做搜索的时候可以按层搜索.
	int8													layer_;
};

}

#ifdef CODE_INLINE
#include "entity.ipp"
#endif
#endif
