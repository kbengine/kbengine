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

#ifndef __KBE_CONTROLLER_HPP__
#define __KBE_CONTROLLER_HPP__

// common include
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"


namespace KBEngine{

class Entity;
class Controllers;

/*
	控制器， 管理trap、Vision等。
*/
class Controller
{
public:
	Controller(Entity* pEntity, int32 userarg, uint32 id = 0);
	virtual ~Controller();
	
	uint32 id(){ return id_; }
	void id(uint32 v){ id_ = v; }
	
	int32 userarg()const{ return userarg_; }
	
	Entity* pEntity()const{ return pEntity_; }
	
	void pControllers(Controllers* v){ pControllers_ = v; }

	virtual void destroy();
protected:
	uint32 id_;
	Entity* pEntity_;
	
	int32 userarg_;
	
	Controllers* pControllers_;

};

}
#endif // __KBE_CONTROLLER_HPP__
