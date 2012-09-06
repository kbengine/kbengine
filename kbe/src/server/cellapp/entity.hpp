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
	void onDestroy(void);
	
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
		脚本获取和设置entity的position 
	*/
	INLINE Position3D& getPosition();
	void setPosition(Position3D& pos);
	DECLARE_PY_GETSET_MOTHOD(pyGetPosition, pySetPosition);

	/** 
		脚本获取和设置entity的方向 
	*/
	INLINE Direction3D& getDirection();
	INLINE void setDirection(Direction3D& dir);
	DECLARE_PY_GETSET_MOTHOD(pyGetDirection, pySetDirection);
	
	/** 
		设置entity方向和位置 
	*/
	void setPositionAndDirection(Position3D& position, 
		Direction3D& direction);

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

	/** 
		脚本请求为当前所在space设置一个几何映射 
	*/
	DECLARE_PY_MOTHOD_ARG2(pyAddSpaceGeometryMapping, SPACE_ID, const_charptr);

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

	/**
		传送成功和失败相关回调
	*/
	void onTeleport();
	void onTeleportFailure();
	void onTeleportSuccess(PyObject* nearbyEntity);
	void _onTeleportSuccess();

	/**
		进入离开cell等回调
	*/
	void onEnteredCell();
	void onEnteringCell();
	void onLeavingCell();
	void onLeftCell();

	/** 
		当前entity设置自身的Aoi半径范围 
	*/
	int32 setAoiRadius(float radius, float hyst);
	INLINE float getAoiRadius(void)const;
	INLINE float getAoiHystArea(void)const;
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
		entity移动导航 
	*/
	bool navigateStep(const Position3D& destination, float velocity, 
					float maxMoveDistance, float maxDistance, 
					bool faceMovement, float girth, PyObject* userData);

	DECLARE_PY_MOTHOD_ARG7(pyNavigateStep, PyObject_ptr, float, float, float, int8, float, PyObject_ptr);
	
	/** 
		停止任何方式的移动行为 
	*/
	bool stopMove();
	DECLARE_PY_MOTHOD_ARG0(pyStopMove);

	/** 
		entity移动到某个点 
	*/
	bool moveToPoint(const Position3D& destination, float velocity, 
			PyObject* userData, bool faceMovement, bool moveVertically);
	
	DECLARE_PY_MOTHOD_ARG5(pyMoveToPoint, PyObject_ptr, float, PyObject_ptr, int32, int32);

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
		脚本请求获得一定范围类的某种类型的entities 
	*/
	DECLARE_PY_MOTHOD_ARG3(pyEntitiesInRange, float, PyObject_ptr, PyObject_ptr);
public:
	/** 网络接口
		远程呼叫本entity的方法 
	*/
	void onRemoteMethodCall(Mercury::Channel* pChannel, MemoryStream& s);

	/** 
		设置这个entity当前所在chunk的ID 
	*/
	//void setCurrentChunk(Chunk* chunk){ Chunk* oldchunk = currChunk_; currChunk_ = chunk; onCurrentChunkChanged(oldchunk); }

	/** 
		当前chunk被改变 
	*/
	//void onCurrentChunkChanged(Chunk* oldChunk);

	/** 
		获得这个entity当前所在chunk的ID 
	*/
	//Chunk* getAtChunk(void)const{ return currChunk_; }

	/**
		观察者
	*/
	INLINE Witness* witness()const;
	INLINE void witness(Witness* w);

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
	void onWitnessed(Entity* entity, float range);

	/** 
		移除一个观察自身的观察者 
	*/
	void onRemoveWitness(Entity* entity);

	/** 网络接口
		entity绑定了一个观察者(客户端)

	*/
	void onGetWitness(Mercury::Channel* pChannel);

	/** 网络接口
		entity丢失了一个观察者(客户端)

	*/
	void onLoseWitness(Mercury::Channel* pChannel);

	/** 
		添加一个陷阱 
	*/
	uint16 addProximity(float range);
	DECLARE_PY_MOTHOD_ARG1(pyAddProximity, float);

	/** 
		删除一个陷阱 
	*/
	void delProximity(uint16 id);
	DECLARE_PY_MOTHOD_ARG1(pyDelProximity, uint16);

	/** 
		一个entity进入了这个entity的某个陷阱 
	*/
	void onEnterTrap(Entity* entity, float range, 
							int controllerID);

	/** 
		一个entity离开了这个entity的某个陷阱 
	*/
	void onLeaveTrap(Entity* entity, float range, 
							int controllerID);

	/** 
		当entity跳到一个新的space上去后，离开陷阱陷阱事件将触发这个接口 
	*/
	void onLeaveTrapID(ENTITY_ID entityID, 
							float range, int controllerID);

	/** 
		entity的一次移动完成 
	*/
	void onMove(PyObject* userData);

	/**
		获取自身在space的entities中的位置
	*/
	INLINE SPACE_ENTITIES::size_type spaceEntityIdx()const;
	INLINE void spaceEntityIdx(SPACE_ENTITIES::size_type idx);

private:
	/** 
		发送teleport结果到base端
	*/
	void _sendBaseTeleportResult(ENTITY_ID sourceEntityID, COMPONENT_ID sourceBaseAppID, SPACE_ID spaceID);
protected:
	EntityMailbox*							clientMailbox_;						// 这个entity的客户端mailbox
	EntityMailbox*							baseMailbox_;						// 这个entity的baseapp mailbox

	Position3D								position_;							// entity的当前位置
	Direction3D								direction_;							// entity的当前方向

	bool									isReal_;							// 自己是否是一个realEntity

	float									aoiRadius_;							// 当前entity的aoi半径
	float									aoiHysteresisArea_;					// 当前entityAoi的一个滞后范围

	bool									isWitnessed_;						// 是否被任何观察者监视到


	float									topSpeed_;							// entity x,z轴最高移动速度
	float									topSpeedY_;							// entity y轴最高移动速度

	SPACE_ENTITIES::size_type				spaceEntityIdx_;					// 自身在space的entities中的位置

	Witness*								pWitness_;							// 观察者对象
};																										
																											

}

#ifdef CODE_INLINE
#include "entity.ipp"
#endif
#endif
