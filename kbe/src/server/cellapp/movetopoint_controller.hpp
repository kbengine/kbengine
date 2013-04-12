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

#ifndef __KBE_MOVETOPOINTCONTROLLER_HPP__
#define __KBE_MOVETOPOINTCONTROLLER_HPP__

#include "controller.hpp"
#include "updatable.hpp"
#include "pyscript/scriptobject.hpp"	

namespace KBEngine{

class MoveToPointController : public Controller, public Updatable
{
public:
	MoveToPointController(Entity* pEntity, const Position3D& destPos, float velocity, float range, bool faceMovement, 
		bool moveVertically, PyObject* userarg, uint32 id = 0);
	virtual ~MoveToPointController();
	
	virtual bool update();

	virtual const Position3D& destPos(){ return destPos_; }
protected:
	Position3D destPos_;
	float velocity_;			// 速度
	bool faceMovement_;			// 是否不改变面向移动
	bool moveVertically_;		// true则可以飞起来移动否则贴地
	PyObject* pyuserarg_;
	float range_;
};
 
}
#endif // __KBE_MOVETOPOINTCONTROLLER_HPP__

