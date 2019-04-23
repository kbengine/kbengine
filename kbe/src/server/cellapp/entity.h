// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_ENTITY_H
#define KBE_ENTITY_H
	
#include "profile.h"
#include "common/timer.h"
#include "common/common.h"
#include "common/smartpointer.h"
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

class Chunk;
class Entity;
class EntityCall;
class EntityComponent;
class Cellapp;
class Witness;
class AllClients;
class CoordinateSystem;
class EntityCoordinateNode;
class Controller;
class Controllers;
class SpaceMemory;
class VolatileInfo;

namespace Network
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
	Entity(ENTITY_ID id, const ScriptDefModule* pScriptModule,
		PyTypeObject* pyType = getScriptType(), bool isInitialised = true);
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
		当前实体所在的space将要销毁时触发  
	*/
	void onSpaceGone();
	
	/** 
		判断自身是否是一个realEntity 
	*/
	INLINE bool isReal(void) const;

	/** 
		判断自身是否有ghostEntity 
	*/
	INLINE bool hasGhost(void) const;

	/** 
		判断自身是否是一个realEntity 
	*/
	INLINE COMPONENT_ID realCell(void) const;
	INLINE void realCell(COMPONENT_ID cellID);

	/** 
		判断自身是否有ghostEntity 
	*/
	INLINE COMPONENT_ID ghostCell(void) const;
	INLINE void ghostCell(COMPONENT_ID cellID);

	/** 
		定义属性数据被改变了 
	*/
	void onDefDataChanged(EntityComponent* pEntityComponent, const PropertyDescription* propertyDescription,
			PyObject* pyData);
	
	/** 
		该entity通信通道
	*/
	INLINE void pChannel(Network::Channel* pchannel);
	INLINE Network::Channel* pChannel(void) const ;

public:
	/** 
		entityCall section
	*/
	INLINE EntityCall* baseEntityCall() const;
	DECLARE_PY_GET_MOTHOD(pyGetBaseEntityCall);
	INLINE void baseEntityCall(EntityCall* entityCall);
	
	INLINE EntityCall* clientEntityCall() const;
	DECLARE_PY_GET_MOTHOD(pyGetClientEntityCall);
	INLINE void clientEntityCall(EntityCall* entityCall);

	/**
		all_clients
	*/
	INLINE AllClients* allClients() const;
	DECLARE_PY_GET_MOTHOD(pyGetAllClients);
	INLINE void allClients(AllClients* clients);

	/**
		other_clients
	*/
	INLINE AllClients* otherClients() const;
	DECLARE_PY_GET_MOTHOD(pyGetOtherClients);
	INLINE void otherClients(AllClients* clients);

	/**
		脚本获取controlledBy属性
	*/
	INLINE bool isControlledNotSelfClient() const;
	INLINE EntityCall* controlledBy() const;
	INLINE void controlledBy(EntityCall* baseEntityCall);
	DECLARE_PY_GETSET_MOTHOD(pyGetControlledBy, pySetControlledBy);
	bool setControlledBy(EntityCall* baseEntityCall);
	void sendControlledByStatusMessage(EntityCall* baseEntityCall, int8 isControlled);

	/** 
		脚本获取和设置entity的position 
	*/
	INLINE Position3D& position();
	INLINE void position(const Position3D& pos);
	DECLARE_PY_GETSET_MOTHOD(pyGetPosition, pySetPosition);

	/** 
		脚本获取和设置entity的方向 
	*/
	INLINE Direction3D& direction();
	INLINE void direction(const Direction3D& dir);
	DECLARE_PY_GETSET_MOTHOD(pyGetDirection, pySetDirection);

	/**
		是否在地面上
	*/
	INLINE void isOnGround(bool v);
	INLINE bool isOnGround() const;
	DECLARE_PY_GET_MOTHOD(pyGetIsOnGround);

	/** 
		设置entity方向和位置 
	*/
	void setPositionAndDirection(const Position3D& pos, 
		const Direction3D& dir);
	
	void onPositionChanged();
	void onDirectionChanged();
	
	void onPyPositionChanged();
	void onPyDirectionChanged();
	
	void updateLastPos();

	bool checkMoveForTopSpeed(const Position3D& position);

	/** 网络接口
		客户端设置新位置
	*/
	void setPosition_XZ_int(Network::Channel* pChannel, int32 x, int32 z);

	/** 网络接口
		客户端设置新位置
	*/
	void setPosition_XYZ_int(Network::Channel* pChannel, int32 x, int32 y, int32 z);

	/** 网络接口
		客户端设置位置
	*/
	void setPosition_XZ_float(Network::Channel* pChannel, float x, float z);

	/** 网络接口
		客户端设置位置
	*/
	void setPosition_XYZ_float(Network::Channel* pChannel, float x, float y, float z);

	/**
		cell上的传送方法
	*/
	DECLARE_PY_MOTHOD_ARG3(pyTeleport, PyObject_ptr, PyObject_ptr, PyObject_ptr);
	void teleport(PyObject_ptr nearbyMBRef, Position3D& pos, Direction3D& dir);
	void teleportLocal(PyObject_ptr nearbyMBRef, Position3D& pos, Direction3D& dir);
	void teleportRefEntity(Entity* entity, Position3D& pos, Direction3D& dir);
	void teleportRefEntityCall(EntityCall* nearbyMBRef, Position3D& pos, Direction3D& dir);
	void onTeleportRefEntityCall(EntityCall* nearbyMBRef, Position3D& pos, Direction3D& dir);

	/**
		传送成功和失败相关回调
	*/
	void onTeleport();
	void onTeleportFailure();
	void onTeleportSuccess(PyObject* nearbyEntity, SPACE_ID lastSpaceID);
	void onReqTeleportOtherAck(Network::Channel* pChannel, ENTITY_ID nearbyMBRefID, 
		SPACE_ID destSpaceID, COMPONENT_ID componentID);

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
	void onEnterSpace(SpaceMemory* pSpace);
	void onLeaveSpace(SpaceMemory* pSpace);

	/** 
		当cellapp意外终止后， baseapp如果能找到合适的cellapp则将其恢复后
		会调用此方法
	*/
	void onRestore();

	/**
		脚本调试view
	*/
	void debugView();
	DECLARE_PY_MOTHOD_ARG0(pyDebugView);

	/** 
		当前entity设置自身的View半径范围 
	*/
	int32 setViewRadius(float radius, float hyst);
	float getViewRadius(void) const;
	float getViewHystArea(void) const;
	DECLARE_PY_MOTHOD_ARG2(pySetViewRadius, float, float);
	DECLARE_PY_MOTHOD_ARG0(pyGetViewRadius);
	DECLARE_PY_MOTHOD_ARG0(pyGetViewHystArea);

	/**
		返回观察该实体的所有观察者
	*/
	DECLARE_PY_MOTHOD_ARG0(pyGetWitnesses);

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
	INLINE int8 layer() const;
	DECLARE_PY_GETSET_MOTHOD(pyGetLayer, pySetLayer);

	/** 
		entity移动导航 
	*/
	bool canNavigate();
	uint32 navigate(const Position3D& destination, float velocity, float distance,
					float maxMoveDistance, float maxSearchDistance,
					bool faceMovement, int8 layer, PyObject* userData);
	bool navigatePathPoints(std::vector<Position3D>& outPaths, const Position3D& destination, float maxSearchDistance, int8 layer);

	DECLARE_PY_MOTHOD_ARG0(pycanNavigate);
	DECLARE_PY_MOTHOD_ARG3(pyNavigatePathPoints, PyObject_ptr, float, int8);
	DECLARE_PY_MOTHOD_ARG8(pyNavigate, PyObject_ptr, float, float, float, float, int8, int8, PyObject_ptr);

	/** 
		entity获得随机点 
	*/
	bool getRandomPoints(std::vector<Position3D>& outPoints, const Position3D& centerPos, float maxRadius, uint32 maxPoints, int8 layer);
	DECLARE_PY_MOTHOD_ARG4(pyGetRandomPoints, PyObject_ptr, float, uint32, int8);

	/** 
		entity移动到某个点 
	*/
	uint32 moveToPoint(const Position3D& destination, float velocity, float distance,
			PyObject* userData, bool faceMovement, bool moveVertically);
	
	DECLARE_PY_MOTHOD_ARG6(pyMoveToPoint, PyObject_ptr, float, float, PyObject_ptr, int32, int32);

	/** 
		entity移动到某个entity 
	*/
	uint32 moveToEntity(ENTITY_ID targetID, float velocity, float distance,
			PyObject* userData, bool faceMovement, bool moveVertically, const Position3D& offsetPos);
	
	static PyObject* __py_pyMoveToEntity(PyObject* self, PyObject* args);

	/**
	entity移动加速
	*/
	float accelerate(const char* type, float acceleration);
	DECLARE_PY_MOTHOD_ARG2(pyAccelerate, const_charptr, float);

	/** 
		脚本获取和设置entity的最高xz移动速度 
	*/
	float topSpeed() const{ return topSpeed_; }
	INLINE void topSpeed(float speed);
	DECLARE_PY_GETSET_MOTHOD(pyGetTopSpeed, pySetTopSpeed);
	
	/** 
		脚本获取和设置entity的最高y移动速度 
	*/
	INLINE float topSpeedY() const;
	INLINE void topSpeedY(float speed);
	DECLARE_PY_GETSET_MOTHOD(pyGetTopSpeedY, pySetTopSpeedY);
	
	/** 
		脚本请求获得一定范围内的某种类型的entities 
	*/
	static PyObject* __py_pyEntitiesInRange(PyObject* self, PyObject* args);

	/** 
		脚本请求获得View范围内的entities 
	*/
	static PyObject* __py_pyEntitiesInView(PyObject* self, PyObject* args);
	PyObject* entitiesInView(bool pending);

	/**
		设置获取是否自动备份
	*/
	INLINE int8 shouldAutoBackup() const;
	INLINE void shouldAutoBackup(int8 v);
	DECLARE_PY_GETSET_MOTHOD(pyGetShouldAutoBackup, pySetShouldAutoBackup);

	/** 网络接口
		远程呼叫本entity的方法 
	*/
	void onRemoteMethodCall(Network::Channel* pChannel, MemoryStream& s);
	void onRemoteCallMethodFromClient(Network::Channel* pChannel, ENTITY_ID srcEntityID, MemoryStream& s);
	void onRemoteMethodCall_(PropertyDescription* pComponentPropertyDescription, 
		MethodDescription* pMethodDescription, ENTITY_ID srcEntityID, MemoryStream& s);

	/**
		观察者
	*/
	INLINE Witness* pWitness() const;
	INLINE void pWitness(Witness* w);

	/** 
		是否被任何proxy监视到, 如果这个entity没有客户端， 则这个值有效 
	*/
	INLINE bool isWitnessed(void) const;
	DECLARE_PY_GET_MOTHOD(pyIsWitnessed);

	/** 
		entity是否是一个观察者 
	*/
	INLINE bool hasWitness(void) const;
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

	/**
		 指定的entity是否是观察自己的人之一
	*/
	bool entityInWitnessed(ENTITY_ID entityID);

	INLINE const std::list<ENTITY_ID>&	witnesses();
	INLINE size_t witnessesSize() const;

	/** 网络接口
		entity绑定了一个观察者(客户端)

	*/
	void setWitness(Witness* pWitness);
	void onGetWitnessFromBase(Network::Channel* pChannel);
	void onGetWitness(bool fromBase = false);

	/** 网络接口
		entity丢失了一个观察者(客户端)

	*/
	void onLoseWitness(Network::Channel* pChannel);

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
		调用客户端实体的方法  
	*/
	DECLARE_PY_MOTHOD_ARG1(pyClientEntity, ENTITY_ID);

	/** 
		恢复所有的范围触发器 
		在teleport时会出现这样的情况
	*/
	void restoreProximitys();

	/** 
		删除一个控制器 
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
		一个entity进入了View区域
	*/
	void onEnteredView(Entity* entity);

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
		entity转动朝向
	*/
	uint32 addYawRotator(float yaw, float velocity,
		PyObject* userData);

	DECLARE_PY_MOTHOD_ARG3(pyAddYawRotator, float, float, PyObject_ptr);
	
	/**
		entity转向完成
	*/
	void onTurn(uint32 controllerId, PyObject* userarg);
	
	/**
		获取自身在space的entities中的位置
	*/
	INLINE SPACE_ENTITIES::size_type spaceEntityIdx() const;
	INLINE void spaceEntityIdx(SPACE_ENTITIES::size_type idx);

	/**
		获取entity所在节点
	*/
	INLINE EntityCoordinateNode* pEntityCoordinateNode() const;
	INLINE void pEntityCoordinateNode(EntityCoordinateNode* pNode);

	/**
		安装卸载节点
	*/
	void installCoordinateNodes(CoordinateSystem* pCoordinateSystem);
	void uninstallCoordinateNodes(CoordinateSystem* pCoordinateSystem);
	void onCoordinateNodesDestroy(EntityCoordinateNode* pEntityCoordinateNode);

	/**
		获取entity位置朝向在某时间是否改变过
	*/
	INLINE GAME_TIME posChangedTime() const;
	INLINE GAME_TIME dirChangedTime() const;

	/** 
		real请求更新属性到ghost
	*/
	void onUpdateGhostPropertys(KBEngine::MemoryStream& s);
	
	/** 
		ghost请求调用def方法real
	*/
	void onRemoteRealMethodCall(KBEngine::MemoryStream& s);

	/** 
		real请求更新属性到ghost
	*/
	void onUpdateGhostVolatileData(KBEngine::MemoryStream& s);

	/** 
		转变为ghost, 自身必须为real
	*/
	void changeToGhost(COMPONENT_ID realCell, KBEngine::MemoryStream& s);

	/** 
		转变为real, 自身必须为ghost
	*/
	void changeToReal(COMPONENT_ID ghostCell, KBEngine::MemoryStream& s);

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

	void addTimersToStream(KBEngine::MemoryStream& s);
	void createTimersFromStream(KBEngine::MemoryStream& s);

	void addControllersToStream(KBEngine::MemoryStream& s);
	void createControllersFromStream(KBEngine::MemoryStream& s);

	void addWitnessToStream(KBEngine::MemoryStream& s);
	void createWitnessFromStream(KBEngine::MemoryStream& s);

	void addMovementHandlerToStream(KBEngine::MemoryStream& s);
	void createMovementHandlerFromStream(KBEngine::MemoryStream& s);
	
	void addEventsToStream(KBEngine::MemoryStream& s);
	void createEventsFromStream(KBEngine::MemoryStream& s);

	/** 
		获得实体控制器管理器
	*/
	INLINE Controllers*	pControllers() const;

	/** 
		设置实体持久化数据是否已脏，脏了会自动存档 
	*/
	INLINE void setDirty(uint32* digest = NULL);
	INLINE bool isDirty() const;
	
	/**
		VolatileInfo section
	*/
	INLINE VolatileInfo* pCustomVolatileinfo(void);
	DECLARE_PY_GETSET_MOTHOD(pyGetVolatileinfo, pySetVolatileinfo);

	/**
		调用实体的回调函数，有可能被缓存
	*/
	bool bufferOrExeCallback(const char * funcName, PyObject * funcArgs, bool notFoundIsOK = true);
	static void bufferCallback(bool enable);

private:
	/** 
		发送teleport结果到base端
	*/
	void _sendBaseTeleportResult(ENTITY_ID sourceEntityID, COMPONENT_ID sourceBaseAppID, 
		SPACE_ID spaceID, SPACE_ID lastSpaceID, bool fromCellTeleport);

private:
	struct BufferedScriptCall
	{
		EntityPtr		entityPtr;
		PyObject *		pyCallable;
		// 可以为NULL， NULL说明没有参数
		PyObject *		pyFuncArgs;
		const char*		funcName;
	};

	typedef std::list<BufferedScriptCall*>					BufferedScriptCallArray;
	static BufferedScriptCallArray							_scriptCallbacksBuffer;
	static int32											_scriptCallbacksBufferNum;
	static int32											_scriptCallbacksBufferCount;

protected:
	// 这个entity的客户端部分的entityCall
	EntityCall*												clientEntityCall_;

	// 这个entity的baseapp部分的entityCall
	EntityCall*												baseEntityCall_;

	/** 这个entity的坐标和朝向当前受谁的客户端控制
	    null表示没有客户端在控制（即系统控制），
	    否则指向控制这个entity的对象的baseEntityCall_，
		玩家自己控制自己则Entity.controlledBy = self.base
	*/
	EntityCall *											controlledBy_;

	// 如果一个entity为ghost，那么entity会存在一个源cell的指向
	COMPONENT_ID											realCell_;

	// 如果一个entity为real，那么entity可能会存在一个ghost的指向
	COMPONENT_ID											ghostCell_;	

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
	size_t													witnesses_count_;

	// 观察者对象
	Witness*												pWitness_;

	AllClients*												allClients_;
	AllClients*												otherClients_;

	// entity节点
	EntityCoordinateNode*									pEntityCoordinateNode_;	

	// 控制器管理器
	Controllers*											pControllers_;
	KBEShared_ptr<Controller>								pMoveController_;
	KBEShared_ptr<Controller>								pTurnController_;
	
	script::ScriptVector3::PYVector3ChangedCallback			pyPositionChangedCallback_;
	script::ScriptVector3::PYVector3ChangedCallback			pyDirectionChangedCallback_;
	
	// entity层，可以做任意表示，基于tile的游戏可以表示为海陆空等层，纯3d也可以表示各种层
	// 在脚本层做搜索的时候可以按层搜索.
	int8													layer_;
	
	// 需要持久化的数据是否变脏（内存sha1），如果没有变脏不需要持久化
	uint32													persistentDigest_[5];

	// 如果用户有设置过Volatileinfo，则此处创建Volatileinfo，否则为NULL使用ScriptDefModule的Volatileinfo
	VolatileInfo*											pCustomVolatileinfo_;
};

}

#ifdef CODE_INLINE
#include "entity.inl"
#endif
#endif // KBE_ENTITY_H
