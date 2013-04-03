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

/*
	控制器， 管理trap、Vision等。
*/
class Controller
{
public:
	Controller(Entity* pEntity, uint32 userarg, uint32 id = 0);
	virtual ~Controller();

	virtual void update() {}
	
	uint32 id(){ return id_; }
	void id(uint32 v){ id_ = v; }
	
	uint32 userarg()const{ return userarg_; }
	
	Entity* pEntity()const{ return pEntity_; }
protected:
	uint32 id_;
	Entity* pEntity_;
	
	uint32 userarg_;

};

}
#endif // __KBE_CONTROLLER_HPP__
