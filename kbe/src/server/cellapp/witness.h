/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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
class AOITrigger;
class Space;

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
	这个模块用来监视我们感兴趣的entity数据， 如：aoi， 属性更新， 调用entity的方法
	并将其传输给监视者。
*/
class Witness : public PoolObject, public Updatable
{
public:
	Witness();
	~Witness();
	
	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

	virtual std::string c_str(){ return "Witness"; }

	typedef KBEShared_ptr< SmartPoolObject< Witness > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj();

	static ObjectPool<Witness>& ObjPool();
	void onReclaimObject();

	virtual size_t getPoolObjectBytes()
	{
		size_t bytes = sizeof(pEntity_)
		 + sizeof(aoiRadius_) + sizeof(aoiHysteresisArea_)
		 + sizeof(pAOITrigger_) + sizeof(pAOIHysteresisAreaTrigger_) + sizeof(clientAOISize_)
		 + sizeof(lastBasePos) + (sizeof(EntityRef*) * aoiEntities_.size());

		return bytes;
	}

	INLINE void pEntity(Entity* pEntity);
	INLINE Entity* pEntity();

	void attach(Entity* pEntity);
	void detach(Entity* pEntity);
	void clear(Entity* pEntity);
	void onAttach(Entity* pEntity);

	void setAoiRadius(float radius, float hyst = 5.0f);
	
	INLINE float aoiRadius() const;
	INLINE float aoiHysteresisArea() const;

	typedef std::vector<Network::Bundle*> Bundles;
	bool pushBundle(Network::Bundle* pBundle);

	/**
		基础位置， 如果有坐骑基础位置可能是坐骑等
	*/
	INLINE const Position3D& basePos();

	bool update();
	
	void onEnterSpace(Space* pSpace);
	void onLeaveSpace(Space* pSpace);

	void onEnterAOI(AOITrigger* pAOITrigger, Entity* pEntity);
	void onLeaveAOI(AOITrigger* pAOITrigger, Entity* pEntity);
	void _onLeaveAOI(EntityRef* pEntityRef);

	/**
		写Volatile数据到流
	*/
	uint32 addEntityVolatileDataToStream(MemoryStream* mstream, Entity* otherEntity);
	

	void addSmartAOIEntityMessageToBundle(Network::Bundle* pBundle, const Network::MessageHandler& normalMsgHandler, 
		const Network::MessageHandler& optimizedMsgHandler, ENTITY_ID entityID);

	bool entityID2AliasID(ENTITY_ID id, uint8& aliasID) const;

	/**
		使用何种协议来更新客户端
	*/
	void addUpdateHeadToStream(Network::Bundle* pForwardBundle, uint32 flags, EntityRef* pEntityRef);

	/**
		添加基础位置到更新包
	*/
	void addBasePosToStream(Network::Bundle* pSendBundle);

	/**
		向witness客户端推送一条消息
	*/
	bool sendToClient(const Network::MessageHandler& msgHandler, Network::Bundle* pBundle);

	INLINE EntityRef::AOI_ENTITIES& aoiEntities();

	/** 获得aoientity的引用 */
	INLINE EntityRef* getAOIEntityRef(ENTITY_ID entityID);

	/** entityID是否在aoi内 */
	INLINE bool entityInAOI(ENTITY_ID entityID);

	INLINE AOITrigger* pAOITrigger();
	INLINE AOITrigger* pAOIHysteresisAreaTrigger();
	
	void installAOITrigger();
	void uninstallAOITrigger();

	/**
		重置AOI范围内的entities， 使其同步状态恢复到最初未同步的状态
	*/
	void resetAOIEntities();

private:
	/**
		如果aoi中entity数量小于256则只发送索引位置
	*/
	INLINE void _addAOIEntityIDToStream(MemoryStream* mstream, EntityRef* entityRef);
	INLINE void _addAOIEntityIDToBundle(Network::Bundle* pBundle, EntityRef* entityRef);
	INLINE void _addAOIEntityIDToBundle(Network::Bundle* pBundle, ENTITY_ID entityID);

private:
	Entity*									pEntity_;

	// 当前entity的aoi半径
	float									aoiRadius_;
	// 当前entityAoi的一个滞后范围
	float									aoiHysteresisArea_;

	AOITrigger*								pAOITrigger_;
	AOITrigger*								pAOIHysteresisAreaTrigger_;

	EntityRef::AOI_ENTITIES					aoiEntities_;

	Position3D								lastBasePos;

	uint16									clientAOISize_;
};

}

#ifdef CODE_INLINE
#include "witness.inl"
#endif
#endif
