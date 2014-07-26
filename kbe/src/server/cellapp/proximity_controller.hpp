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

#ifndef __KBE_PROXIMITYCONTROLLER_HPP__
#define __KBE_PROXIMITYCONTROLLER_HPP__

#include "controller.hpp"	


namespace KBEngine{

class TrapTrigger;

/*
	����trap��
*/
class ProximityController : public Controller
{
public:
	ProximityController(Entity* pEntity, float xz, float y, int32 userarg, uint32 id = 0);
	~ProximityController();
	
	bool reinstall(CoordinateNode* pCoordinateNode);

	void onEnter(Entity* pEntity, float xz, float y);
	void onLeave(Entity* pEntity, float xz, float y);
protected:
	TrapTrigger* pTrapTrigger_;
};

}
#endif // __KBE_PROXIMITYCONTROLLER_HPP__
