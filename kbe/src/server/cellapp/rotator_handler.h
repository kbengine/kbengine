// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_RotatorHandler_H
#define KBE_RotatorHandler_H

#include "controller.h"
#include "updatable.h"
#include "pyscript/scriptobject.h"	
#include "math/math.h"	


namespace KBEngine{

class RotatorHandler : public Updatable
{
public:
	RotatorHandler(KBEShared_ptr<Controller> pController, const Direction3D& destDir, float velocity, PyObject* userarg);

	RotatorHandler();
	virtual ~RotatorHandler();

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

	virtual bool update();

	virtual const Direction3D& destDir();
	virtual bool requestTurnOver();
	void pController(KBEShared_ptr<Controller> pController){ pController_ = pController; }

	float velocity() const {
		return velocity_;
	}

	void velocity(float v) {
		velocity_ = v;
	}

protected:
	Direction3D destDir_;
	float velocity_;
	PyObject* pyuserarg_;
	KBEShared_ptr<Controller> pController_;
};
 
}
#endif // KBE_MOVETOENTITYHANDLER_H

