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

#ifndef __KBE_MOVETOPOINTHANDLER_HPP__
#define __KBE_MOVETOPOINTHANDLER_HPP__

#include "controller.hpp"
#include "updatable.hpp"
#include "pyscript/scriptobject.hpp"	

namespace KBEngine{

class MoveToPointHandler : public Updatable
{
public:
	virtual std::string c_str(){ return "MoveToPointHandler"; }

	MoveToPointHandler(Controller* pController, const Position3D& destPos, float velocity, float range, bool faceMovement, 
		bool moveVertically, PyObject* userarg);
	virtual ~MoveToPointHandler();
	
	virtual bool update();

	virtual const Position3D& destPos(){ return destPos_; }
	virtual bool requestMoveOver();

	virtual bool isOnGround(){ return false; }
		
	void pController(Controller* pController){ pController_ = pController; }
protected:
	Position3D destPos_;
	float velocity_;			// 速度
	bool faceMovement_;			// 是否不改变面向移动
	bool moveVertically_;		// true则可以飞起来移动否则贴地
	PyObject* pyuserarg_;
	float range_;
	Controller* pController_;
};
 
}
#endif // __KBE_MOVETOPOINTHANDLER_HPP__

