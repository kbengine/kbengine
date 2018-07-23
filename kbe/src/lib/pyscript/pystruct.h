// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPT_STRUCT_H
#define KBE_SCRIPT_STRUCT_H

#include "common/common.h"
#include "scriptobject.h"

namespace KBEngine{ namespace script{

class PyStruct
{						
public:	
	/** 
		代理 struct.pack
	*/
	static std::string pack(PyObject* fmt, PyObject* args);

	/** 
		代理 struct.unpack
	*/
	static PyObject* unpack(PyObject* fmt, PyObject* args);

	/** 
		初始化pickler 
	*/
	static bool initialize(void);
	static void finalise(void);
	
private:
	static PyObject* pack_;									// struct.pack方法指针
	static PyObject* unpack_;								// struct.unpack方法指针

	static bool	isInit;										// 是否已经被初始化
} ;

}
}
#endif // KBE_SCRIPT_STRUCT_H
