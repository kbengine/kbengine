/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#ifndef KBE_MEMORY_HELPER_H
#define KBE_MEMORY_HELPER_H

#include "common/common.h"
#include "common/kbemalloc.h"
#include "helper/debug_helper.h"

#if KBE_PLATFORM == PLATFORM_WIN32
#ifdef _DEBUG
//#include "vld/vld.h"

namespace KBEngine{
	inline void startLeakDetection(COMPONENT_TYPE type, COMPONENT_ID id)
	{
		//std::wstring leak_filename = fmt::format(L".\\{}_{}.leaks", COMPONENT_NAME_EX(type), 
		//	id);

		//VLDSetReportOptions (VLD_OPT_REPORT_TO_DEBUGGER | VLD_OPT_REPORT_TO_FILE, leak_filename.c_str());
	}
}
#else
namespace KBEngine{
	inline void startLeakDetection(COMPONENT_TYPE type, COMPONENT_ID id){}
}
#endif
#else
namespace KBEngine{
	inline void startLeakDetection(COMPONENT_TYPE type, COMPONENT_ID id){}
}
#endif

namespace KBEngine{



}

#endif // KBE_MEMORY_HELPER_H
