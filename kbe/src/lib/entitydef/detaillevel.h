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


#ifndef KBE_DETAILLEVEL_H
#define KBE_DETAILLEVEL_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "pyscript/scriptobject.h"	

namespace KBEngine{

/** entity 详情级别类型定义 
	默认有3个级别分别为:
	 近， 中， 远
*/
struct DetailLevel
{
	struct Level
	{
		Level():radius(FLT_MAX), hyst(1.0f){};
		float radius;
		float hyst;

		bool inLevel(float dist)
		{
			if(radius >= dist)
				return true;

			return false;
		}
	};
	
	DetailLevel()
	{
	}

	~DetailLevel()
	{
	}

	Level level[3]; // 近， 中， 远
};

}


#endif // KBE_DETAILLEVEL_H

