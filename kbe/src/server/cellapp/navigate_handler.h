// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NAVIGATEHANDLER_H
#define KBE_NAVIGATEHANDLER_H

#include "move_controller.h"	
#include "math/math.h"
#include "navigation/navigation_handle.h"

namespace KBEngine{

class NavigateHandler : public MoveToPointHandler
{
public:
	NavigateHandler(KBEShared_ptr<Controller>& pController, const Position3D& destPos, float velocity, float distance, bool faceMovement, 
		float maxMoveDistance, VECTOR_POS3D_PTR paths_ptr,
		PyObject* userarg);

	NavigateHandler();
	virtual ~NavigateHandler();
	
	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

	virtual bool requestMoveOver(const Position3D& oldPos);

	virtual bool isOnGround(){ return true; }

	virtual MoveType type() const { return MOVE_TYPE_NAV; }

protected:
	int destPosIdx_;
	VECTOR_POS3D_PTR paths_;

	float maxMoveDistance_;
};
 
}
#endif // KBE_NAVIGATEHANDLER_H

