// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "pyscript/copy.h"
#include "py_entitydef.h"
#include "pyscript/pyobject_pointer.h"
#include <stack>

namespace KBEngine{ namespace script{ namespace entitydef {

struct CallContext
{
	PyObjectPtr pyArgs;
	PyObjectPtr pyKwargs;
	std::string optionName;
};

struct DefContext
{
	DefContext()
	{
		optionName = "";

		pyModuleName = "";
		pyAttrName = "";
		pyMethodArgs = "";
		pyReturnType = "";

		isModuleScope = false;

		exposed = false;
		hasClient = false;
		persistent = false;
		databaseLength = 0;

		propertyFlags = "";
		propertyIndex = "";

		implementedBy = "";
	}

	std::string optionName;

	std::string pyModuleName;
	std::string pyAttrName;
	std::string pyMethodArgs;
	std::string pyReturnType;

	std::vector< std::string > argsvecs;
	std::map< std::string, std::string > annotationsMaps;

	bool isModuleScope;

	bool exposed;
	bool hasClient;
	bool persistent;
	int databaseLength;

	std::string propertyFlags;
	std::string propertyIndex;

	std::string implementedBy;
};

static std::stack<CallContext> g_callContexts;
static std::string pyDefModuleName = "";

//-------------------------------------------------------------------------------------
static bool isRefEntityDefModule(PyObject *pyObj)
{
	PyObject *entitydefModule = PyImport_AddModule(pyDefModuleName.c_str());
	PyObject* pydict = PyObject_GetAttrString(entitydefModule, "__dict__");

	PyObject *key, *value;
	Py_ssize_t pos = 0;

	while (PyDict_Next(pydict, &pos, &key, &value)) {
		if (value == pyObj)
		{
			Py_DECREF(pydict);
			return true;
		}
	}

	Py_DECREF(pydict);
	return false;
}

//-------------------------------------------------------------------------------------
static PyObject* __py_def_parse(PyObject *self, PyObject* args)
{
	CallContext cc = g_callContexts.top();
	g_callContexts.pop();

	DefContext defContext;
	defContext.optionName = cc.optionName;

	if (defContext.optionName == "method")
	{
		static char * keywords[] =
		{
			const_cast<char *> ("exposed"),
			NULL
		};

		PyObject* pyExposed = NULL;

		if (!PyArg_ParseTupleAndKeywords(cc.pyArgs.get(), cc.pyKwargs.get(), "|O",
			keywords, &pyExposed))
		{
			return NULL;
		}

		defContext.exposed = pyExposed == Py_True;
	}
	else if (defContext.optionName == "property")
	{
		static char * keywords[] =
		{
			const_cast<char *> ("flags"),
			const_cast<char *> ("persistent"),
			const_cast<char *> ("index"),
			const_cast<char *> ("databaseLength"),
			NULL
		};

		PyObject* pyFlags = NULL;
		PyObject* pyPersistent = NULL;
		PyObject* pyIndex = NULL;
		PyObject* pyDatabaseLength = NULL;

		if (!PyArg_ParseTupleAndKeywords(cc.pyArgs.get(), cc.pyKwargs.get(), "|OOOO",
			keywords, &pyFlags, &pyPersistent, &pyIndex, &pyDatabaseLength))
		{
			return NULL;
		}

		if (!isRefEntityDefModule(pyFlags))
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'flags\' must be referenced from the [Def.ALL_CLIENTS, Def.*] module!\n", defContext.optionName.c_str());
			return NULL;
		}

		if (!isRefEntityDefModule(pyIndex))
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'index\' must be referenced from the [Def.UNIQUE, Def.INDEX] module!\n", defContext.optionName.c_str());
			return NULL;
		}

		if (pyDatabaseLength && PyLong_Check(pyDatabaseLength))
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'databaseLength\' error! not a number type.\n", defContext.optionName.c_str());
			return NULL;
		}

		if (pyPersistent && PyBool_Check(pyPersistent))
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'persistent\' error! not a bool type.\n", defContext.optionName.c_str());
			return NULL;
		}

		defContext.propertyFlags = PyUnicode_AsUTF8AndSize(pyFlags, NULL);

		if(pyPersistent)
			defContext.persistent = pyPersistent == Py_True;

		if (pyIndex)
			defContext.propertyIndex = PyUnicode_AsUTF8AndSize(pyIndex, NULL);

		if (pyDatabaseLength)
			defContext.databaseLength = (int)PyLong_AsLong(pyDatabaseLength);
	}
	else if (defContext.optionName == "entity")
	{
		defContext.isModuleScope = true;

		static char * keywords[] =
		{
			const_cast<char *> ("hasClient"),
			NULL
		};

		PyObject* pyHasClient = NULL;

		if (!PyArg_ParseTupleAndKeywords(cc.pyArgs.get(), cc.pyKwargs.get(), "|O",
			keywords, &pyHasClient))
		{
			return NULL;
		}

		defContext.hasClient = pyHasClient == Py_True;
	}
	else if (defContext.optionName == "interface")
	{
		defContext.isModuleScope = true;
	}
	else if (defContext.optionName == "component")
	{
		defContext.isModuleScope = true;
	}
	else if (defContext.optionName == "fixed_dict")
	{
		defContext.isModuleScope = true;

		static char * keywords[] =
		{
			const_cast<char *> ("implementedBy"),
			NULL
		};

		PyObject* pImplementedBy = NULL;

		if (!PyArg_ParseTupleAndKeywords(cc.pyArgs.get(), cc.pyKwargs.get(), "|O",
			keywords, &pImplementedBy))
		{
			return NULL;
		}

		if (pImplementedBy)
		{
			PyObject* pyQualname = PyObject_GetAttrString(pImplementedBy, "__qualname__");
			if (!pyQualname)
			{
				return NULL;
			}

			defContext.implementedBy = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
			Py_DECREF(pyQualname);
		}
	}
	else if (defContext.optionName == "fixed_array")
	{
		defContext.isModuleScope = true;
	}
	else if (defContext.optionName == "fixed_item")
	{
	}
	else if (defContext.optionName == "rename")
	{
		defContext.isModuleScope = true;
	}
	else
	{
		PyErr_Format(PyExc_AssertionError, "Def.%s: not support!\n", defContext.optionName.c_str());
		return NULL;
	}

	if (!args || PyTuple_Size(args) < 1)
	{
		PyErr_Format(PyExc_AssertionError, "Def.__py_def_call(Def.%s): error!\n", defContext.optionName.c_str());
		return NULL;
	}

	PyObject* pyFunc = PyTuple_GET_ITEM(args, 0);

	PyObject* pyQualname = PyObject_GetAttrString(pyFunc, "__qualname__");
	if (!pyQualname)
	{
		return NULL;
	}

	const char* qualname = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
	Py_DECREF(pyQualname);

	if (!defContext.isModuleScope)
	{
		std::vector<std::string> outs;

		if (qualname)
			strutil::kbe_splits(qualname, ".", outs);

		if (outs.size() != 2)
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' must be defined in the entity module!\n", defContext.optionName.c_str(), qualname);
			return NULL;
		}

		defContext.pyModuleName = outs[0];
		defContext.pyAttrName = outs[1];

		PyObject* pyInspectModule =
			PyImport_ImportModule(const_cast<char*>("inspect"));

		PyObject* pyGetfullargspec = NULL;
		if (pyInspectModule)
		{
			Py_DECREF(pyInspectModule);

			pyGetfullargspec =
				PyObject_GetAttrString(pyInspectModule, const_cast<char *>("getfullargspec"));
		}
		else
		{
			return NULL;
		}

		if (pyGetfullargspec)
		{
			PyObject* pyGetMethodArgs = PyObject_CallFunction(pyGetfullargspec,
				const_cast<char*>("(O)"), pyFunc);

			if (!pyGetMethodArgs)
			{
				return NULL;
			}
			else
			{
				PyObject* pyGetMethodArgsResult = PyObject_GetAttrString(pyGetMethodArgs, const_cast<char *>("args"));
				PyObject* pyGetMethodAnnotationsResult = PyObject_GetAttrString(pyGetMethodArgs, const_cast<char *>("annotations"));

				Py_DECREF(pyGetMethodArgs);

				if (!pyGetMethodArgsResult || !pyGetMethodAnnotationsResult)
				{
					Py_XDECREF(pyGetMethodArgsResult);
					Py_XDECREF(pyGetMethodAnnotationsResult);
					return NULL;
				}

				PyObjectPtr pyGetMethodArgsResultPtr = pyGetMethodArgsResult;
				PyObjectPtr pyGetMethodAnnotationsResultPtr = pyGetMethodAnnotationsResult;
				Py_DECREF(pyGetMethodArgsResult);
				Py_DECREF(pyGetMethodAnnotationsResult);

				Py_ssize_t argsSize = PyList_Size(pyGetMethodArgsResult);
				if (argsSize == 0)
				{
					PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' did not find \'self\' parameter!\n", defContext.optionName.c_str(), qualname);
					return NULL;
				}

				for (Py_ssize_t i = 1; i < argsSize; ++i)
				{
					PyObject* pyItem = PyList_GetItem(pyGetMethodArgsResult, i);

					const char* ccattr = PyUnicode_AsUTF8AndSize(pyItem, NULL);
					if (!ccattr)
					{
						return NULL;
					}

					defContext.argsvecs.push_back(ccattr);

					PyErr_Format(PyExc_AssertionError, "Def.%s: -------arg---------------- %s!\n", defContext.optionName.c_str(), ccattr);
					PyErr_PrintEx(0);
				}

				PyObject *key, *value;
				Py_ssize_t pos = 0;

				while (PyDict_Next(pyGetMethodAnnotationsResult, &pos, &key, &value)) {
					const char* skey = PyUnicode_AsUTF8AndSize(key, NULL);
					if (!skey)
					{
						return NULL;
					}

					std::string svalue = "";

					if (PyUnicode_Check(value))
					{
						svalue = PyUnicode_AsUTF8AndSize(value, NULL);
					}
					else
					{
						PyObject* pyQualname = PyObject_GetAttrString(value, "__qualname__");
						if (!pyQualname)
							return NULL;

						svalue = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
						Py_DECREF(pyQualname);
					}

					if (svalue.size() == 0)
					{
						return NULL;
					}

					if (std::string(skey) == "return")
						defContext.pyReturnType = svalue;
					else
						defContext.annotationsMaps[skey] = svalue;

					PyErr_Format(PyExc_AssertionError, "Def.%s: -------annotations---------------- %s.%s!\n", defContext.optionName.c_str(), skey, svalue.c_str());
					PyErr_PrintEx(0);
				}
			}
		}
	}
	else
	{
		defContext.pyModuleName = qualname;

		if (defContext.optionName != "rename")
		{
			PyObject* pyBases = PyObject_GetAttrString(pyFunc, "__bases__");
			if (!pyBases)
				return NULL;

			Py_ssize_t basesSize = PyTuple_Size(pyBases);
			if (basesSize == 0)
			{
				PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' does not inherit the KBEngine.Entity class!\n", defContext.optionName.c_str(), qualname);
				Py_XDECREF(pyBases);
				return NULL;
			}

			for (Py_ssize_t i = 0; i < basesSize; ++i)
			{
				PyObject* pyClass = PyTuple_GetItem(pyBases, i);

				PyObject* pyQualname = PyObject_GetAttrString(pyClass, "__qualname__");
				if (!pyQualname)
				{
					Py_XDECREF(pyBases);
					return NULL;
				}

				std::string parentClass = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
				Py_DECREF(pyQualname);


				PyErr_Format(PyExc_AssertionError, "Def.%s: -------parentclass---------------- %s!\n", defContext.optionName.c_str(), parentClass.c_str());
				PyErr_PrintEx(0);
			}

			Py_XDECREF(pyBases);
		}
		PyErr_Format(PyExc_AssertionError, "Def.%s: -------class---------------- %s--%d!\n", defContext.optionName.c_str(), qualname, defContext.hasClient);
		PyErr_PrintEx(0);
	}

	if (defContext.optionName == "method" || defContext.optionName == "clientmethod")
	{
		if (defContext.annotationsMaps.size() != defContext.argsvecs.size())
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' all parameters must have annotations!\n", defContext.optionName.c_str(), qualname);
			return NULL;
		}
	}

	Py_INCREF(pyFunc);
	return pyFunc;
}

//-------------------------------------------------------------------------------------
static PyMethodDef __call_def_parse = { "_PyEntityDefParse", (PyCFunction)&__py_def_parse, METH_VARARGS, 0 };

#define PY_DEF_HOOK(NAME)	\
	static PyObject* __py_def_##NAME(PyObject *self, PyObject* args, PyObject * kwargs)	\
	{	\
		CallContext cc;	\
		cc.pyArgs = PyObjectPtr(Copy::deepcopy(args));	\
		cc.pyKwargs = PyObjectPtr(Copy::deepcopy(kwargs));	\
		cc.optionName = #NAME;	\
		g_callContexts.push(cc);	\
		Py_XDECREF(cc.pyArgs.get());	\
		Py_XDECREF(cc.pyKwargs.get());	\
	\
		return PyCFunction_New(&__call_def_parse, self);	\
	}

#define PY_ADD_METHOD(NAME, DOCS) APPEND_SCRIPT_MODULE_METHOD(entitydefModule, NAME, __py_def_##NAME, METH_VARARGS | METH_KEYWORDS, 0);

PY_DEF_HOOK(rename)
PY_DEF_HOOK(method)
PY_DEF_HOOK(clientmethod)
PY_DEF_HOOK(property)
PY_DEF_HOOK(entity)
PY_DEF_HOOK(interface)
PY_DEF_HOOK(component)
PY_DEF_HOOK(fixed_dict)
PY_DEF_HOOK(fixed_array)
PY_DEF_HOOK(fixed_item)

//-------------------------------------------------------------------------------------
bool installModule(const char* moduleName)
{
	pyDefModuleName = moduleName;

	PyObject *entitydefModule = PyImport_AddModule(pyDefModuleName.c_str());
	PyObject_SetAttrString(entitydefModule, "__doc__", PyUnicode_FromString("This module is created by KBEngine!"));

	PY_ADD_METHOD(rename, "");
	PY_ADD_METHOD(method, "");
	PY_ADD_METHOD(clientmethod, "");
	PY_ADD_METHOD(property, "");
	PY_ADD_METHOD(entity, "");
	PY_ADD_METHOD(interface, "");
	PY_ADD_METHOD(component, "");
	PY_ADD_METHOD(fixed_dict, "");
	PY_ADD_METHOD(fixed_array, "");
	PY_ADD_METHOD(fixed_item, "");

	return true;
}

//-------------------------------------------------------------------------------------
bool uninstallModule()
{
	g_callContexts.empty();
	return true; 
}

//-------------------------------------------------------------------------------------
bool initialize(std::vector<PyTypeObject*>& scriptBaseTypes,
	COMPONENT_TYPE loadComponentType)
{
	PyObject *entitydefModule = PyImport_AddModule(pyDefModuleName.c_str());

	ENTITYFLAGMAP::iterator iter = g_entityFlagMapping.begin();
	for (; iter != g_entityFlagMapping.end(); ++iter)
	{
		if (PyModule_AddStringConstant(entitydefModule, iter->first.c_str(), iter->first.c_str()))
		{
			ERROR_MSG(fmt::format("PyEntityDef::initialize(): Unable to set Def.{} to {}\n",
				iter->first, iter->first));

			return false;
		}
	}

	static const char* UNIQUE = "UNIQUE";
	if (PyModule_AddStringConstant(entitydefModule, UNIQUE, UNIQUE))
	{
		ERROR_MSG(fmt::format("PyEntityDef::initialize(): Unable to set Def.{} to {}\n",
			iter->first, iter->first));

		return false;
	}

	static const char* INDEX = "INDEX";
	if (PyModule_AddStringConstant(entitydefModule, INDEX, INDEX))
	{
		ERROR_MSG(fmt::format("PyEntityDef::initialize(): Unable to set Def.{} to {}\n",
			iter->first, iter->first));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool finalise(bool isReload)
{
	return true;
}

//-------------------------------------------------------------------------------------
void reload(bool fullReload)
{
}

//-------------------------------------------------------------------------------------
bool initializeWatcher()
{
	return true;
}

}
}
}
