/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_CLIENT_ENTITY_ASPECT_H
#define KBE_CLIENT_ENTITY_ASPECT_H

#include "common/common.h"


namespace KBEngine{


class EntityAspect
{
public:
	EntityAspect(const EntityAspect& entityAspect);
	EntityAspect(ENTITY_ID aspectID);
	EntityAspect();
	virtual ~EntityAspect();
	
	void modelres(const std::string& modelres){ modelres_ = modelres; }
	const std::string& modelres(){ return modelres_; }

	ENTITY_ID aspectID() const{ return aspectID_; }

	float modelScale() const{ return modelScale_; }
protected:
	ENTITY_ID aspectID_;
	std::string modelres_;
	float modelScale_;	
};

}
#endif
