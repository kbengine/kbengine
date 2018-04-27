// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_MOVETOPOINTCONTROLLERBASE_H
#define KBE_MOVETOPOINTCONTROLLERBASE_H

#include "controller.h"
#include "updatable.h"
#include "moveto_point_handler.h"
#include "pyscript/scriptobject.h"	

namespace KBEngine{

class MoveController : public Controller
{
public:
	MoveController(Entity* pEntity, MoveToPointHandler* pMoveToPointHandler = NULL, uint32 id = 0);
	virtual ~MoveController();
	
	void pMoveToPointHandler(MoveToPointHandler* pMoveToPointHandler)
		{ pMoveToPointHandler_ = pMoveToPointHandler; }

	virtual void destroy();
	virtual void addToStream(KBEngine::MemoryStream& s);
	virtual void createFromStream(KBEngine::MemoryStream& s);

	float velocity() const {
		return pMoveToPointHandler_->velocity();
	}

	void velocity(float v) {
		pMoveToPointHandler_->velocity(v);
	}

protected:
	MoveToPointHandler* pMoveToPointHandler_;
};
 
}
#endif // KBE_MOVETOPOINTCONTROLLERBASE_H

