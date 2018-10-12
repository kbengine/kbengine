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

#ifndef KBE_PY_URL_H
#define KBE_PY_URL_H

#include "common/common.h"
#include "scriptobject.h"
#include "pyobject_pointer.h"
#include "network/http_utility.h"

namespace KBEngine{ namespace script{

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
