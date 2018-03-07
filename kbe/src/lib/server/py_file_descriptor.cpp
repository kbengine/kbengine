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

	if(PyArg_ParseTuple(args, "i|O", &fd, &pycallback) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerReadFileDescriptor: args is error!");
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

	if(PyArg_ParseTuple(args, "i", &fd) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deregisterReadFileDescriptor: args is error!");
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

	if(PyArg_ParseTuple(args, "i|O", &fd, &pycallback) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerWriteFileDescriptor: args is error!");
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

	if(PyArg_ParseTuple(args, "i", &fd) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deregisterWriteFileDescriptor: args is error!");
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
		ERROR_MSG(fmt::format("PyFileDescriptor::callback: can't found callback:{}.\n", fd_));
	}
}

//-------------------------------------------------------------------------------------
}
