// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CONTROLLER_H
#define KBE_CONTROLLER_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"


namespace KBEngine{

class Entity;
class Controllers;
class MemoryStream;

/*
	控制器， 管理trap、Vision等。
*/
class Controller
{
public:
	enum ControllerType
	{
		CONTROLLER_TYPE_NORMAL = 0,			// 常规类型
		CONTROLLER_TYPE_PROXIMITY = 1,		// 范围触发器类型
		CONTROLLER_TYPE_MOVE = 2,			// 移动控制器类型
		CONTROLLER_TYPE_ROTATE = 3,			// 旋转控制器类型
	};

	Controller(Controller::ControllerType type, Entity* pEntity, int32 userarg, uint32 id = 0);
	Controller(Entity* pEntity);
	virtual ~Controller();
	
	uint32 id() { return id_; }
	void id(uint32 v) { id_ = v; }
	
	int32 userarg() const { return userarg_; }
	
	Entity* pEntity() const { return pEntity_; }
	
	void pControllers(Controllers* v) { pControllers_ = v; }

	virtual void destroy();

	Controller::ControllerType type() { return type_; }
	void type(Controller::ControllerType t) { type_ = t; }

	virtual void addToStream(KBEngine::MemoryStream& s);
	virtual void createFromStream(KBEngine::MemoryStream& s);

protected:
	uint32 id_;
	Entity* pEntity_;
	
	int32 userarg_;
	
	Controllers* pControllers_;

	ControllerType type_;

};

}
#endif // KBE_CONTROLLER_H
