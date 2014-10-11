/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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

#include "cellapp.hpp"
#include "entity.hpp"
#include "moveto_point_handler.hpp"	
#include "move_controller.hpp"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
MoveToPointHandler::MoveToPointHandler(Controller* pController, int layer, const Position3D& destPos, 
											 float velocity, float range, bool faceMovement, 
											bool moveVertically, PyObject* userarg):
destPos_(destPos),
velocity_(velocity),
faceMovement_(faceMovement),
moveVertically_(moveVertically),
pyuserarg_(userarg),
range_(range),
pController_(pController),
layer_(layer)
{
	static_cast<MoveController*>(pController)->pMoveToPointHandler(this);
	Cellapp::getSingleton().addUpdatable(this);
}

//-------------------------------------------------------------------------------------
MoveToPointHandler::MoveToPointHandler():
destPos_(),
velocity_(0.f),
faceMovement_(false),
moveVertically_(false),
pyuserarg_(NULL),
range_(0.f),
pController_(NULL),
layer_(0)
{
	Cellapp::getSingleton().addUpdatable(this);
}

//-------------------------------------------------------------------------------------
MoveToPointHandler::~MoveToPointHandler()
{
	if(pyuserarg_ != NULL)
	{
		Py_DECREF(pyuserarg_);
	}

	// DEBUG_MSG(fmt::format("MoveToPointHandler::~MoveToPointHandler(): {:p}\n"), (void*)this));
}

//-------------------------------------------------------------------------------------
void MoveToPointHandler::addToStream(KBEngine::MemoryStream& s)
{
	uint8 utype = type();

	s << utype << destPos_.x << destPos_.y << destPos_.z << velocity_ << faceMovement_ << moveVertically_ <<
		range_ << layer_;

	s.appendBlob(script::Pickler::pickle(pyuserarg_));
}

//-------------------------------------------------------------------------------------
void MoveToPointHandler::createFromStream(KBEngine::MemoryStream& s)
{
	s >> /*utype <<*/ destPos_.x >> destPos_.y >> destPos_.z >> velocity_ >> faceMovement_ >> moveVertically_ >>
		range_ >> layer_;

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
		pController_->destroy();
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool MoveToPointHandler::update()
{
	if(pController_ == NULL)
	{
		delete this;
		return false;
	}
	
	Entity* pEntity = pController_->pEntity();
	const Position3D& dstPos = destPos();
	Position3D currpos = pEntity->position();
	Position3D currpos_backup = currpos;
	Direction3D direction = pEntity->direction();

	Vector3 movement = dstPos - currpos;
	if (!moveVertically_) movement.y = 0.f;
	
	bool ret = true;

	if(KBEVec3Length(&movement) < velocity_ + range_)
	{
		float y = currpos.y;
		currpos = dstPos;

		if(range_ > 0.0f)
		{
			// ��λ������
			KBEVec3Normalize(&movement, &movement); 
			movement *= range_;
			currpos -= movement;
		}

		if (!moveVertically_)
			currpos.y = y;

		ret = false;
	}
	else
	{
		// ��λ������
		KBEVec3Normalize(&movement, &movement); 

		// �ƶ�λ��
		movement *= velocity_;
		currpos += movement;
	}
	
	// �Ƿ���Ҫ�ı�����
	if (faceMovement_ && (movement.x != 0.f || movement.z != 0.f))
		direction.yaw(movement.yaw());
	
	// ����entity����λ�ú�����
	if(pController_)
		pEntity->setPositionAndDirection(currpos, direction);

	// ��navigate������ȷ�����ڵ�����
	if(pController_)
		pEntity->isOnGround(isOnGround());

	// ֪ͨ�ű�
	if(pController_)
		pEntity->onMove(pController_->id(), layer_, currpos_backup, pyuserarg_);

	// ����ﵽĿ�ĵ��򷵻�true
	if(!ret)
	{
		return !requestMoveOver(currpos_backup);
	}

	return true;
}

//-------------------------------------------------------------------------------------
}

