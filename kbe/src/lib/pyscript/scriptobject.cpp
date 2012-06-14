#include "scriptobject.hpp"
namespace KBEngine{ namespace script{

SCRIPT_METHOD_DECLARE_BEGIN(ScriptObject)
SCRIPT_METHOD_DECLARE_END()


SCRIPT_MEMBER_DECLARE_BEGIN(ScriptObject)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ScriptObject)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ScriptObject, 0, 0, 0, 0, 0)									

//-------------------------------------------------------------------------------------
ScriptObject::ScriptObject(PyTypeObject* pyType, bool isInitialised)
{
	if (PyType_Ready(pyType) < 0)
	{
		ERROR_MSG("ScriptObject: Type %s is not ready\n", pyType->tp_name);
	}

	if (!isInitialised)
	{
		PyObject_INIT(static_cast<PyObject*>(this), pyType);
	}
}

//-------------------------------------------------------------------------------------
ScriptObject::~ScriptObject()
{
	assert(this->ob_refcnt == 0);
}

PyObject* ScriptObject::tp_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	return type->tp_alloc(type, 0); 
};

//-------------------------------------------------------------------------------------
PyObject* ScriptObject::onScriptGetAttribute(PyObject* attr)
{
	return PyObject_GenericGetAttr(this, attr);
}

//-------------------------------------------------------------------------------------
int ScriptObject::onScriptSetAttribute(PyObject* attr, PyObject* value)
{
	return PyObject_GenericSetAttr(static_cast<PyObject*>(this), attr, value);
}

//-------------------------------------------------------------------------------------
int ScriptObject::onScriptDelAttribute(PyObject* attr)
{
	return this->onScriptSetAttribute(attr, NULL);
}

//-------------------------------------------------------------------------------------
int ScriptObject::onScriptInit(PyObject* self, PyObject* args, PyObject* kwds)
{
	return 0;
}

//-------------------------------------------------------------------------------------
PyObject* ScriptObject::tp_repr()
{
	char s[512];
	sprintf(s, "%s object at 0x%08X", this->getObjTypeName(), (unsigned int)(void*)this);
	return PyUnicode_FromString(s);
}

//-------------------------------------------------------------------------------------
PyObject* ScriptObject::tp_str()
{
	char s[512];
	sprintf(s, "%s object at 0x%08X", this->getObjTypeName(), (unsigned int)(void*)this);
	return PyUnicode_FromString(s);
}

//-------------------------------------------------------------------------------------

}
}