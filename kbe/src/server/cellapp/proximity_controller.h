// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_PROXIMITYCONTROLLER_H
#define KBE_PROXIMITYCONTROLLER_H

#include "controller.h"	


namespace KBEngine{

class TrapTrigger;
class CoordinateNode;

/*
	π‹¿Ìtrap°£
*/
class ProximityController : public Controller
{
public:
	ProximityController(Entity* pEntity, float xz, float y, int32 userarg, uint32 id = 0);
	ProximityController(Entity* pEntity);
	~ProximityController();
	
	bool reinstall(CoordinateNode* pCoordinateNode);

	void onEnter(Entity* pEntity, float xz, float y);
	void onLeave(Entity* pEntity, float xz, float y);

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

protected:
	TrapTrigger* pTrapTrigger_;
	float xz_; 
	float y_;
};

}
#endif // KBE_PROXIMITYCONTROLLER_H
