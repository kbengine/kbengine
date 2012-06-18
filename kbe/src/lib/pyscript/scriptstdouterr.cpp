#include "scriptstdouterr.hpp"
namespace KBEngine{ namespace script{

SCRIPT_METHOD_DECLARE_BEGIN(ScriptStdOutErr)
SCRIPT_METHOD_DECLARE("write",				write,				METH_VARARGS,			0)	
SCRIPT_METHOD_DECLARE("flush",				flush,				METH_VARARGS,			0)	
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(ScriptStdOutErr)
SCRIPT_MEMBER_DECLARE("softspace",			softspace_,		T_CHAR,					0,			0)	
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ScriptStdOutErr)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ScriptStdOutErr, 0, 0, 0, 0, 0)									

//-------------------------------------------------------------------------------------
ScriptStdOutErr::ScriptStdOutErr():
ScriptObject(getScriptType(), false),
softspace_(0),
sysModule_(NULL),
prevStdout_(NULL),
prevStderr_(NULL),
isInstall_(false),
sbuffer_()
{
}

//-------------------------------------------------------------------------------------
ScriptStdOutErr::~ScriptStdOutErr()
{
}

//-------------------------------------------------------------------------------------
PyObject* ScriptStdOutErr::__py_write(PyObject* self, PyObject *args)
{
	char * msg;
	Py_ssize_t msglen;
	if (!PyArg_ParseTuple(args, "s#", &msg, &msglen))
	{
		ERROR_MSG("ScriptStdOutErr::write: Bad args\n");
		return NULL;
	}
		
	static_cast<ScriptStdOutErr*>(self)->onPrint(msg);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptStdOutErr::__py_flush(PyObject* self, PyObject *args)
{
	S_Return;
}

//-------------------------------------------------------------------------------------
void ScriptStdOutErr::onPrint(const char* msg)
{
	sbuffer_ += msg;
	if(msg[0] == '\n')
	{
		PRINT_MSG("%s", sbuffer_.c_str());
		sbuffer_ = "";
	}
}

//-------------------------------------------------------------------------------------
bool ScriptStdOutErr::install(void)
{
	sysModule_ = PyImport_ImportModule("sys");
	if (!sysModule_)
	{
		ERROR_MSG("ScriptStdOut: Failed to import sys module\n");
		return false;
	}

	prevStderr_ = PyObject_GetAttrString(sysModule_, "stderr");
	prevStdout_ = PyObject_GetAttrString(sysModule_, "stdout");

	PyObject_SetAttrString(sysModule_, "stdout", (PyObject *)this);
	PyObject_SetAttrString(sysModule_, "stderr", (PyObject *)this);
	isInstall_ = true;
	return true;	
}

//-------------------------------------------------------------------------------------
bool ScriptStdOutErr::uninstall(void)
{
	if (prevStderr_)
	{
		PyObject_SetAttrString(sysModule_, "stderr", prevStderr_);
		Py_DECREF(prevStderr_);
		prevStderr_ = NULL;
	}

	if (prevStdout_)
	{
		PyObject_SetAttrString(sysModule_, "stdout", prevStdout_);
		Py_DECREF(prevStdout_);
		prevStdout_ = NULL;
	}

	Py_DECREF(sysModule_);
	sysModule_ = NULL;
	isInstall_ = false;
	return true;	
}

//-------------------------------------------------------------------------------------

}
}