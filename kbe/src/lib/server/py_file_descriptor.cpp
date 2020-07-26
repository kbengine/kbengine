// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "network/event_dispatcher.h"
#include "network/event_poller.h"
#include "network/network_interface.h"
#include "py_file_descriptor.h"
#include "server/components.h"
#include "helper/debug_helper.h"
#include "pyscript/pyobject_pointer.h"

namespace KBEngine{

//-------------------------------------------------------------------------------------
PyFileDescriptor::PyFileDescriptor(int fd, PyObject* pyCallback, bool write) : 
	fd_(fd),
	pyCallback_(pyCallback),
	write_(write)
{
	if(write)
		Components::getSingleton().pNetworkInterface()->dispatcher().registerWriteFileDescriptor(fd_, this);
	else
		Components::getSingleton().pNetworkInterface()->dispatcher().registerReadFileDescriptor(fd_, this);
}

//-------------------------------------------------------------------------------------
PyFileDescriptor::~PyFileDescriptor()
{
	if(write_)
		Components::getSingleton().pNetworkInterface()->dispatcher().deregisterWriteFileDescriptor(fd_);
	else
		Components::getSingleton().pNetworkInterface()->dispatcher().deregisterReadFileDescriptor(fd_);
}

//-------------------------------------------------------------------------------------
PyObject* PyFileDescriptor::__py_registerReadFileDescriptor(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 2)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerReadFileDescriptor: args != (fileDescriptor, callback)!");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pycallback = NULL;
	int fd = 0;

	if(!PyArg_ParseTuple(args, "i|O", &fd, &pycallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerReadFileDescriptor: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(fd <= 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerReadFileDescriptor: fd <= 0!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(!PyCallable_Check(pycallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerReadFileDescriptor: invalid pycallback!");
		PyErr_PrintEx(0);
		return NULL;
	}

	new PyFileDescriptor(fd, pycallback, false);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* PyFileDescriptor::__py_deregisterReadFileDescriptor(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deregisterReadFileDescriptor: args != (fileDescriptor)!");
		PyErr_PrintEx(0);
		return NULL;
	}

	int fd = 0;

	if(!PyArg_ParseTuple(args, "i", &fd))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deregisterReadFileDescriptor: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(fd <= 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deregisterReadFileDescriptor: fd <= 0!");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyFileDescriptor* pPyFileDescriptor = 
		static_cast<PyFileDescriptor*>(Components::getSingleton().pNetworkInterface()->dispatcher().pPoller()->findForRead(fd));

	if(pPyFileDescriptor)
		delete pPyFileDescriptor;

	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* PyFileDescriptor::__py_registerWriteFileDescriptor(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 2)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerWriteFileDescriptor: args != (fileDescriptor, callback)!");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pycallback = NULL;
	int fd = 0;

	if(!PyArg_ParseTuple(args, "i|O", &fd, &pycallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerWriteFileDescriptor: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(fd <= 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerWriteFileDescriptor: fd <= 0!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(!PyCallable_Check(pycallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerWriteFileDescriptor: invalid pycallback!");
		PyErr_PrintEx(0);
		return NULL;
	}

	new PyFileDescriptor(fd, pycallback, true);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* PyFileDescriptor::__py_deregisterWriteFileDescriptor(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deregisterWriteFileDescriptor: args != (fileDescriptor)!");
		PyErr_PrintEx(0);
		return NULL;
	}

	int fd = 0;

	if(!PyArg_ParseTuple(args, "i", &fd))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deregisterWriteFileDescriptor: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(fd <= 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deregisterWriteFileDescriptor: fd <= 0!");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyFileDescriptor* pPyFileDescriptor = 
		static_cast<PyFileDescriptor*>(Components::getSingleton().pNetworkInterface()->dispatcher().pPoller()->findForWrite(fd));

	if(pPyFileDescriptor)
		delete pPyFileDescriptor;

	S_Return;
}

//-------------------------------------------------------------------------------------
int PyFileDescriptor::handleInputNotification(int fd)
{
	//INFO_MSG(fmt::format("PyFileDescriptor:handleInputNotification: fd = {}\n",
	//			fd));

	callback();
	return 0;
}

//-------------------------------------------------------------------------------------
int PyFileDescriptor::handleOutputNotification( int fd )
{
	//INFO_MSG(fmt::format("PyFileDescriptor:handleOutputNotification: fd = {}\n",
	//			fd));

	callback();
	return 0;
}

//-------------------------------------------------------------------------------------
void PyFileDescriptor::callback()
{
	if(pyCallback_ != NULL)
	{
		PyObject* pyResult = PyObject_CallFunction(pyCallback_.get(), 
											const_cast<char*>("i"), 
											fd_);

		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			SCRIPT_ERROR_CHECK();
	}
	else
	{
		ERROR_MSG(fmt::format("PyFileDescriptor::callback: not found callback:{}.\n", fd_));
	}
}

//-------------------------------------------------------------------------------------
}
