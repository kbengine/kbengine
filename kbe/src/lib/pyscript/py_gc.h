// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
