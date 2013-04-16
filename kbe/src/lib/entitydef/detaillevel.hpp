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


#ifndef __DETAILLEVEL_H__
#define __DETAILLEVEL_H__
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "pyscript/scriptobject.hpp"	

namespace KBEngine{

/** entity 详情级别类型定义 */
struct DetailLevel
{
	struct Level
	{
		Level():radius(0.0f), hyst(0.0f){};
		float radius;
		float hyst;
	};
	
	DetailLevel()
	{
		level[0] = NULL;
		level[1] = NULL;
		level[2] = NULL;
		level[3] = NULL;
	}

	~DetailLevel()
	{
		SAFE_RELEASE(level[0]);
		SAFE_RELEASE(level[1]);
		SAFE_RELEASE(level[2]);
		SAFE_RELEASE(level[3]);
	}




	Level* level[4];
};

}


#endif
