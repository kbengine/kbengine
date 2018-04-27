// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CLIENT_MOVETOPOINTHANDLER_H
#define KBE_CLIENT_MOVETOPOINTHANDLER_H

#include "pyscript/scriptobject.h"	
#include "math/math.h"
#include "script_callbacks.h"

namespace KBEngine{
namespace client
{

class Entity;
class MoveToPointHandler : public ScriptCallbackHandler
{
public:
	enum MoveType
	{
		MOVE_TYPE_POINT = 0,		// 常规类型
		MOVE_TYPE_ENTITY = 1,		// 范围触发器类型
		MOVE_TYPE_NAV = 2,			// 移动控制器类型
	};

	MoveToPointHandler(ScriptCallbacks& scriptCallbacks, client::Entity* pEntity, int layer, 
		const Position3D& destPos, float velocity, float distance, bool faceMovement, 
		bool moveVertically, PyObject* userarg);

	MoveToPointHandler();
	virtual ~MoveToPointHandler();
	
	virtual bool update(TimerHandle& handle);

	virtual const Position3D& destPos(){ return destPos_; }
	virtual bool requestMoveOver(TimerHandle& handle, const Position3D& oldPos);

	virtual bool isOnGround(){ return false; }

	virtual MoveType type() const{ return MOVE_TYPE_POINT; }

protected:
	virtual void handleTimeout( TimerHandle handle, void * pUser );
	virtual void onRelease( TimerHandle handle, void * /*pUser*/ );

protected:
	Position3D destPos_;
	float velocity_;			// 速度
	bool faceMovement_;			// 是否不改变面向移动
	bool moveVertically_;		// true则可以飞起来移动否则贴地
	PyObject* pyuserarg_;
	float distance_;
	int layer_;
	client::Entity* pEntity_;
};
 
}
}
#endif // KBE_CLIENT_MOVETOPOINTHANDLER_H

