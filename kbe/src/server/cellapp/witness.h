// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_WITNESS_H
#define KBE_WITNESS_H

// common include
#include "updatable.h"
#include "entityref.h"
#include "helper/debug_helper.h"
#include "common/common.h"
#include "common/objectpool.h"
#include "math/math.h"

// #define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32	
#else
// linux include
#endif

namespace KBEngine{

namespace Network
{
	class Bundle;
	class MessageHandler;
}

class Entity;
class MemoryStream;
class ViewTrigger;
class SpaceMemory;

/** 观察者信息结构 */
struct WitnessInfo
{
	WitnessInfo(const int8& lv, Entity* e, const float& r):
	detailLevel(lv),
	entity(e),
	range(r)
	{
		for(int i=0; i<3; ++i)
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

/**
	这个模块用来监视我们感兴趣的entity数据， 如：view， 属性更新， 调用entity的方法
	并将其传输给监视者。
*/
class Witness : public PoolObject, public Updatable
{
public:
	typedef std::list<EntityRef*> VIEW_ENTITIES;
	typedef std::map<ENTITY_ID, EntityRef*> VIEW_ENTITIES_MAP;

	Witness();
	~Witness();
	
	virtual uint8 updatePriority() const {
		return 1;
	}

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

	typedef KBEShared_ptr< SmartPoolObject< Witness > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj(const std::string& logPoint);

	static ObjectPool<Witness>& ObjPool();
	static Witness* createPoolObject(const std::string& logPoint);
	static void reclaimPoolObject(Witness* obj);
	static void destroyObjPool();
	void onReclaimObject();

	virtual size_t getPoolObjectBytes()
	{
		size_t bytes = sizeof(pEntity_)
		 + sizeof(viewRadius_) + sizeof(viewHysteresisArea_)
		 + sizeof(pViewTrigger_) + sizeof(pViewHysteresisAreaTrigger_) + sizeof(clientViewSize_)
		 + sizeof(lastBasePos_) + (sizeof(EntityRef*) * viewEntities_map_.size());

		return bytes;
	}

	INLINE void pEntity(Entity* pEntity);
	INLINE Entity* pEntity();

	void attach(Entity* pEntity);
	void detach(Entity* pEntity);
	void clear(Entity* pEntity);
	void onAttach(Entity* pEntity);

	void setViewRadius(float radius, float hyst = 5.0f);
	
	INLINE float viewRadius() const;
	INLINE float viewHysteresisArea() const;

	typedef std::vector<Network::Bundle*> Bundles;
	bool pushBundle(Network::Bundle* pBundle);

	/**
		基础位置， 如果有坐骑基础位置可能是坐骑等
	*/
	INLINE const Position3D& basePos();

	/**
	基础朝向， 如果有坐骑基础朝向可能是坐骑等
	*/
	INLINE const Direction3D& baseDir();

	bool update();
	
	void onEnterSpace(SpaceMemory* pSpace);
	void onLeaveSpace(SpaceMemory* pSpace);

	void onEnterView(ViewTrigger* pViewTrigger, Entity* pEntity);
	void onLeaveView(ViewTrigger* pViewTrigger, Entity* pEntity);
	void _onLeaveView(EntityRef* pEntityRef);

	/**
		获得实体本次同步Volatile数据的标记
	*/
	uint32 getEntityVolatileDataUpdateFlags(Entity* otherEntity);
	

	const Network::MessageHandler& getViewEntityMessageHandler(const Network::MessageHandler& normalMsgHandler, 
											   const Network::MessageHandler& optimizedMsgHandler, ENTITY_ID entityID, int& ialiasID);

	bool entityID2AliasID(ENTITY_ID id, uint8& aliasID);

	/**
		使用何种协议来更新客户端
	*/
	void addUpdateToStream(Network::Bundle* pForwardBundle, uint32 flags, EntityRef* pEntityRef);

	/**
		添加基础位置到更新包
	*/
	void addBaseDataToStream(Network::Bundle* pSendBundle);

	/**
		向witness客户端推送一条消息
	*/
	bool sendToClient(const Network::MessageHandler& msgHandler, Network::Bundle* pBundle);
	Network::Channel* pChannel();
		
	INLINE VIEW_ENTITIES_MAP& viewEntitiesMap();
	INLINE VIEW_ENTITIES& viewEntities();

	/** 获得viewentity的引用 */
	INLINE EntityRef* getViewEntityRef(ENTITY_ID entityID);

	/** entityID是否在view内 */
	INLINE bool entityInView(ENTITY_ID entityID);

	INLINE ViewTrigger* pViewTrigger();
	INLINE ViewTrigger* pViewHysteresisAreaTrigger();
	
	void installViewTrigger();
	void uninstallViewTrigger();

	/**
		重置View范围内的entities， 使其同步状态恢复到最初未同步的状态
	*/
	void resetViewEntities();

private:
	/**
		如果view中entity数量小于256则只发送索引位置
	*/
	INLINE void _addViewEntityIDToBundle(Network::Bundle* pBundle, EntityRef* pEntityRef);
	
	/**
		当update执行时view列表有改变的时候需要更新entityRef的aliasID
	*/
	void updateEntitiesAliasID();
		
private:
	Entity*									pEntity_;

	// 当前entity的view半径
	float									viewRadius_;
	// 当前entityView的一个滞后范围
	float									viewHysteresisArea_;

	ViewTrigger*							pViewTrigger_;
	ViewTrigger*							pViewHysteresisAreaTrigger_;

	VIEW_ENTITIES							viewEntities_;
	VIEW_ENTITIES_MAP						viewEntities_map_;

	Position3D								lastBasePos_;
	Direction3D								lastBaseDir_;

	uint16									clientViewSize_;
};

}

#ifdef CODE_INLINE
#include "witness.inl"
#endif
#endif
