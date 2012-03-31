#include <Python.h>
#include <kbemain.hpp>
#include <smartpointer.hpp>
#include <pyobject_pointer.hpp>

int KBENGINE_MAIN(int argc, char* argv[])
{
	Py_Initialize();
	PyRun_SimpleString("print 'i am a static python exe!!!'");

	KBEngine::SmartPointer<PyObject> aaa(::PyString_FromString("fdsfsa"));
	Py_Finalize();
	getchar();
	return 0; 
}