// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_PY_URL_H
#define KBE_PY_URL_H

#include "common/common.h"
#include "scriptobject.h"
#include "pyobject_pointer.h"
#include "network/http_utility.h"

namespace KBEngine { namespace script{

class Script;

class PyUrl
{						
public:	
	/** 
		初始化
	*/
	static bool initialize(Script* pScript);
	static void finalise(void);
	
	static PyObject* __py_urlopen(PyObject* self, PyObject* args);

	static void onHttpCallback(bool success, const Network::Http::Request& pRequest, const std::string& data);

private:
	static bool	isInit; // 是否已经被初始化
	static std::map<PyObject*, PyObjectPtr> pyCallbacks;

};

}
}

#endif // KBE_PY_URL_H
