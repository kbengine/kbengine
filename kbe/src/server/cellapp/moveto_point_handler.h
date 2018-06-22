// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_MOVETOPOINTHANDLER_H
#define KBE_MOVETOPOINTHANDLER_H

#include "controller.h"
#include "updatable.h"
#include "pyscript/scriptobject.h"	
#include "math/math.h"

namespace KBEngine{

class MoveToPointHandler : public Updatable
{
public:
	enum MoveType
	{
		MOVE_TYPE_POINT = 0,		// 常规类型
		MOVE_TYPE_ENTITY = 1,		// 范围触发器类型
		MOVE_TYPE_NAV = 2,			// 移动控制器类型
	};

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

	MoveToPointHandler(KBEShared_ptr<Controller>& pController, int layer, const Position3D& destPos, float velocity, float distance, bool faceMovement, 
		bool moveVertically, PyObject* userarg);

	MoveToPointHandler();
	virtual ~MoveToPointHandler();
	
	virtual bool update();

	virtual const Position3D& destPos() { return destPos_; }
	virtual bool requestMoveOver(const Position3D& oldPos);

	virtual bool isOnGround() { return false; }

	virtual MoveType type() const { return MOVE_TYPE_POINT; }

	void destroy() { isDestroyed_ = true; }

	float velocity() const {
		return velocity_;
	}

	void velocity(float v) {
		velocity_ = v;
	}

protected:
	Position3D destPos_;
	float velocity_;			// 速度
	bool faceMovement_;			// 是否不改变面向移动
	bool moveVertically_;		// true则可以飞起来移动否则贴地
	PyObject* pyuserarg_;
	float distance_;
	KBEShared_ptr<Controller> pController_;
	int layer_;
	bool isDestroyed_;
};
 
}
#endif // KBE_MOVETOPOINTHANDLER_H

