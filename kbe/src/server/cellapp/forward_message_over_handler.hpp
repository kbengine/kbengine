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

#ifndef __FORWARD_MESSAGE_HANDLER__
#define __FORWARD_MESSAGE_HANDLER__

// common include
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
// #define NDEBUG
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif

namespace KBEngine{

class FMH_Baseapp_onEntityGetCell : public ForwardMessageOverHandler
{
public:
	FMH_Baseapp_onEntityGetCell(Entity* e, SPACE_ID spaceID);

	virtual void process();
private:
	Entity* _e;
	SPACE_ID _spaceID;
};


}
#endif
