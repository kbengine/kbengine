/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#ifndef __SCRIPT_MATH_H__
#define __SCRIPT_MATH_H__
#include "cstdkbe/cstdkbe.hpp"
#include "scriptobject.hpp"
#include "vector2.hpp"
#include "vector3.hpp"	
#include "vector4.hpp"

namespace KBEngine{ namespace script{ namespace math {
	
/** 安装数学模块 */
bool installModule(const char* moduleName);
bool uninstallModule();

}
}
}
#endif