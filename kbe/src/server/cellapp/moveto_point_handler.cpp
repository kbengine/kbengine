// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "cellapp.h"
#include "entity.h"
#include "moveto_point_handler.h"	
#include "move_controller.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
MoveToPointHandler::MoveToPointHandler(KBEShared_ptr<Controller>& pController, int layer, const Position3D& destPos, 
											 float velocity, float distance, bool faceMovement, 
											bool moveVertically, PyObject* userarg):
destPos_(destPos),
velocity_(velocity),
faceMovement_(faceMovement),
moveVertically_(moveVertically),
pyuserarg_(userarg),
distance_(distance),
pController_(pController),
layer_(layer),
isDestroyed_(false)
{
	updatableName = "MoveToPointHandler";

	Py_INCREF(userarg);

	//std::static_pointer_cast<MoveController>(pController)->pMoveToPointHandler(this);
	static_cast<MoveController*>(pController.get())->pMoveToPointHandler(this);
	Cellapp::getSingleton().addUpdatable(this);
}

//-------------------------------------------------------------------------------------
MoveToPointHandler::MoveToPointHandler():
destPos_(),
velocity_(0.f),
faceMovement_(false),
moveVertically_(false),
pyuserarg_(NULL),
distance_(0.f),
layer_(0),
isDestroyed_(false)
{
	updatableName = "MoveToPointHandler";

	Cellapp::getSingleton().addUpdatable(this);
}

//-------------------------------------------------------------------------------------
MoveToPointHandler::~MoveToPointHandler()
{
	if(pyuserarg_ != NULL)
	{
		Py_DECREF(pyuserarg_);
	}

	//DEBUG_MSG(fmt::format("MoveToPointHandler::~MoveToPointHandler(): {}\n", (void*)this));
}

//-------------------------------------------------------------------------------------
void MoveToPointHandler::addToStream(KBEngine::MemoryStream& s)
{
	// uint8 utype = type();

	s << /*utype <<*/ destPos_.x << destPos_.y << destPos_.z << velocity_ << faceMovement_ << moveVertically_ <<
		distance_ << layer_;

	s.appendBlob(script::Pickler::pickle(pyuserarg_));
}

//-------------------------------------------------------------------------------------
void MoveToPointHandler::createFromStream(KBEngine::MemoryStream& s)
{
	s >> /*utype <<*/ destPos_.x >> destPos_.y >> destPos_.z >> velocity_ >> faceMovement_ >> moveVertically_ >>
		distance_ >> layer_;

	std::string val = "";
	s.readBlob(val);

	pyuserarg_ = script::Pickler::unpickle(val);
}

//-------------------------------------------------------------------------------------
bool MoveToPointHandler::requestMoveOver(const Position3D& oldPos)
{
	if(pController_)
	{
		if(pController_->pEntity())
			pController_->pEntity()->onMoveOver(pController_->id(), layer_, oldPos, pyuserarg_);

		// 如果在onMoveOver中调用cancelController（id）会导致MoveController析构导致pController_为NULL
		pController_->destroy();
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool MoveToPointHandler::update()
{
	if (isDestroyed_)
	{
		delete this;
		return false;
	}
	
	Entity* pEntity = pController_->pEntity();
	Py_INCREF(pEntity);

	const Position3D& dstPos = destPos();
	Position3D currpos = pEntity->position();
	Position3D currpos_backup = currpos;
	Direction3D direction = pEntity->direction();

	Vector3 movement = dstPos - currpos;
	if (!moveVertically_) movement.y = 0.f;
	
	bool ret = true;
	float dist_len = KBEVec3Length(&movement);

	if (dist_len < velocity_ + distance_)
	{
		float y = currpos.y;

		if (distance_ > 0.0f)
		{
			// 单位化向量
			KBEVec3Normalize(&movement, &movement); 
				
			if(dist_len > distance_)
			{
				movement *= distance_;
				currpos = dstPos - movement;
			}
		}
		else
		{
			currpos = dstPos;
		}

		if (!moveVertically_)
			currpos.y = y;

		ret = false;
	}
	else
	{
		// 单位化向量
		KBEVec3Normalize(&movement, &movement); 

		// 移动位置
		movement *= velocity_;
		currpos += movement;
	}
	
	// 是否需要改变面向
	if (faceMovement_)
	{
		if (movement.x != 0.f || movement.z != 0.f)
			direction.yaw(movement.yaw());

		if (movement.y != 0.f)
			direction.pitch(movement.pitch());
	}
	
	// 设置entity的新位置和面向
	if(!isDestroyed_)
		pEntity->setPositionAndDirection(currpos, direction);

	// 非navigate都不能确定其在地面上
	if(!isDestroyed_)
		pEntity->isOnGround(isOnGround());

	// 通知脚本
	if(!isDestroyed_)
		pEntity->onMove(pController_->id(), layer_, currpos_backup, pyuserarg_);

	// 如果在onMove过程中被停止，又或者达到目的地了，则直接销毁并返回false
	if (isDestroyed_ || 
		(!ret && requestMoveOver(currpos_backup)))
	{
		Py_DECREF(pEntity);
		delete this;
		return false;
	}

	Py_DECREF(pEntity);
	return true;
}

//-------------------------------------------------------------------------------------
}

