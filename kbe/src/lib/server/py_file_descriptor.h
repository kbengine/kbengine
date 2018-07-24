// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_PY_FILE_DESCRIPTOR_H
#define KBE_PY_FILE_DESCRIPTOR_H

#include "common/common.h"
#include "pyscript/scriptobject.h"
#include "common/smartpointer.h"

namespace KBEngine{
typedef SmartPointer<PyObject> PyObjectPtr;

class PyFileDescriptor : public Network::InputNotificationHandler, public Network::OutputNotificationHandler
{
public:
	PyFileDescriptor(int fd, PyObject* pyCallback, bool write);
	virtual ~PyFileDescriptor();
	
	/** 
		脚本请求(注册/注销)文件描述符(读和写)
	*/
	static PyObject* __py_registerReadFileDescriptor(PyObject* self, PyObject* args);
	static PyObject* __py_registerWriteFileDescriptor(PyObject* self, PyObject* args);
	static PyObject* __py_deregisterReadFileDescriptor(PyObject* self, PyObject* args);
	static PyObject* __py_deregisterWriteFileDescriptor(PyObject* self, PyObject* args);
protected:

	virtual int handleInputNotification( int fd );
	virtual int handleOutputNotification( int fd );

	void callback();

	int fd_;
	PyObjectPtr pyCallback_;

	bool write_;
};

}

#endif // KBE_PY_FILE_DESCRIPTOR_H
