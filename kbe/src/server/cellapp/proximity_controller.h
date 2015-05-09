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

#ifndef KBE_PROXIMITYCONTROLLER_H
#define KBE_PROXIMITYCONTROLLER_H

#include "controller.h"	


namespace KBEngine{

class TrapTrigger;
class CoordinateNode;

/*
	π‹¿Ìtrap°£
*/
class ProximityController : public Controller
{
public:
	ProximityController(Entity* pEntity, float xz, float y, int32 userarg, uint32 id = 0);
	ProximityController(Entity* pEntity);
	~ProximityController();
	
	bool reinstall(CoordinateNode* pCoordinateNode);

	void onEnter(Entity* pEntity, float xz, float y);
	void onLeave(Entity* pEntity, float xz, float y);

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

protected:
	TrapTrigger* pTrapTrigger_;
	float xz_; 
	float y_;
};

}
#endif // KBE_PROXIMITYCONTROLLER_H
