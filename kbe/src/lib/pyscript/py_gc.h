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

#ifndef KBE_PY_GC_H
#define KBE_PY_GC_H

#include "common/common.h"
#include "scriptobject.h"

namespace KBEngine{ namespace script{

class PyGC
{						
public:	
	static uint32 DEBUG_STATS;
	static uint32 DEBUG_COLLECTABLE;
	static uint32 DEBUG_UNCOLLECTABLE;
	static uint32 DEBUG_SAVEALL;
	static uint32 DEBUG_LEAK;
	
	/** 
		初始化pickler 
	*/
	static bool initialize(void);
	static void finalise(void);
	
	/** 
		强制回收垃圾
	*/
	static void collect(int8 generations = -1);

	/** 
		设置调试标志
	*/
	static void set_debug(uint32 flags);
	
	/**
		增加计数
	*/
	static void incTracing(std::string name);

	/**
		减少计数
	*/
	static void decTracing(std::string name);

	/**
		debug追踪kbe封装的py对象计数
	*/
	static void debugTracing(bool shuttingdown = true);

	/**
		脚本调用
	*/
	static PyObject* __py_debugTracing(PyObject* self, PyObject* args);

private:
	static PyObject* collectMethod_;							// cPicket.dumps方法指针
	static PyObject* set_debugMethod_;							// cPicket.loads方法指针

	static bool	isInit;											// 是否已经被初始化

	static KBEUnordered_map<std::string, int> tracingCountMap_;	// 追踪特定的对象计数器
} ;

}
}

#endif // KBE_PY_GC_H
