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

#ifndef __KBE_AOI_TRIGGER_HPP__
#define __KBE_AOI_TRIGGER_HPP__

#include "range_trigger.hpp"

namespace KBEngine{

class AOITrigger : public RangeTrigger
{
public:
	AOITrigger(RangeNode* origin, float xz = 0.0f, float y = 0.0f);
	virtual ~AOITrigger();


	
	/**
		某个节点进入或者离开了rangeTrigger
	*/
	virtual void onEnter(RangeNode * pNode);
	virtual void onLeave(RangeNode * pNode);
protected:
};

}

#ifdef CODE_INLINE
#include "aoi_trigger.ipp"
#endif
#endif
