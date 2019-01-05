// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPT_COPY_H
#define KBE_SCRIPT_COPY_H

#include "common/common.h"
#include "scriptobject.h"

namespace KBEngine{ namespace script{

class Copy
{						
public:	
	/** 代理 copy.copy */
	static PyObject* copy(PyObject* pyobj);
	static PyObject* deepcopy(PyObject* pyobj);

	/** 初始化copy */
	static bool initialize(void);
	static void finalise(void);

private:
	static PyObject* copyMethod_;
	static PyObject* deepcopyMethod_;
	static bool	isInit;										// 是否已经被初始化
} ;

}
}
#endif // KBE_SCRIPT_COPY_H
