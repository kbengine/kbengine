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


#ifndef KBE_PYBOTS_HPP
#define KBE_PYBOTS_HPP
	
// common include	
#include "profile.hpp"
#include "create_and_login_handler.hpp"
#include "cstdkbe/timer.hpp"
#include "pyscript/script.hpp"
#include "network/endpoint.hpp"
#include "helper/debug_helper.hpp"
#include "xmlplus/xmlplus.hpp"	
#include "cstdkbe/singleton.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/timer.hpp"
#include "network/interfaces.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/event_poller.hpp"
#include "client_lib/clientapp.hpp"
#include "pyscript/pyobject_pointer.hpp"
#include "entitydef/entitydef.hpp"

namespace KBEngine{

class PyBots  : public script::ScriptObject
{
	/** 
		子类化 将一些py操作填充进派生类 
	*/
	INSTANCE_SCRIPT_HREADER(PyBots, ScriptObject)	
public:
	PyBots();
	~PyBots();

	/** 
		暴露一些字典方法给python 
	*/
	DECLARE_PY_MOTHOD_ARG1(pyHas_key, ENTITY_ID);
	DECLARE_PY_MOTHOD_ARG0(pyKeys);
	DECLARE_PY_MOTHOD_ARG0(pyValues);
	DECLARE_PY_MOTHOD_ARG0(pyItems);
	
	static PyObject* __py_pyGet(PyObject * self, 
		PyObject * args, PyObject* kwds);

	/** 
		map操作函数相关 
	*/
	static PyObject* mp_subscript(PyObject * self, PyObject * key);

	static int mp_length(PyObject * self);

	static PyMappingMethods mappingMethods;
protected:
};

}

#endif // KBE_PYBOTS_HPP
