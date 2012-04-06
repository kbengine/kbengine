#include "scriptstdouterr.hpp"
namespace KBEngine{ namespace script{

SCRIPT_METHOD_DECLARE_BEGIN(ScriptStdOutErr)
SCRIPT_METHOD_DECLARE("write",				write,				METH_VARARGS,			0)	
SCRIPT_METHOD_DECLARE("flush",				flush,				METH_VARARGS,			0)	
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(ScriptStdOutErr)
SCRIPT_MEMBER_DECLARE("softspace",			m_softspace_,		T_CHAR,					0,			0)	
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ScriptStdOutErr)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ScriptStdOutErr, 0, 0, 0, 0, 0)									

//-------------------------------------------------------------------------------------
ScriptStdOutErr::ScriptStdOutErr():
ScriptObject(getScriptType(), false)
{
	m_softspace_ = 0;
	m_isInstall_ = false;
	m_sysModule_ = m_prevStdout_ = m_prevStderr_ = NULL;
}

//-------------------------------------------------------------------------------------
ScriptStdOutErr::~ScriptStdOutErr()
{
}

//-------------------------------------------------------------------------------------
PyObject* ScriptStdOutErr::write(PyObject* self, PyObject *args)
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
PyObject* ScriptStdOutErr::flush(PyObject* self, PyObject *args)
{
	S_Return;
}

//-------------------------------------------------------------------------------------
void ScriptStdOutErr::onPrint(const char* msg)
{
	PRINT_MSG("%s", msg);
}

//-------------------------------------------------------------------------------------
bool ScriptStdOutErr::install(void)
{
	m_sysModule_ = PyImport_ImportModule("sys");
	if (!m_sysModule_)
	{
		ERROR_MSG("ScriptStdOut: Failed to import sys module\n");
		return false;
	}

	m_prevStderr_ = PyObject_GetAttrString(m_sysModule_, "stderr");
	m_prevStdout_ = PyObject_GetAttrString(m_sysModule_, "stdout");

	PyObject_SetAttrString(m_sysModule_, "stdout", (PyObject *)this);
	PyObject_SetAttrString(m_sysModule_, "stderr", (PyObject *)this);
	m_isInstall_ = true;
	return true;	
}

//-------------------------------------------------------------------------------------
bool ScriptStdOutErr::uninstall(void)
{
	if (m_prevStderr_)
	{
		PyObject_SetAttrString(m_sysModule_, "stderr", m_prevStderr_);
		Py_DECREF(m_prevStderr_);
		m_prevStderr_ = NULL;
	}

	if (m_prevStdout_)
	{
		PyObject_SetAttrString(m_sysModule_, "stdout", m_prevStdout_);
		Py_DECREF(m_prevStdout_);
		m_prevStdout_ = NULL;
	}

	Py_DECREF(m_sysModule_);
	m_sysModule_ = NULL;
	m_isInstall_ = false;
	return true;	
}

//-------------------------------------------------------------------------------------

}
}