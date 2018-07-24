// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_TURNENTITYCONTROLLERBASE_H
#define KBE_TURNENTITYCONTROLLERBASE_H

#include "controller.h"
#include "updatable.h"
#include "rotator_handler.h"
#include "pyscript/scriptobject.h"	

namespace KBEngine{

class TurnController : public Controller
{
public:
	TurnController(Entity* pEntity, RotatorHandler* pRotatorHandler = NULL, uint32 id = 0);
	virtual ~TurnController();
	
	void pRotatorHandler(RotatorHandler* pRotatorHandler)
	{
		pRotatorHandler_ = pRotatorHandler;
	}
	
	virtual void destroy();
	virtual void addToStream(KBEngine::MemoryStream& s);
	virtual void createFromStream(KBEngine::MemoryStream& s);

	float velocity() const {
		return pRotatorHandler_->velocity();
	}

	void velocity(float v) {
		pRotatorHandler_->velocity(v);
	}

protected:
	RotatorHandler* pRotatorHandler_;
};

}
#endif // KBE_TURNENTITYCONTROLLERBASE_H

