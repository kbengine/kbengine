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

#ifndef __KBE_WITNESS_HPP__
#define __KBE_WITNESS_HPP__

// common include
#include "updatable.hpp"
#include "entityref.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/objectpool.hpp"

// #define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32	
#else
// linux include
#endif

namespace KBEngine{

namespace Mercury
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

/**
	这个模块用来监视我们感兴趣的entity数据， 如：aoi， 属性更新， 调用entity的方法
	并将其传输给监视者。
*/
class Witness : public PoolObject, public Updatable
{
public:
	Witness();
	~Witness();

	typedef KBEShared_ptr< SmartPoolObject< Witness > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj();

	static ObjectPool<Witness>& ObjPool();
	void onReclaimObject();

	INLINE Entity* pEntity();

	void attach(Entity* pEntity);
	void detach(Entity* pEntity);

	void setAoiRadius(float radius, float hyst = 5.0f);
	
	INLINE float aoiRadius()const;
	INLINE float aoiHysteresisArea()const;

	typedef std::vector<Mercury::Bundle*> Bundles;
	Bundles* pBundles();

	bool update();
	
	void onEnterSpace(Space* pSpace);
	void onLeaveSpace(Space* pSpace);

	void onEnterAOI(Entity* pEntity);
	void onLeaveAOI(Entity* pEntity);

	/**
		向witness客户端推送一条消息
	*/
	bool sendToClient(const Mercury::MessageHandler& msgHandler, Mercury::Bundle* pBundle);

	typedef std::vector<EntityRef*> AOI_ENTITIES;
private:
	Entity*									pEntity_;

	float									aoiRadius_;							// 当前entity的aoi半径
	float									aoiHysteresisArea_;					// 当前entityAoi的一个滞后范围

	AOITrigger*								pAOITrigger_;

	AOI_ENTITIES							aoiEntities_;

};

}

#ifdef CODE_INLINE
#include "witness.ipp"
#endif
#endif
