// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "pyscript/copy.h"
#include "py_entitydef.h"

namespace KBEngine{ namespace script{ namespace entitydef {

static PyObject* g_pyArgs = NULL;
static PyObject* g_pyKwargs = NULL;
static std::string g_optionName = "";
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
static PyObject* __py_def_call(PyObject *self, PyObject* args)
{
	std::string pyEntityModuleName = "";
	std::string pyEntityAttrName = "";
	std::string pyEntityMethodArgs = "";
	std::string pyReturnType = "";
	std::vector< std::string > argsvecs;
	std::map< std::string, std::string > annotationsMaps;

	bool exposed = false;
	std::string propertyFlags = "";
	std::string propertyIndex = "";
	bool persistent = false;
	int databaseLength = 0;

	if (g_optionName == "method")
	{
		static char * keywords[] =
		{
			const_cast<char *> ("exposed"),
			NULL
		};

		PyObject* pyExposed = NULL;

		if (!PyArg_ParseTupleAndKeywords(g_pyArgs, g_pyKwargs, "|O",
			keywords, &pyExposed))
		{
			return NULL;
		}

		exposed = pyExposed == Py_True;
	}
	else if (g_optionName == "property")
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

		if (!PyArg_ParseTupleAndKeywords(g_pyArgs, g_pyKwargs, "|OOOO",
			keywords, &pyFlags, &pyPersistent, &pyIndex, &pyDatabaseLength))
		{
			return NULL;
		}

		if (!isRefEntityDefModule(pyFlags))
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'flags\' must be referenced from the [Def.ALL_CLIENTS, Def.*] module!\n", g_optionName.c_str());
			return NULL;
		}

		if (!isRefEntityDefModule(pyIndex))
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'index\' must be referenced from the [Def.UNIQUE, Def.INDEX] module!\n", g_optionName.c_str());
			return NULL;
		}

		if (pyDatabaseLength && PyLong_Check(pyDatabaseLength))
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'databaseLength\' error! not a number type.\n", g_optionName.c_str());
			return NULL;
		}

		if (pyPersistent && PyBool_Check(pyPersistent))
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'persistent\' error! not a bool type.\n", g_optionName.c_str());
			return NULL;
		}

		propertyFlags = PyUnicode_AsUTF8AndSize(pyFlags, NULL);

		if(pyPersistent)
			persistent = pyPersistent == Py_True;

		if (pyIndex)
			propertyIndex = PyUnicode_AsUTF8AndSize(pyIndex, NULL);

		if (pyDatabaseLength)
			databaseLength = (int)PyLong_AsLong(pyDatabaseLength);
	}

	Py_XDECREF(g_pyArgs);
	Py_XDECREF(g_pyKwargs);

	if (!args || PyTuple_Size(args) < 1)
	{
		PyErr_Format(PyExc_AssertionError, "Def.__py_def_call(Def.%s): error!\n", g_optionName.c_str());
		return NULL;
	}

	PyObject* pyQualname = PyObject_GetAttrString(PyTuple_GET_ITEM(args, 0), "__qualname__");
	if (!pyQualname)
	{
		return NULL;
	}

	const char* qualname = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
	Py_DECREF(pyQualname);
	std::vector<std::string> outs;

	if(qualname)
		strutil::kbe_splits(qualname, ".", outs);

	if (outs.size() != 2)
	{
		PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' must be defined in the entity module!\n", g_optionName.c_str(), qualname);
		return NULL;
	}

	pyEntityModuleName = outs[0];
	pyEntityAttrName = outs[1];

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

	PyObject* pyFunc = PyTuple_GET_ITEM(args, 0);

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

			Py_ssize_t argsSize = PyList_Size(pyGetMethodArgsResult);
			if (argsSize == 0)
			{
				PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' did not find \'self\' parameter!\n", g_optionName.c_str(), qualname);
				Py_XDECREF(pyGetMethodArgsResult);
				Py_XDECREF(pyGetMethodAnnotationsResult);
				return NULL;
			}

			for (Py_ssize_t i = 1; i < argsSize; ++i)
			{
				PyObject* pyItem = PyList_GetItem(pyGetMethodArgsResult, i);

				const char* ccattr = PyUnicode_AsUTF8AndSize(pyItem, NULL);
				if (!ccattr)
				{
					Py_XDECREF(pyGetMethodArgsResult);
					Py_XDECREF(pyGetMethodAnnotationsResult);
					return NULL;
				}

				argsvecs.push_back(ccattr);

				PyErr_Format(PyExc_AssertionError, "Def.%s: -------arg---------------- %s!\n", g_optionName.c_str(), ccattr);
				PyErr_PrintEx(0);
			}

			PyObject *key, *value;
			Py_ssize_t pos = 0;

			while (PyDict_Next(pyGetMethodAnnotationsResult, &pos, &key, &value)) {
				const char* skey = PyUnicode_AsUTF8AndSize(key, NULL);
				if (!skey)
				{
					Py_XDECREF(pyGetMethodArgsResult);
					Py_XDECREF(pyGetMethodAnnotationsResult);
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
					Py_XDECREF(pyGetMethodArgsResult);
					Py_XDECREF(pyGetMethodAnnotationsResult);
					return NULL;
				}

				if (skey == "return")
					pyReturnType = svalue;
				else
					annotationsMaps[skey] = svalue;

				PyErr_Format(PyExc_AssertionError, "Def.%s: -------annotations---------------- %s.%s!\n", g_optionName.c_str(), skey, svalue.c_str());
				PyErr_PrintEx(0);
			}

			Py_XDECREF(pyGetMethodArgsResult);
			Py_XDECREF(pyGetMethodAnnotationsResult);
		}
	}

	if (g_optionName == "method")
	{
		if (annotationsMaps.size() != argsvecs.size())
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' all parameters must have annotations!\n", g_optionName.c_str(), qualname);
			return NULL;
		}
	}

	Py_INCREF(pyFunc);
	return pyFunc;
}

//-------------------------------------------------------------------------------------
static PyObject* __py_def_method(PyObject *self, PyObject* args, PyObject * kwargs)
{
	g_pyArgs = Copy::deepcopy(args);
	g_pyKwargs = Copy::deepcopy(kwargs);
	g_optionName = "method";

	static PyMethodDef __call__Method =
	{ "_PyEntityDefCall", (PyCFunction)&__py_def_call, METH_VARARGS, 0 };

	return PyCFunction_New(&__call__Method, self);
}

//-------------------------------------------------------------------------------------
static PyObject* __py_def_property(PyObject *self, PyObject* args, PyObject * kwargs)
{
	g_pyArgs = Copy::deepcopy(args);
	g_pyKwargs = Copy::deepcopy(kwargs);
	g_optionName = "property";

	static PyMethodDef __call__Method =
	{ "_PyEntityDefCall", (PyCFunction)&__py_def_call, METH_VARARGS, 0 };

	return PyCFunction_New(&__call__Method, self);
}

//-------------------------------------------------------------------------------------
bool installModule(const char* moduleName)
{
	pyDefModuleName = moduleName;

	PyObject *entitydefModule = PyImport_AddModule(pyDefModuleName.c_str());
	PyObject_SetAttrString(entitydefModule, "__doc__", PyUnicode_FromString("This module is created by KBEngine!"));

	APPEND_SCRIPT_MODULE_METHOD(entitydefModule, method, __py_def_method, METH_VARARGS | METH_KEYWORDS, 0);
	APPEND_SCRIPT_MODULE_METHOD(entitydefModule, property, __py_def_property, METH_VARARGS | METH_KEYWORDS, 0);
	return true;
}

//-------------------------------------------------------------------------------------
bool uninstallModule()
{

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
