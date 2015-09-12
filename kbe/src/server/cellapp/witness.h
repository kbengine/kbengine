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

/** �۲�����Ϣ�ṹ */
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
	
	int8 detailLevel;							// ��ǰ�������鼶��
	Entity* entity;								// ������entity
	float range;								// ��ǰ�����entity�ľ���
	bool detailLevelLog[3];						// ��ʾ���entity���������entity����Щ���鼶�� �ṩ���Թ㲥�Ż��õ�
												// ��û�н����ĳ����ʱ�� �Ὣ���������������Ը��¸����� ����ֻ���½���ʱ�������ı��������
	std::vector<uint32> changeDefDataLogs[3];	// entity�뿪��ĳ�����鼶��(û������witness)�� ���ڼ���ĳ�����鼶������Ըı����¼������
};

/**
	���ģ�������������Ǹ���Ȥ��entity���ݣ� �磺aoi�� ���Ը��£� ����entity�ķ���
	�����䴫��������ߡ�
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
		  + sizeof(pAOITrigger_) + sizeof(clientAOISize_)
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
		����λ�ã� ������������λ�ÿ����������
	*/
	INLINE const Position3D& basePos();

	bool update();
	
	void onEnterSpace(Space* pSpace);
	void onLeaveSpace(Space* pSpace);

	void onEnterAOI(Entity* pEntity);
	void onLeaveAOI(Entity* pEntity);
	void _onLeaveAOI(EntityRef* pEntityRef);

	/**
		дVolatile���ݵ���
	*/
	uint32 addEntityVolatileDataToStream(MemoryStream* mstream, Entity* otherEntity);
	

	void addSmartAOIEntityMessageToBundle(Network::Bundle* pBundle, const Network::MessageHandler& normalMsgHandler, 
		const Network::MessageHandler& optimizedMsgHandler, ENTITY_ID entityID);

	bool entityID2AliasID(ENTITY_ID id, uint8& aliasID) const;

	/**
		ʹ�ú���Э�������¿ͻ���
	*/
	void addUpdateHeadToStream(Network::Bundle* pForwardBundle, uint32 flags, EntityRef* pEntityRef);

	/**
		��ӻ���λ�õ����°�
	*/
	void addBasePosToStream(Network::Bundle* pSendBundle);

	/**
		��witness�ͻ�������һ����Ϣ
	*/
	bool sendToClient(const Network::MessageHandler& msgHandler, Network::Bundle* pBundle);

	INLINE EntityRef::AOI_ENTITIES& aoiEntities();

	/** ���aoientity������ */
	INLINE EntityRef* getAOIEntityRef(ENTITY_ID entityID);

	/** entityID�Ƿ���aoi�� */
	INLINE bool entityInAOI(ENTITY_ID entityID);

	INLINE AOITrigger* pAOITrigger();

	/**
		����AOI��Χ�ڵ�entities�� ʹ��ͬ��״̬�ָ������δͬ����״̬
	*/
	void resetAOIEntities();

private:
	/**
		���aoi��entity����С��256��ֻ��������λ��
	*/
	INLINE void _addAOIEntityIDToStream(MemoryStream* mstream, EntityRef* entityRef);
	INLINE void _addAOIEntityIDToBundle(Network::Bundle* pBundle, EntityRef* entityRef);
	INLINE void _addAOIEntityIDToBundle(Network::Bundle* pBundle, ENTITY_ID entityID);

private:
	Entity*									pEntity_;

	float									aoiRadius_;							// ��ǰentity��aoi�뾶
	float									aoiHysteresisArea_;					// ��ǰentityAoi��һ���ͺ�Χ

	AOITrigger*								pAOITrigger_;

	EntityRef::AOI_ENTITIES					aoiEntities_;

	Position3D								lastBasePos;

	uint16									clientAOISize_;

};

}

#ifdef CODE_INLINE
#include "witness.inl"
#endif
#endif
