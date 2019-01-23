// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "pyscript/copy.h"
#include "entitydef.h"
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
	enum DCType
	{
		DC_TYPE_UNKNOWN = 0,
		DC_TYPE_ENTITY = 1,
		DC_TYPE_COMPONENT = 2,
		DC_TYPE_INTERFACE = 3,

		DC_TYPE_PROPERTY = 4,
		DC_TYPE_METHOD = 5,
		DC_TYPE_CLIENT_METHOD = 6,

		DC_TYPE_FIXED_DICT = 7,
		DC_TYPE_FIXED_ARRAY = 8,
		DC_TYPE_FIXED_ITEM = 9,

		DC_TYPE_RENAME = 10,
	};

	DefContext()
	{
		optionName = "";

		moduleName = "";
		attrName = "";
		methodArgs = "";
		returnType = "";

		isModuleScope = false;

		exposed = false;
		hasClient = false;
		persistent = -1;
		databaseLength = 0;

		propertyFlags = "";
		propertyIndex = "";
		propertyDefaultVal = "";

		inheritEngineModuleType = DC_TYPE_UNKNOWN;
		type = DC_TYPE_UNKNOWN;
	}

	std::string optionName;

	std::string moduleName;
	std::string attrName;
	std::string methodArgs;
	std::string returnType;

	std::vector< std::string > argsvecs;
	std::map< std::string, std::string > annotationsMaps;

	bool isModuleScope;

	bool exposed;
	bool hasClient;

	// -1：未设置， 0：false， 1：true
	int persistent;

	int databaseLength;

	std::string propertyFlags;
	std::string propertyIndex;
	std::string propertyDefaultVal;

	PyObjectPtr implementedBy;

	std::vector< std::string > baseClasses;

	DCType inheritEngineModuleType;
	DCType type;

	std::vector< DefContext > methods;
	std::vector< DefContext > clienbt_methods;
	std::vector< DefContext > propertys;
	std::vector< std::string > components;

	bool addChildContext(DefContext& defContext)
	{
		std::vector< DefContext >* pContexts = NULL;

		if (defContext.type == DefContext::DC_TYPE_PROPERTY)
			pContexts = &propertys;
		else if (defContext.type == DefContext::DC_TYPE_METHOD)
			pContexts = &methods;
		else if (defContext.type == DefContext::DC_TYPE_CLIENT_METHOD)
			pContexts = &clienbt_methods;
		else if (defContext.type == DefContext::DC_TYPE_FIXED_ITEM)
			pContexts = &propertys;
		else
			KBE_ASSERT(false);

		std::vector< DefContext >::iterator iter = pContexts->begin();
		for (; iter != pContexts->end(); ++iter)
		{
			if ((*iter).attrName == defContext.attrName)
			{
				// 当多次assemblyContexts时可能会发生这样的情况
				// 不做操作即可
				if (moduleName != defContext.moduleName)
					return true;

				return false;
			}
		}

		pContexts->push_back(defContext);
		return true;
	}
};

static std::stack<CallContext> g_callContexts;
static std::string pyDefModuleName = "";

typedef std::map<std::string, DefContext> DEF_CONTEXT_MAP;
static DEF_CONTEXT_MAP g_allScriptDefContextMaps;

//-------------------------------------------------------------------------------------
static bool assemblyContexts(bool notfoundModuleError = false)
{
	std::vector< std::string > dels;

	DEF_CONTEXT_MAP::iterator iter = g_allScriptDefContextMaps.begin();
	for (; iter != g_allScriptDefContextMaps.end(); ++iter)
	{
		DefContext& defContext = iter->second;

		if (defContext.type == DefContext::DC_TYPE_PROPERTY ||
			defContext.type == DefContext::DC_TYPE_METHOD ||
			defContext.type == DefContext::DC_TYPE_CLIENT_METHOD ||
			defContext.type == DefContext::DC_TYPE_FIXED_ITEM)
		{
			DEF_CONTEXT_MAP::iterator fiter = g_allScriptDefContextMaps.find(defContext.moduleName);
			if (fiter == g_allScriptDefContextMaps.end())
			{
				if (notfoundModuleError)
				{
					PyErr_Format(PyExc_AssertionError, "PyEntityDef::process(): No \'%s\' module defined!\n", defContext.moduleName.c_str());
					return false;
				}

				return true;
			}

			if (!fiter->second.addChildContext(defContext))
			{
				PyErr_Format(PyExc_AssertionError, "\'%s.%s\' already exists!\n",
					fiter->second.moduleName.c_str(), defContext.attrName.c_str());

				return false;
			}

			dels.push_back(iter->first);
		}
		else
		{
		}
	}

	std::vector< std::string >::iterator diter = dels.begin();
	for (; diter != dels.end(); ++diter)
	{
		g_allScriptDefContextMaps.erase((*diter));
	}

	// 尝试将父类信息填充到派生类
	iter = g_allScriptDefContextMaps.begin();
	for (; iter != g_allScriptDefContextMaps.end(); ++iter)
	{
		DefContext& defContext = iter->second;
		if (defContext.baseClasses.size() > 0)
		{
			for (size_t i = 0; i < defContext.baseClasses.size(); ++i)
			{
				std::string parentClass = defContext.baseClasses[i];

				DEF_CONTEXT_MAP::iterator fiter = g_allScriptDefContextMaps.find(parentClass);
				if (fiter == g_allScriptDefContextMaps.end())
				{
					//PyErr_Format(PyExc_AssertionError, "not found parentClass(\'%s\')!\n", parentClass.c_str());
					//return false;
					continue;
				}

				DefContext& parentDefContext = fiter->second;
				std::vector< DefContext > childContexts;

				childContexts.insert(childContexts.end(), parentDefContext.methods.begin(), parentDefContext.methods.end());
				childContexts.insert(childContexts.end(), parentDefContext.clienbt_methods.begin(), parentDefContext.clienbt_methods.end());
				childContexts.insert(childContexts.end(), parentDefContext.propertys.begin(), parentDefContext.propertys.end());

				std::vector< DefContext >::iterator itemIter = childContexts.begin();
				for (; itemIter != childContexts.end(); ++itemIter)
				{
					DefContext& parentDefContext = (*itemIter);
					if (!defContext.addChildContext(parentDefContext))
					{
						PyErr_Format(PyExc_AssertionError, "\'%s.%s\' already exists(%s.%s)!\n",
							defContext.moduleName.c_str(), parentDefContext.attrName.c_str(), parentDefContext.moduleName.c_str(), parentDefContext.attrName.c_str());

						return false;
					}
				}
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
static bool registerDefContext(DefContext& defContext)
{
	std::string name = defContext.moduleName;

	if (defContext.type == DefContext::DC_TYPE_PROPERTY ||
		defContext.type == DefContext::DC_TYPE_METHOD ||
		defContext.type == DefContext::DC_TYPE_CLIENT_METHOD ||
		defContext.type == DefContext::DC_TYPE_FIXED_ITEM)
		name += "." + defContext.attrName;

	DEF_CONTEXT_MAP::iterator iter = g_allScriptDefContextMaps.find(name);
	if (iter != g_allScriptDefContextMaps.end())
	{
		PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' already exists!\n", 
			defContext.optionName.c_str(), name.c_str());

		return false;
	}

	g_allScriptDefContextMaps[name] = defContext;
	return assemblyContexts();
}

//-------------------------------------------------------------------------------------
static bool onDefRename(DefContext& defContext)
{
	defContext.type = DefContext::DC_TYPE_RENAME;
	return registerDefContext(defContext);
}

//-------------------------------------------------------------------------------------
static bool onDefFixedDict(DefContext& defContext)
{
	defContext.type = DefContext::DC_TYPE_FIXED_DICT;
	return registerDefContext(defContext);
}

//-------------------------------------------------------------------------------------
static bool onDefFixedArray(DefContext& defContext)
{
	defContext.type = DefContext::DC_TYPE_FIXED_ARRAY;
	return registerDefContext(defContext);
}

//-------------------------------------------------------------------------------------
static bool onDefFixedItem(DefContext& defContext)
{
	defContext.type = DefContext::DC_TYPE_FIXED_ITEM;
	return registerDefContext(defContext);
}

//-------------------------------------------------------------------------------------
static bool onDefProperty(DefContext& defContext)
{
	defContext.type = DefContext::DC_TYPE_PROPERTY;
	return registerDefContext(defContext);
}

//-------------------------------------------------------------------------------------
static bool onDefMethod(DefContext& defContext)
{
	defContext.type = DefContext::DC_TYPE_METHOD;
	return registerDefContext(defContext);
}

//-------------------------------------------------------------------------------------
static bool onDefClientMethod(DefContext& defContext)
{
	defContext.type = DefContext::DC_TYPE_CLIENT_METHOD;
	return registerDefContext(defContext);
}

//-------------------------------------------------------------------------------------
static bool onDefEntity(DefContext& defContext)
{
	defContext.type = DefContext::DC_TYPE_ENTITY;
	return registerDefContext(defContext);
}

//-------------------------------------------------------------------------------------
static bool onDefInterface(DefContext& defContext)
{
	defContext.type = DefContext::DC_TYPE_INTERFACE;
	return registerDefContext(defContext);
}

//-------------------------------------------------------------------------------------
static bool onDefComponent(DefContext& defContext)
{
	defContext.type = DefContext::DC_TYPE_COMPONENT;
	return registerDefContext(defContext);
}

//-------------------------------------------------------------------------------------
static bool isRefEntityDefModule(PyObject *pyObj)
{
	if(!pyObj)
		return true;

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
#define PY_RETURN_ERROR { g_allScriptDefContextMaps.clear(); while(!g_callContexts.empty()) g_callContexts.pop(); return NULL; }

static PyObject* __py_def_parse(PyObject *self, PyObject* args)
{
	CallContext cc = g_callContexts.top();
	g_callContexts.pop();
	
	DefContext defContext;
	defContext.optionName = cc.optionName;

	if (!args || PyTuple_Size(args) < 1)
	{
		PyErr_Format(PyExc_AssertionError, "Def.__py_def_call(Def.%s): error!\n", defContext.optionName.c_str());
		PY_RETURN_ERROR;
	}

	PyObject* pyFunc = PyTuple_GET_ITEM(args, 0);

	PyObject* pyModuleQualname = PyObject_GetAttrString(pyFunc, "__qualname__");
	if (!pyModuleQualname)
	{
		PY_RETURN_ERROR;
	}

	const char* moduleQualname = PyUnicode_AsUTF8AndSize(pyModuleQualname, NULL);
	Py_DECREF(pyModuleQualname);

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
			PY_RETURN_ERROR;
		}

		defContext.exposed = pyExposed == Py_True;
	}
	else if (defContext.optionName == "property" || defContext.optionName == "fixed_item")
	{
		if (defContext.optionName != "fixed_item")
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
				PY_RETURN_ERROR;
			}

			if (!pyFlags || !isRefEntityDefModule(pyFlags))
			{
				PyErr_Format(PyExc_AssertionError, "Def.%s: \'flags\' must be referenced from the [Def.ALL_CLIENTS, Def.*] module!\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			if (!isRefEntityDefModule(pyIndex))
			{
				PyErr_Format(PyExc_AssertionError, "Def.%s: \'index\' must be referenced from the [Def.UNIQUE, Def.INDEX] module!\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			if (pyDatabaseLength && !PyLong_Check(pyDatabaseLength))
			{
				PyErr_Format(PyExc_AssertionError, "Def.%s: \'databaseLength\' error! not a number type.\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			if (pyPersistent && !PyBool_Check(pyPersistent))
			{
				PyErr_Format(PyExc_AssertionError, "Def.%s: \'persistent\' error! not a bool type.\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			defContext.propertyFlags = PyUnicode_AsUTF8AndSize(pyFlags, NULL);

			if (pyPersistent)
				defContext.persistent = pyPersistent == Py_True;

			if (pyIndex)
				defContext.propertyIndex = PyUnicode_AsUTF8AndSize(pyIndex, NULL);

			if (pyDatabaseLength)
				defContext.databaseLength = (int)PyLong_AsLong(pyDatabaseLength);
		}

		// 对于属性， 我们需要获得返回值作为默认值
		PyObject* pyRet = PyObject_CallFunction(pyFunc,
			const_cast<char*>("(O)"), Py_None);

		if (!pyRet)
			return NULL;

		if (pyRet != Py_None)
		{
			PyObject* pyStrResult = PyObject_Str(pyRet);
			defContext.propertyDefaultVal = PyUnicode_AsUTF8AndSize(pyStrResult, NULL);
			Py_DECREF(pyStrResult);

			// 验证这个字符串是否可以还原成对象
			if (defContext.propertyDefaultVal.size() > 0)
			{
				PyObject* module = PyImport_AddModule("__main__");
				if (module == NULL)
					return NULL;

				PyObject* mdict = PyModule_GetDict(module); // Borrowed reference.
				PyObject* result = PyRun_String(const_cast<char*>(defContext.propertyDefaultVal.c_str()),
					Py_eval_input, mdict, mdict);

				if (result == NULL)
					return NULL;

				Py_DECREF(result);
			}
		}

		Py_DECREF(pyRet);
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
			PY_RETURN_ERROR;
		}

		defContext.hasClient = pyHasClient == Py_True;
	}
	else if (defContext.optionName == "interface")
	{
		defContext.isModuleScope = true;
		defContext.inheritEngineModuleType = DefContext::DC_TYPE_INTERFACE;
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
			PY_RETURN_ERROR;
		}

		if (pImplementedBy)
		{
			if (isRefEntityDefModule(pImplementedBy))
			{
				if (std::string(PyUnicode_AsUTF8AndSize(pImplementedBy, NULL)) == "thisClass")
				{
					defContext.implementedBy = pyFunc;
				}
			}
			else
			{
				PyObject* pyQualname = PyObject_GetAttrString(pImplementedBy, "__qualname__");
				if (!pyQualname)
				{
					PY_RETURN_ERROR;
				}

				defContext.implementedBy = pImplementedBy;
				Py_DECREF(pyQualname);
			}
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
	}
	else
	{
		PyErr_Format(PyExc_AssertionError, "Def.%s: not support!\n", defContext.optionName.c_str());
		PY_RETURN_ERROR;
	}

	if (!defContext.isModuleScope)
	{
		std::vector<std::string> outs;

		if (moduleQualname)
			strutil::kbe_splits(moduleQualname, ".", outs);

		if (defContext.optionName != "rename")
		{
			if (outs.size() != 2)
			{
				if(PyFunction_Check(pyFunc))
					PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' must be defined in the entity module!\n", 
						defContext.optionName.c_str(), moduleQualname);
				else
					PyErr_Format(PyExc_AssertionError, "Def.%s: please check the command format is: Def.%s(..)\n", 
						defContext.optionName.c_str(), defContext.optionName.c_str());

				PY_RETURN_ERROR;
			}

			defContext.moduleName = outs[0];
			defContext.attrName = outs[1];
		}
		else
		{
			if (outs.size() != 1)
			{
				PyErr_Format(PyExc_AssertionError, "Def.%s: error! such as: @Def.rename()\n\tdef ENTITY_ID() -> int: pass\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			defContext.moduleName = outs[0];
		}

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
			PY_RETURN_ERROR;
		}

		if (pyGetfullargspec)
		{
			PyObject* pyGetMethodArgs = PyObject_CallFunction(pyGetfullargspec,
				const_cast<char*>("(O)"), pyFunc);

			if (!pyGetMethodArgs)
			{
				PY_RETURN_ERROR;
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
					PY_RETURN_ERROR;
				}

				PyObjectPtr pyGetMethodArgsResultPtr = pyGetMethodArgsResult;
				PyObjectPtr pyGetMethodAnnotationsResultPtr = pyGetMethodAnnotationsResult;
				Py_DECREF(pyGetMethodArgsResult);
				Py_DECREF(pyGetMethodAnnotationsResult);

				if (defContext.optionName != "rename")
				{
					Py_ssize_t argsSize = PyList_Size(pyGetMethodArgsResult);
					if (argsSize == 0)
					{
						PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' did not find \'self\' parameter!\n", defContext.optionName.c_str(), moduleQualname);
						PY_RETURN_ERROR;
					}

					for (Py_ssize_t i = 1; i < argsSize; ++i)
					{
						PyObject* pyItem = PyList_GetItem(pyGetMethodArgsResult, i);

						const char* ccattr = PyUnicode_AsUTF8AndSize(pyItem, NULL);
						if (!ccattr)
						{
							PY_RETURN_ERROR;
						}

						defContext.argsvecs.push_back(ccattr);
					}
				}

				PyObject *key, *value;
				Py_ssize_t pos = 0;

				while (PyDict_Next(pyGetMethodAnnotationsResult, &pos, &key, &value)) {
					const char* skey = PyUnicode_AsUTF8AndSize(key, NULL);
					if (!skey)
					{
						PY_RETURN_ERROR;
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
							PY_RETURN_ERROR;

						svalue = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
						Py_DECREF(pyQualname);
					}

					if (svalue.size() == 0)
					{
						PY_RETURN_ERROR;
					}

					if (std::string(skey) == "return")
						defContext.returnType = svalue;
					else
						defContext.annotationsMaps[skey] = svalue;
				}
			}
		}
	}
	else
	{
		defContext.moduleName = moduleQualname;

		PyObject* pyBases = PyObject_GetAttrString(pyFunc, "__bases__");
		if (!pyBases)
			PY_RETURN_ERROR;

		Py_ssize_t basesSize = PyTuple_Size(pyBases);
		if (basesSize == 0)
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' does not inherit the KBEngine.Entity class!\n", defContext.optionName.c_str(), moduleQualname);
			Py_XDECREF(pyBases);
			PY_RETURN_ERROR;
		}

		for (Py_ssize_t i = 0; i < basesSize; ++i)
		{
			PyObject* pyClass = PyTuple_GetItem(pyBases, i);

			PyObject* pyQualname = PyObject_GetAttrString(pyClass, "__qualname__");
			if (!pyQualname)
			{
				Py_XDECREF(pyBases);
				PY_RETURN_ERROR;
			}

			std::string parentClass = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
			Py_DECREF(pyQualname);

			if (parentClass == "object")
			{
				continue;
			}
			else if (parentClass == "Entity" || parentClass == "Proxy")
			{
				defContext.inheritEngineModuleType = DefContext::DC_TYPE_ENTITY;
				continue;
			}
			else if (parentClass == "EntityComponent")
			{
				defContext.inheritEngineModuleType = DefContext::DC_TYPE_COMPONENT;
				continue;
			}

			defContext.baseClasses.push_back(parentClass);
		}

		Py_XDECREF(pyBases);
	}

	bool noerror = true;

	if (defContext.optionName == "method" || defContext.optionName == "clientmethod")
	{
		if (defContext.annotationsMaps.size() != defContext.argsvecs.size())
		{
			PyErr_Format(PyExc_AssertionError, "Def.%s: \'%s\' all parameters must have annotations!\n", defContext.optionName.c_str(), moduleQualname);
			PY_RETURN_ERROR;
		}

		if (defContext.optionName == "method")
			noerror = onDefMethod(defContext);
		else
			noerror = onDefClientMethod(defContext);
	}
	else if (defContext.optionName == "rename")
	{
		noerror = onDefRename(defContext);
	}
	else if (defContext.optionName == "property")
	{
		noerror = onDefProperty(defContext);
	}
	else if (defContext.optionName == "entity")
	{
		noerror = onDefEntity(defContext);
	}
	else if (defContext.optionName == "interface")
	{
		noerror = onDefInterface(defContext);
	}
	else if (defContext.optionName == "component")
	{
		noerror = onDefComponent(defContext);
	}
	else if (defContext.optionName == "fixed_dict")
	{
		noerror = onDefFixedDict(defContext);
	}
	else if (defContext.optionName == "fixed_array")
	{
		noerror = onDefFixedArray(defContext);
	}
	else if (defContext.optionName == "fixed_item")
	{
		noerror = onDefFixedItem(defContext);
	}

	if (!noerror)
	{
		PY_RETURN_ERROR;
	}

	Py_INCREF(pyFunc);
	return pyFunc;
}

//-------------------------------------------------------------------------------------
static PyMethodDef __call_def_parse = { "_PyEntityDefParse", (PyCFunction)&__py_def_parse, METH_VARARGS, 0 };

#define PY_DEF_HOOK(NAME)	\
	static PyObject* __py_def_##NAME(PyObject* self, PyObject* args, PyObject* kwargs)	\
	{	\
		CallContext cc;	\
		cc.pyArgs = PyObjectPtr(Copy::deepcopy(args));	\
		cc.pyKwargs = kwargs ? PyObjectPtr(Copy::deepcopy(kwargs)) : PyObjectPtr(NULL);	\
		cc.optionName = #NAME;	\
		g_callContexts.push(cc);	\
		Py_XDECREF(cc.pyArgs.get());	\
		Py_XDECREF(cc.pyKwargs.get());	\
		\
		return PyCFunction_New(&__call_def_parse, self);	\
	}

static PyObject* __py_def_rename(PyObject* self, PyObject* args, PyObject* kwargs)
	{
		CallContext cc;
		cc.pyArgs = PyObjectPtr(Copy::deepcopy(args));
		cc.pyKwargs = kwargs ? PyObjectPtr(Copy::deepcopy(kwargs)) : PyObjectPtr(NULL);
		cc.optionName = "rename";
		
		Py_XDECREF(cc.pyArgs.get());
		Py_XDECREF(cc.pyKwargs.get());

		// 类似这种定义方式 Def.rename(ENTITY_ID=int)
		if (kwargs)
		{
			PyObject *key, *value;
			Py_ssize_t pos = 0;

			while (PyDict_Next(kwargs, &pos, &key, &value)) {
				if (!PyType_Check(value))
				{
					PyErr_Format(PyExc_AssertionError, "Def.%s: arg2 not legal type! such as: Def.rename(ENTITY_ID=int)\n", cc.optionName.c_str());
					return NULL;
				}

				PyObject* pyQualname = PyObject_GetAttrString(value, "__qualname__");
				if (!pyQualname)
				{
					PyErr_Format(PyExc_AssertionError, "Def.%s: arg2 get __qualname__ error! such as: Def.rename(ENTITY_ID=int)\n", cc.optionName.c_str());
					return NULL;
				}

				std::string typeName = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
				Py_DECREF(pyQualname);

				if (!PyUnicode_Check(key))
				{
					PyErr_Format(PyExc_AssertionError, "Def.%s: arg1 must be a string! such as: Def.rename(ENTITY_ID=int)\n", cc.optionName.c_str());
					return NULL;
				}

				DefContext defContext;
				defContext.optionName = cc.optionName;
				defContext.moduleName = PyUnicode_AsUTF8AndSize(key, NULL);
				defContext.returnType = typeName;

				if (!onDefRename(defContext))
					return NULL;
			}

			S_Return;
		}

		g_callContexts.push(cc);

		// @Def.rename()
		// def ENTITY_ID() -> int: pass
		return PyCFunction_New(&__call_def_parse, self);
	}

#define PY_ADD_METHOD(NAME, DOCS) APPEND_SCRIPT_MODULE_METHOD(entitydefModule, NAME, __py_def_##NAME, METH_VARARGS | METH_KEYWORDS, 0);

#ifdef interface
#undef interface
#endif

#ifdef property
#undef property
#endif

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
	while (!g_callContexts.empty()) g_callContexts.pop();
	g_allScriptDefContextMaps.clear();
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

	static const char* thisClass = "thisClass";
	if (PyModule_AddStringConstant(entitydefModule, thisClass, thisClass))
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

//-------------------------------------------------------------------------------------
static bool loadAllScripts()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool process()
{
	if (!loadAllScripts())
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}

	if (!assemblyContexts(true))
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}

	while (!g_callContexts.empty()) 
		g_callContexts.pop();

	g_allScriptDefContextMaps.clear();
	return true;
}

}
}
}
