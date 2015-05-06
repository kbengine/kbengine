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
	};

	Controller(Controller::ControllerType type, Entity* pEntity, int32 userarg, uint32 id = 0);
	Controller(Entity* pEntity);
	virtual ~Controller();
	
	uint32 id(){ return id_; }
	void id(uint32 v){ id_ = v; }
	
	int32 userarg() const{ return userarg_; }
	
	Entity* pEntity() const{ return pEntity_; }
	
	void pControllers(Controllers* v){ pControllers_ = v; }

	virtual void destroy();

	Controller::ControllerType type(){ return type_; }
	void type(Controller::ControllerType t){ type_ = t; }

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
