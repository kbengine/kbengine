// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPT_COPY_H
#define KBE_SCRIPT_COPY_H

#include "common/common.h"
#include "scriptobject.h"

namespace KBEngine{ namespace script{

class Copy
{						
public:	
	/** ���� copy.copy */
	static PyObject* copy(PyObject* pyobj);
	static PyObject* deepcopy(PyObject* pyobj);

	/** ��ʼ��copy */
	static bool initialize(void);
	static void finalise(void);

private:
	static PyObject* copyMethod_;
	static PyObject* deepcopyMethod_;
	static bool	isInit;										// �Ƿ��Ѿ�����ʼ��
} ;

}
}
#endif // KBE_SCRIPT_COPY_H
