/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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
		MOVE_TYPE_POINT = 0,		// ��������
		MOVE_TYPE_ENTITY = 1,		// ��Χ����������
		MOVE_TYPE_NAV = 2,			// �ƶ�����������
	};

	virtual std::string c_str(){ return "Move_Handler"; }

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

	MoveToPointHandler(KBEShared_ptr<Controller> pController, int layer, const Position3D& destPos, float velocity, float distance, bool faceMovement, 
		bool moveVertically, PyObject* userarg);

	MoveToPointHandler();
	virtual ~MoveToPointHandler();
	
	virtual bool update();

	virtual const Position3D& destPos(){ return destPos_; }
	virtual bool requestMoveOver(const Position3D& oldPos);

	virtual bool isOnGround(){ return false; }
		
	void pController(KBEShared_ptr<Controller> pController){ pController_ = pController; }

	virtual MoveType type() const{ return MOVE_TYPE_POINT; }

protected:
	Position3D destPos_;
	float velocity_;			// �ٶ�
	bool faceMovement_;			// �Ƿ񲻸ı������ƶ�
	bool moveVertically_;		// true����Է������ƶ���������
	PyObject* pyuserarg_;
	float distance_;
	KBEShared_ptr<Controller> pController_;
	int layer_;
};
 
}
#endif // KBE_MOVETOPOINTHANDLER_H

