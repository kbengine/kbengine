// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "math.h"
namespace KBEngine{ namespace script{ namespace math {

//-------------------------------------------------------------------------------------
bool installModule(const char* moduleName)
{
	// ��ʼ��һ����ѧ��ص�ģ��
	PyObject *mathModule = PyImport_AddModule(moduleName);
	PyObject* pyDoc = PyUnicode_FromString("This module is created by KBEngine!");
	PyObject_SetAttrString(mathModule, "__doc__", pyDoc);
	Py_DECREF(pyDoc);

	// ��ʼ��ScriptVector2
	script::ScriptVector2::installScript(mathModule, "Vector2");
	// ��ʼ��ScriptVector3
	script::ScriptVector3::installScript(mathModule, "Vector3");
	// ��ʼ��ScriptVector4
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
