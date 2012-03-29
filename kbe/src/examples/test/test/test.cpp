// ppp.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <Python.h>
#include <server/kbemain.hpp>
#include <cstdkbe/smartpointer.hpp>
#include <cstdkbe/debug.hpp>
#include <pyscript/pyobject_pointer.hpp>

int KBENGINE_MAIN(int argc, char* argv[])
{
	Py_SetPythonHome(L"../../res/script/common");
	Py_Initialize();
	PyRun_SimpleString("print (\'kbe:python is init successfully!!!\')");
	KBEngine::DEBUG_MSG("kbe:python is init successfully!!! %d\n", 88);
	KBEngine::SmartPointer<PyObject> aaa(::PyBytes_FromString("fdsfsa"));
	Py_Finalize();
	getchar();
	return 0; 
}