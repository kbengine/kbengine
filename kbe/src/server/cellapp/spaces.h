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

#ifndef KBE_SPACEMANAGER_H
#define KBE_SPACEMANAGER_H

#include "helper/debug_helper.h"
#include "common/common.h"
#include "common/singleton.h"
#include "updatable.h"
#include "space.h"

namespace KBEngine{

class Spaces
{
public:
	Spaces();
	~Spaces();
	
	static void finalise();

	typedef std::map<SPACE_ID, KBEShared_ptr<Space> > SPACES;

	/** 
		创建一个新的space 
	*/
	static Space* createNewSpace(SPACE_ID spaceID, const std::string& scriptModuleName);
	
	/**
		销毁一个space
	*/
	static bool destroySpace(SPACE_ID spaceID, ENTITY_ID entityID);

	/** 
		寻找一个指定space 
	*/
	static Space* findSpace(SPACE_ID spaceID);
	
	/** 
		更新所有的space 
	*/
	static void update();

	static size_t size(){ return spaces_.size(); }

protected:
	static SPACES spaces_;
};

}
#endif
