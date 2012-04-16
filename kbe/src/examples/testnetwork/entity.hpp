/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __ENTITY_H__
#define __ENTITY_H__
	
// common include
//#include "entitymovecontroller.hpp"
//#include "timer.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
//#include "entityMailbox.hpp"
#include "pyscript/math.hpp"
#include "pyscript/scriptobject.hpp"
#include "entitydef/datatypes.hpp"	
#include "entitydef/entitydef.hpp"	
//#define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{
class Chunk;
class Entity;

/** 观察者信息结构 */
struct WitnessInfo
{
	WitnessInfo(const int8& lv, Entity* e, const float& r):
	detailLevel(lv),
	entity(e),
	range(r)
	{
		for(int i=0; i<3; i++)
			if(lv == i)
				detailLevelLog[i] = true;
			else
				detailLevelLog[i] = false;
	}
	
	int8 detailLevel;							// 当前所在详情级别
	Entity* entity;								// 所表达的entity
	float range;								// 当前与这个entity的距离
	bool detailLevelLog[3];						// 表示这个entity都进入过该entity的哪些详情级别， 提供属性广播优化用的
												// 当没有进入过某级别时， 会将所有这个级别的属性更新给他， 否则只更新近段时间曾经改变过的属性
	std::vector<uint32> changeDefDataLogs[3];	// entity离开了某个详情级别(没有脱离witness)后， 这期间有某个详情级别的属性改变均记录在这里
};

class Entity : public script::ScriptObject
{
	/** 子类化 将一些py操作填充进派生类 */
	BASE_SCRIPT_HREADER(Entity, ScriptObject)	
public:
	typedef std::map<ENTITY_ID, WitnessInfo*>	WITNESSENTITY_DETAILLEVEL_MAP;
public:
	Entity(ENTITY_ID id, ScriptModule* scriptModule);
	~Entity();

	/** entity脚本初始化 */
	void initializeScript();

	/** 创建这个entity的名字空间内的字典数据 */
	void createNamespace(PyObject* dictData);

	/** 获得这个entity cell部分定义的指定flags所有属性的数据, 返回值是一个pyDict */
	PyObject* getCellDataByFlags(uint32 flags);
	/** 获得这个entity cell部分定义的指定详情级别的所有属性的数据 */
	void getCellDataByDetailLevel(const int8& detailLevel, MemoryStream* mstream);
	
	/** 销毁这个entity */
	void onDestroy(void);
	void destroy();
	
	/** 判断自身是否是一个realEntity */
	inline bool isReal(void)const{ return m_isReal_; }
	
	/** 脚本请求设置属性或者方法 */
	int onScriptSetAttribute(PyObject* attr, PyObject * value);	
	
	/** 脚本请求获取属性或者方法 */
	PyObject* onScriptGetAttribute(PyObject* attr);	
			
	/** 脚本请求删除一个属性 */
	int onScriptDelAttribute(PyObject* attr);

	/** onTimer被触发 */
	//void onTimer(const TIMER_ID& timerID, TimerArgsBase* args);
	
	/** 获得脚本名称 */
	const char* getScriptModuleName(void)const{ return m_scriptModule_->getScriptType()->tp_name; }	
	
	/** 获取这个entity的脚本模块封装对象 */
	ScriptModule* getScriptModule(void)const{ return m_scriptModule_; }
	
	/** 支持pickler 方法 */
	static PyObject* __reduce_ex__(PyObject* self, PyObject* protocol);
	
	/** 定义属性数据被改变了 */
	void onDefDataChanged(PropertyDescription* propertyDescription, PyObject* pyData);
public:
	/** 获得entity的ID */
	ENTITY_ID getID()const{ return m_id_; }
	void setID(const int& id){ m_id_ = id; }
	static PyObject* pyGetID(Entity *self, void *closure){ return PyLong_FromLong(self->getID()); }

	/** 获得entity的所在space的ID */
	uint32 getSpaceID()const{ return m_spaceID_; }
	void setSpaceID(int id){ m_spaceID_ = id; }
	static PyObject* pyGetSpaceID(Entity *self, void *closure){ return PyLong_FromLong(self->getSpaceID()); }

	/** 脚本请求销毁实体 */
	void destroyEntity(void);
	static PyObject* pyDestroyEntity(PyObject* self, PyObject* args, PyObject* kwds);
	
	/** 脚本获取mailbox */
	/*
	EntityMailbox* getBaseMailbox()const{ return m_baseMailbox_; }
	static PyObject* pyGetBaseMailbox(Entity *self, void *closure);
	void setBaseMailbox(EntityMailbox* mailbox){ m_baseMailbox_ = mailbox; }
	*/
	/** 脚本获取mailbox */
	/*
	EntityMailbox* getClientMailbox()const{ return m_clientMailbox_; }
	static PyObject* pyGetClientMailbox(Entity *self, void *closure);
	void setClientMailbox(EntityMailbox* mailbox){ m_clientMailbox_ = mailbox; if(m_clientMailbox_!= NULL) onGetWitness(); else onLoseWitness(); }
	*/
	/** 脚本获取和设置entity的position */
	Position3D& getPosition(){ return m_position_; }
	static PyObject* pyGetPosition(Entity *self, void *closure);
	static int pySetPosition(Entity *self, PyObject *value, void *closure);
	void setPosition(Position3D& pos);

	/** 脚本获取和设置entity的方向 */
	Direction3D& getDirection(){ return m_direction_; }
	static PyObject* pyGetDirection(Entity *self, void *closure);
	static int pySetDirection(Entity *self, PyObject *value, void *closure);
	void setDirection(Direction3D& dir){ m_direction_ = dir; }
	
	/** 设置entity方向和位置 */
	void setPositionAndDirection(Position3D& position, Direction3D& direction);

	/** 添加和删除一个timer */
	PyObject* addTimer(uint32 startTrrigerIntervalTime, uint32 loopTrrigerIntervalTime, PyObject* args);
	PyObject* delTimer(TIMER_ID timerID);
	static PyObject* pyAddTimer(PyObject* self, PyObject* args, PyObject* kwds);
	static PyObject* pyDelTimer(PyObject* self, PyObject* args, PyObject* kwds);
	
	/** 脚本请求为当前所在space设置一个几何映射 */
	static PyObject* pyAddSpaceGeometryMapping(PyObject* self, PyObject* args, PyObject* kwds);

	/** 当前entity设置自身的Aoi半径范围 */
	PyObject* setAoiRadius(float radius, float hyst);
	float getAoiRadius(void)const{ return m_aoiRadius_; }
	float getAoiHystArea(void)const{ return m_aoiHysteresisArea_; }
	static PyObject* pySetAoiRadius(PyObject* self, PyObject* args, PyObject* kwds);
	
	/** 当前entity是否为real */
	static PyObject* pyIsReal(PyObject* self, PyObject* args, PyObject* kwds);
	
	/** 脚本获得当前entity是否为将要销毁的entity */
	bool isDestroyed(){ return m_isDestroyed_; }
	static PyObject* pyGetIsDestroyed(Entity *self, void *closure);
	
	/** entity移动导航 */
	bool navigateStep(Position3D& destination, float& velocity, float& maxMoveDistance, float& maxDistance, 
					bool faceMovement, float& girth, PyObject* userData);
	static PyObject* pyNavigateStep(PyObject* self, PyObject* args, PyObject* kwds);
	
	/** 停止任何方式的移动行为 */
	bool stopMove();
	static PyObject* pyStopMove(PyObject* self, PyObject* args, PyObject* kwds);

	/** entity移动到某个点 */
	bool moveToPoint(Position3D& destination, float& velocity, PyObject* userData, bool faceMovement, bool moveVertically);
	static PyObject* pyMoveToPoint(PyObject* self, PyObject* args, PyObject* kwds);

	
	/** 脚本获取和设置entity的最高xz移动速度 */
	float getTopSpeed()const{ return m_topSpeed_; }
	static PyObject* pyGetTopSpeed(Entity *self, void *closure){ return PyFloat_FromDouble(self->getTopSpeed()); };
	static int pySetTopSpeed(Entity *self, PyObject *value, void *closure){ self->setTopSpeed(float(PyFloat_AsDouble(PySequence_GetItem(value, 0)))); return 0; };
	void setTopSpeed(float speed){ m_topSpeed_ = speed; }
	
	/** 脚本获取和设置entity的最高y移动速度 */
	float getTopSpeedY()const{ return m_topSpeedY_; }
	static PyObject* pyGetTopSpeedY(Entity *self, void *closure){ return PyFloat_FromDouble(self->getTopSpeedY()); };
	static int pySetTopSpeedY(Entity *self, PyObject *value, void *closure){ self->setTopSpeedY(float(PyFloat_AsDouble(PySequence_GetItem(value, 0)))); return 0; };
	void setTopSpeedY(float speed){ m_topSpeedY_ = speed; }
	
	/** 脚本请求获得一定范围类的某种类型的entities */
	static PyObject* pyEntitiesInRange(PyObject* self, PyObject* args, PyObject* kwds);
public:
	/** 远程呼叫本entity的方法 */
	//void onRemoteMethodCall(SocketPacket& recvPacket);
	/** 收到邮件 */
	//void onReceiveMail(MAIL_TYPE& mailType, SocketPacket& recvPacket);

	/** 设置这个entity当前所在chunk的ID */
	//void setCurrentChunk(Chunk* chunk){ Chunk* oldchunk = m_currChunk_; m_currChunk_ = chunk; onCurrentChunkChanged(oldchunk); }
	/** 当前chunk被改变 */
	//void onCurrentChunkChanged(Chunk* oldChunk);
	/** 获得这个entity当前所在chunk的ID */
	//Chunk* getAtChunk(void)const{ return m_currChunk_; }

	/** 是否被任何proxy监视到, 如果这个entity没有客户端， 则这个值有效 */
	bool isWitnessed(void)const{ return m_isWitnessed_; }
	static PyObject* pyIsWitnessed(Entity *self, void *closure);
	/** entity是否是一个观察者 */
	bool hasWitness(void)const{ return m_hasWitness_; }
	static PyObject* pyHasWitness(Entity *self, void *closure);
	/** 自身被一个观察者观察到了 */
	void onWitnessed(Entity* entity, const float& range);
	/** 移除一个观察自身的观察者 */
	void onRemoveWitness(Entity* entity);
	/** 这个entity获得了观察者身份 */
	void onGetWitness(void);
	/** entity丢失了观察者身份 */
	void onLoseWitness(void);
	/** 获得所观察到的entities列表 */
	std::map<ENTITY_ID, Entity*>& getViewEntities(void){ return m_viewEntities_; }
	/** 获得所有观察到我的观察者 */
	WITNESSENTITY_DETAILLEVEL_MAP& getWitnessEntities(void){ return m_witnessEntityDetailLevelMap_; }
	/** 更新witness的状态 */
	void onUpdateWitness(Entity* entity, const float& range);
	/** 一个新进入视野范围的entity */
	void onViewEntity(Entity* entity);
	/** 一个entity离开了视野范围 */
	void onLoseViewEntity(Entity* entity);
	/** 一个entity第一次被设置(第一次进入到这个区域中)detailLevel级别 */
	void onEntityInitDetailLevel(Entity* entity, int8 detailLevel);
	/** 一个entity因为移动改变了它在本entity的detailLevel的级别 */
	void onEntityDetailLevelChanged(WitnessInfo* witnessInfo, const int8& oldDetailLevel, const int8& newDetailLevel);
	
	/** 添加一个陷阱 */
	uint16 addProximity(float range);
	static PyObject* pyAddProximity(PyObject* self, PyObject* args, PyObject* kwds);
	/** 删除一个陷阱 */
	void delProximity(const uint16& id);
	static PyObject* pyDelProximity(PyObject* self, PyObject* args, PyObject* kwds);
	/** 一个entity进入了这个entity的某个陷阱 */
	void onEnterTrap(Entity* entity, const float& range, const int& controllerID);
	/** 一个entity离开了这个entity的某个陷阱 */
	void onLeaveTrap(Entity* entity, const float& range, const int& controllerID);
	/** 当entity跳到一个新的space上去后，离开陷阱陷阱事件将触发这个接口 */
	void onLeaveTrapID(const ENTITY_ID& entityID, const float& range, const int& controllerID);
	/** 获得陷阱管理器 */
//	ProximityMgr& getTrapMgr(void){ return m_trapMgr_; }

	/** entity的一次移动完成 */
	void onMove(PyObject* userData);
protected:
	ENTITY_ID								m_id_;								// id号码
	ScriptModule*							m_scriptModule_;					// 该entity所使用的脚本模块对象
	ScriptModule::PROPERTYDESCRIPTION_MAP*	m_lpPropertyDescrs_;				// 属于这个entity的属性描述
	uint32									m_spaceID_;							// 这个entity所在space的ID
	//EntityMailbox*							m_clientMailbox_;					// 这个entity的客户端mailbox
	//EntityMailbox*							m_baseMailbox_;						// 这个entity的baseapp mailbox
	Position3D								m_position_;						// entity的当前位置
	Direction3D								m_direction_;						// entity的当前方向
//	TimerFunc								m_TimerFunc_;						// onTimer函数地址
//	Timer									m_timers_;							// timers管理器
//	Chunk*									m_currChunk_;						// 这个当前entity所在的chunk
	bool									m_isReal_;							// 自己是否是一个realEntity
	bool									m_isDestroyed_;						// 自身是否将要销毁
	float									m_aoiRadius_;						// 当前entity的aoi半径
	float									m_aoiHysteresisArea_;				// 当前entityAoi的一个滞后范围
	bool									m_isWitnessed_;						// 是否被任何观察者监视到
	bool									m_hasWitness_;						// 这个entity是否是一个观察者
	std::map<ENTITY_ID, Entity*>			m_viewEntities_;					// 自身视野范围内的entityID， entity必须是一个观察者才有这个属性存在
	WITNESSENTITY_DETAILLEVEL_MAP			m_witnessEntityDetailLevelMap_;		// 这个变量记录了一个观察者在当前entity的详情级别
	std::map<ENTITY_ID, Entity*>			m_witnessEntities_[4];				// 观察者entity的ID列表， 被保存在一个详情级别队列里，详情级别总共4级， 分：近中远, 超远
//	ProximityMgr							m_trapMgr_;							// entity陷阱管理器
	float									m_topSpeed_;						// entity x,z轴最高移动速度
	float									m_topSpeedY_;						// entity y轴最高移动速度
};

}
#endif
