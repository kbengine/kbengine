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

#include "network/event_dispatcher.hpp"
#include "network/event_poller.hpp"
#include "py_file_descriptor.hpp"
#include "baseapp.hpp"
#include "helper/debug_helper.hpp"
#include "pyscript/pyobject_pointer.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
PyFileDescriptor::PyFileDescriptor(int fd, PyObject* pyCallback, bool write) : 
	fd_(fd),
	pyCallback_(pyCallback),
	write_(write)
{
	if(write)
		Baseapp::getSingleton().networkInterface().dispatcher().registerWriteFileDescriptor(fd_, this);
	else
		Baseapp::getSingleton().networkInterface().dispatcher().registerFileDescriptor(fd_, this);
}

//-------------------------------------------------------------------------------------
PyFileDescriptor::~PyFileDescriptor()
{
	if(write_)
		Baseapp::getSingleton().networkInterface().dispatcher().deregisterWriteFileDescriptor(fd_);
	else
		Baseapp::getSingleton().networkInterface().dispatcher().deregisterFileDescriptor(fd_);
}

//-------------------------------------------------------------------------------------
PyObject* PyFileDescriptor::__py_registerFileDescriptor(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 2)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerFileDescriptor: args != (fileDescriptor, callback)!");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pycallback = NULL;
	int fd = 0;

	if(PyArg_ParseTuple(args, "i|O", &fd, &pycallback) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerFileDescriptor: args is error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(fd <= 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerFileDescriptor: fd <= 0!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(!PyCallable_Check(pycallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerFileDescriptor: invalid pycallback!");
		PyErr_PrintEx(0);
		return NULL;
	}

	new PyFileDescriptor(fd, pycallback, false);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* PyFileDescriptor::__py_deregisterFileDescriptor(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerFileDescriptor: args != (fileDescriptor)!");
		PyErr_PrintEx(0);
		return NULL;
	}

	int fd = 0;

	if(PyArg_ParseTuple(args, "i", &fd) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerFileDescriptor: args is error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(fd <= 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::registerFileDescriptor: fd <= 0!");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyFileDescriptor* pPyFileDescriptor = 
		static_cast<PyFileDescriptor*>(Baseapp::getSingleton().networkInterface().dispatcher().pPoller()->find(fd, true));

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
		static_cast<PyFileDescriptor*>(Baseapp::getSingleton().networkInterface().dispatcher().pPoller()->find(fd, false));

	if(pPyFileDescriptor)
		delete pPyFileDescriptor;

	S_Return;
}

//-------------------------------------------------------------------------------------
int PyFileDescriptor::handleInputNotification(int fd)
{
	INFO_MSG(boost::format("PyFileDescriptor:handleInputNotification: fd = %1%\n") %
				fd );

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
		ERROR_MSG(boost::format("PyFileDescriptor::callback: can't found callback:%1%.\n") % fd_);
	}
}

//-------------------------------------------------------------------------------------
}
