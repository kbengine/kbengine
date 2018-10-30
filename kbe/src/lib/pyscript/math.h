// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPT_MATH_H
#define KBE_SCRIPT_MATH_H

#include "common/common.h"
#include "scriptobject.h"
#include "vector2.h"
#include "vector3.h"	
#include "vector4.h"

namespace KBEngine{ namespace script{ namespace math {
	
/** 安装数学模块 */
bool installModule(const char* moduleName);
bool uninstallModule();

}
}
}

#endif // KBE_SCRIPT_MATH_H
