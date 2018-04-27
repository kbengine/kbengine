// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
