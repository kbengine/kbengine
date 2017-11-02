/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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


#include "math.h"
namespace KBEngine{ namespace script{ namespace math {

//-------------------------------------------------------------------------------------
bool installModule(const char* moduleName)
{
	// 初始化一个数学相关的模块
	PyObject *mathModule = PyImport_AddModule(moduleName);
	PyObject_SetAttrString(mathModule, "__doc__", PyUnicode_FromString("This module is created by KBEngine!"));

	// 初始化ScriptVector2
	script::ScriptVector2::installScript(mathModule, "Vector2");
	// 初始化ScriptVector3
	script::ScriptVector3::installScript(mathModule, "Vector3");
	// 初始化ScriptVector4
	script::ScriptVector4::installScript(mathModule, "Vector4");
	return true;
}

//-------------------------------------------------------------------------------------
bool uninstallModule()
{
	script::ScriptVector2::uninstallScript();
	script::ScriptVector3::uninstallScript();
	script::ScriptVector4::uninstallScript();
	return true;
}

}
}
}
