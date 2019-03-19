// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include <stack>
#include <future>
#include <chrono>

#include "common.h"
#include "entitydef.h"
#include "datatypes.h"
#include "py_entitydef.h"
#include "scriptdef_module.h"
#include "pyscript/py_platform.h"
#include "pyscript/script.h"
#include "pyscript/copy.h"
#include "resmgr/resmgr.h"

namespace KBEngine{ namespace script{ namespace entitydef {

struct CallContext
{
	PyObjectPtr pyArgs;
	PyObjectPtr pyKwargs;
	std::string optionName;
};

static std::stack<CallContext> g_callContexts;
static std::string pyDefModuleName = "";

DefContext::DEF_CONTEXT_MAP DefContext::allScriptDefContextMaps;
DefContext::DEF_CONTEXT_MAP DefContext::allScriptDefContextLineMaps;

static bool g_inited = false;

//-------------------------------------------------------------------------------------
static PyObject* __py_array(PyObject* self, PyObject* args)
{
	if (PyTuple_GET_SIZE(args) == 0)
	{
		PyErr_Format(PyExc_AssertionError, "EntityDef.ARRAY: does not set itemType! should be like this \"EntityDef.ARRAY(itemType)\"\n");
		return NULL;
	}

	PyObject *entitydefModule = PyImport_AddModule(pyDefModuleName.c_str());
	PyObject* pyARRAY = PyObject_GetAttrString(entitydefModule, "ARRAY");

	PyObject* pyArrayItemType = PyTuple_GET_ITEM(args, 0);
	Py_INCREF(pyArrayItemType);

	PyObject* ret = PyTuple_New(2);
	PyTuple_SET_ITEM(ret, 0, pyARRAY);
	PyTuple_SET_ITEM(ret, 1, pyArrayItemType);

	return ret;
}

//-------------------------------------------------------------------------------------
class Entity : public script::ScriptObject
{
	BASE_SCRIPT_HREADER(Entity, ScriptObject)
public:
	Entity(PyTypeObject* pyType = getScriptType(), bool isInitialised = true) :
		ScriptObject(pyType, isInitialised) {}
	~Entity() {}
};

SCRIPT_METHOD_DECLARE_BEGIN(Entity)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Entity)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Entity)
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Entity, 0, 0, 0, 0, 0)

//-------------------------------------------------------------------------------------
class Space : public script::ScriptObject
{
	BASE_SCRIPT_HREADER(Space, ScriptObject)
public:
	Space(PyTypeObject* pyType = getScriptType(), bool isInitialised = true) :
		ScriptObject(pyType, isInitialised) {}
	~Space() {}
};

SCRIPT_METHOD_DECLARE_BEGIN(Space)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Space)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Space)
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Space, 0, 0, 0, 0, 0)

//-------------------------------------------------------------------------------------
class Proxy : public script::ScriptObject
{
	BASE_SCRIPT_HREADER(Proxy, ScriptObject)
public:
	Proxy(PyTypeObject* pyType = getScriptType(), bool isInitialised = true) :
		ScriptObject(pyType, isInitialised) {}
	~Proxy() {}
};

SCRIPT_METHOD_DECLARE_BEGIN(Proxy)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Proxy)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Proxy)
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Proxy, 0, 0, 0, 0, 0)

//-------------------------------------------------------------------------------------
class EntityComponent : public script::ScriptObject
{
	BASE_SCRIPT_HREADER(EntityComponent, ScriptObject)
public:
	EntityComponent(PyTypeObject* pyType = getScriptType(), bool isInitialised = true) :
		ScriptObject(pyType, isInitialised) {}
	~EntityComponent() {}
};

SCRIPT_METHOD_DECLARE_BEGIN(EntityComponent)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(EntityComponent)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(EntityComponent)
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(EntityComponent, 0, 0, 0, 0, 0)

//-------------------------------------------------------------------------------------
DefContext::DefContext()
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
	utype = -1;
	detailLevel = "";

	propertyFlags = "";
	propertyIndex = "";
	propertyDefaultVal = "";

	implementedByModuleName = "";
	implementedByModuleFile = "";
	pyObjectSourceFile = "";

	inheritEngineModuleType = DC_TYPE_UNKNOWN;
	type = DC_TYPE_UNKNOWN;

	componentType = UNKNOWN_COMPONENT_TYPE;
}

//-------------------------------------------------------------------------------------
bool DefContext::addToStream(MemoryStream* pMemoryStream)
{
	(*pMemoryStream) << optionName;
	(*pMemoryStream) << moduleName;
	(*pMemoryStream) << attrName;
	(*pMemoryStream) << methodArgs;
	(*pMemoryStream) << returnType;

	(*pMemoryStream) << (int)argsvecs.size();
	std::vector< std::string >::iterator argsvecsIter = argsvecs.begin();
	for(; argsvecsIter != argsvecs.end(); ++argsvecsIter)
		(*pMemoryStream) << (*argsvecsIter);

	(*pMemoryStream) << (int)annotationsMaps.size();
	std::map< std::string, std::string >::iterator annotationsMapsIter = annotationsMaps.begin();
	for (; annotationsMapsIter != annotationsMaps.end(); ++annotationsMapsIter)
		(*pMemoryStream) << annotationsMapsIter->first << annotationsMapsIter->second;

	(*pMemoryStream) << isModuleScope;
	(*pMemoryStream) << exposed;
	(*pMemoryStream) << hasClient;

	(*pMemoryStream) << persistent;
	(*pMemoryStream) << databaseLength;
	(*pMemoryStream) << utype;
	(*pMemoryStream) << detailLevel;
	
	(*pMemoryStream) << propertyFlags;
	(*pMemoryStream) << propertyIndex;
	(*pMemoryStream) << propertyDefaultVal;

	(*pMemoryStream) << implementedByModuleName;
	(*pMemoryStream) << implementedByModuleFile;
	(*pMemoryStream) << pyObjectSourceFile;

	(*pMemoryStream) << (int)baseClasses.size();
	std::vector< std::string >::iterator baseClassesIter = baseClasses.begin();
	for (; baseClassesIter != baseClasses.end(); ++baseClassesIter)
		(*pMemoryStream) << (*baseClassesIter);

	(*pMemoryStream) << (int)inheritEngineModuleType;
	(*pMemoryStream) << (int)type;

	(*pMemoryStream) << (int)methods.size();
	std::vector< DefContext >::iterator methodsIter = methods.begin();
	for (; methodsIter != methods.end(); ++methodsIter)
		(*methodsIter).addToStream(pMemoryStream);

	(*pMemoryStream) << (int)client_methods.size();
	std::vector< DefContext >::iterator client_methodsIter = client_methods.begin();
	for (; client_methodsIter != client_methods.end(); ++client_methodsIter)
		(*client_methodsIter).addToStream(pMemoryStream);

	(*pMemoryStream) << (int)propertys.size();
	std::vector< DefContext >::iterator propertysIter = propertys.begin();
	for (; propertysIter != propertys.end(); ++propertysIter)
		(*propertysIter).addToStream(pMemoryStream);

	(*pMemoryStream) << (int)components.size();
	std::vector< std::string >::iterator componentsIter = components.begin();
	for (; componentsIter != components.end(); ++componentsIter)
		(*pMemoryStream) << (*componentsIter);

	return true;
}

//-------------------------------------------------------------------------------------
bool DefContext::createFromStream(MemoryStream* pMemoryStream)
{
	(*pMemoryStream) >> optionName;
	(*pMemoryStream) >> moduleName;
	(*pMemoryStream) >> attrName;
	(*pMemoryStream) >> methodArgs;
	(*pMemoryStream) >> returnType;

	int size = 0;

	(*pMemoryStream) >> size;
	for (int i = 0; i < size; ++i)
	{
		std::string str;
		(*pMemoryStream) >> str;

		argsvecs.push_back(str);
	}

	(*pMemoryStream) >> size;
	for (int i = 0; i < size; ++i)
	{
		std::string key, val;
		(*pMemoryStream) >> key >> val;

		annotationsMaps[key] = val;
	}

	(*pMemoryStream) >> isModuleScope;
	(*pMemoryStream) >> exposed;
	(*pMemoryStream) >> hasClient;

	(*pMemoryStream) >> persistent;
	(*pMemoryStream) >> databaseLength;
	(*pMemoryStream) >> utype;
	(*pMemoryStream) >> detailLevel;

	(*pMemoryStream) >> propertyFlags;
	(*pMemoryStream) >> propertyIndex;
	(*pMemoryStream) >> propertyDefaultVal;

	(*pMemoryStream) >> implementedByModuleName;
	(*pMemoryStream) >> implementedByModuleFile;
	(*pMemoryStream) >> pyObjectSourceFile;

	(*pMemoryStream) >> size;
	for (int i = 0; i < size; ++i)
	{
		std::string str;
		(*pMemoryStream) >> str;

		baseClasses.push_back(str);
	}

	int t_inheritEngineModuleType;
	(*pMemoryStream) >> t_inheritEngineModuleType;
	inheritEngineModuleType = (DCType)t_inheritEngineModuleType;

	int t_type;
	(*pMemoryStream) >> t_type;
	type = (DCType)t_type;

	(*pMemoryStream) >> size;
	for (int i = 0; i < size; ++i)
	{
		DefContext dc;
		dc.createFromStream(pMemoryStream);

		methods.push_back(dc);
	}

	(*pMemoryStream) >> size;
	for (int i = 0; i < size; ++i)
	{
		DefContext dc;
		dc.createFromStream(pMemoryStream);

		client_methods.push_back(dc);
	}

	(*pMemoryStream) >> size;
	for (int i = 0; i < size; ++i)
	{
		DefContext dc;
		dc.createFromStream(pMemoryStream);

		propertys.push_back(dc);
	}

	(*pMemoryStream) >> size;
	for (int i = 0; i < size; ++i)
	{
		std::string str;
		(*pMemoryStream) >> str;

		components.push_back(str);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DefContext::addChildContext(DefContext& defContext)
{
	std::vector< DefContext >* pContexts = NULL;

	if (defContext.type == DefContext::DC_TYPE_PROPERTY)
		pContexts = &propertys;
	else if (defContext.type == DefContext::DC_TYPE_METHOD)
		pContexts = &methods;
	else if (defContext.type == DefContext::DC_TYPE_CLIENT_METHOD)
		pContexts = &client_methods;
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
			if (moduleName != defContext.moduleName || (*iter).pyObjectSourceFile == defContext.pyObjectSourceFile)
				return true;

			return false;
		}
	}

	pContexts->push_back(defContext);
	return true;
}

//-------------------------------------------------------------------------------------
static bool assemblyContexts(bool notfoundModuleError = false)
{
	std::vector< std::string > dels;

	DefContext::DEF_CONTEXT_MAP::iterator iter = DefContext::allScriptDefContextMaps.begin();
	for (; iter != DefContext::allScriptDefContextMaps.end(); ++iter)
	{
		DefContext& defContext = iter->second;

		if (defContext.type == DefContext::DC_TYPE_PROPERTY ||
			defContext.type == DefContext::DC_TYPE_METHOD ||
			defContext.type == DefContext::DC_TYPE_CLIENT_METHOD ||
			defContext.type == DefContext::DC_TYPE_FIXED_ITEM)
		{
			DefContext::DEF_CONTEXT_MAP::iterator fiter = DefContext::allScriptDefContextMaps.find(defContext.moduleName);
			if (fiter == DefContext::allScriptDefContextMaps.end())
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
		DefContext::allScriptDefContextMaps.erase((*diter));
	}

	// 尝试将父类信息填充到派生类
	iter = DefContext::allScriptDefContextMaps.begin();
	for (; iter != DefContext::allScriptDefContextMaps.end(); ++iter)
	{
		DefContext& defContext = iter->second;
		if (defContext.baseClasses.size() > 0)
		{
			for (size_t i = 0; i < defContext.baseClasses.size(); ++i)
			{
				std::string parentClass = defContext.baseClasses[i];

				DefContext::DEF_CONTEXT_MAP::iterator fiter = DefContext::allScriptDefContextMaps.find(parentClass);
				if (fiter == DefContext::allScriptDefContextMaps.end())
				{
					//PyErr_Format(PyExc_AssertionError, "not found parentClass(\'%s\')!\n", parentClass.c_str());
					//return false;
					continue;
				}

				DefContext& parentDefContext = fiter->second;
				std::vector< DefContext > childContexts;

				childContexts.insert(childContexts.end(), parentDefContext.methods.begin(), parentDefContext.methods.end());
				childContexts.insert(childContexts.end(), parentDefContext.client_methods.begin(), parentDefContext.client_methods.end());
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
	DefContext::allScriptDefContextLineMaps[defContext.pyObjectSourceFile] = defContext;

	std::string name = defContext.moduleName;

	if (defContext.type == DefContext::DC_TYPE_PROPERTY)
	{
		if(!EntityDef::validDefPropertyName(name))
		{
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: '%s' is limited!\n\n",
				defContext.optionName.c_str(), name.c_str());

			return false;
		}

		// 检查作用域是否属于该进程
		bool flagsGood = true;
		if (defContext.componentType == BASEAPP_TYPE)
			flagsGood = (stringToEntityDataFlags(defContext.propertyFlags) & ENTITY_BASE_DATA_FLAGS) != 0;
		else if (defContext.componentType == CELLAPP_TYPE)
			flagsGood = (stringToEntityDataFlags(defContext.propertyFlags) & ENTITY_CELL_DATA_FLAGS) != 0;
		else if (defContext.componentType == CLIENT_TYPE)
			flagsGood = (stringToEntityDataFlags(defContext.propertyFlags) & ENTITY_CLIENT_DATA_FLAGS) != 0;

		name += "." + defContext.attrName;

		if (!flagsGood)
		{
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: '%s'(%s) not a valid %s property flags!\n\n",
				defContext.optionName.c_str(), name.c_str(), defContext.propertyFlags.c_str(), COMPONENT_NAME_EX(defContext.componentType));

			return false;
		}
	}
	else if(defContext.type == DefContext::DC_TYPE_METHOD ||
		defContext.type == DefContext::DC_TYPE_CLIENT_METHOD)
	{
		name += "." + defContext.attrName;
	}
	else if (defContext.type == DefContext::DC_TYPE_FIXED_ITEM)
	{
		name += "." + defContext.attrName;
	}
	else if (defContext.type == DefContext::DC_TYPE_FIXED_ARRAY ||
		defContext.type == DefContext::DC_TYPE_FIXED_DICT ||
		defContext.type == DefContext::DC_TYPE_RENAME)
	{
		if (!DataTypes::validTypeName(name))
		{
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: Not allowed to use the prefix \"_\"! typeName=%s\n",
				defContext.optionName.c_str(), name.c_str());

			return false;
		}
	}

	DefContext::DEF_CONTEXT_MAP::iterator iter = DefContext::allScriptDefContextMaps.find(name);
	if (iter != DefContext::allScriptDefContextMaps.end())
	{
		if (iter->second.pyObjectSourceFile != defContext.pyObjectSourceFile)
		{
			// 如果是不同进程的脚本部分，那么需要进行合并注册
			if (iter->second.componentType != defContext.componentType && DefContext::allScriptDefContextLineMaps.find(defContext.pyObjectSourceFile) != DefContext::allScriptDefContextLineMaps.end() &&
				(defContext.type == DefContext::DC_TYPE_ENTITY || defContext.type == DefContext::DC_TYPE_COMPONENT || defContext.type == DefContext::DC_TYPE_INTERFACE) && 
				iter->second.type == defContext.type)
			{
				std::vector< std::string >::iterator bciter = defContext.baseClasses.begin();
				for (; bciter != defContext.baseClasses.end(); ++bciter)
				{
					if (std::find(iter->second.baseClasses.begin(), iter->second.baseClasses.end(), (*bciter)) == iter->second.baseClasses.end())
					{
						iter->second.baseClasses.push_back((*bciter));
					}

					KBE_ASSERT(defContext.methods.size() == 0 && defContext.client_methods.size() == 0 && defContext.components.size() == 0 && defContext.propertys.size() == 0);
					if (defContext.hasClient)
						iter->second.hasClient = true;

				}

				return true;
			}

			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'%s\' already exists!\n",
				defContext.optionName.c_str(), name.c_str());

			return false;
		}

		return true;
	}

	DefContext::allScriptDefContextMaps[name] = defContext;
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
#define PY_RETURN_ERROR { DefContext::allScriptDefContextLineMaps.clear(); DefContext::allScriptDefContextMaps.clear(); while(!g_callContexts.empty()) g_callContexts.pop(); return NULL; }

#define PYOBJECT_SOURCEFILE(PYOBJ, OUT)	\
{	\
	PyObject* pyInspectModule =	\
	PyImport_ImportModule(const_cast<char*>("inspect"));	\
	\
	PyObject* pyGetsourcefile = NULL;	\
	PyObject* pyGetLineno = NULL;	\
	PyObject* pyGetCurrentFrame = NULL;	\
	\
	if (pyInspectModule)	\
	{	\
		pyGetsourcefile =	\
			PyObject_GetAttrString(pyInspectModule, const_cast<char *>("getsourcefile"));	\
		pyGetLineno =	\
			PyObject_GetAttrString(pyInspectModule, const_cast<char *>("getlineno"));	\
		pyGetCurrentFrame =	\
			PyObject_GetAttrString(pyInspectModule, const_cast<char *>("currentframe"));	\
		\
		Py_DECREF(pyInspectModule);	\
	}	\
	else	\
	{	\
		PY_RETURN_ERROR;	\
	}	\
	\
	if (pyGetsourcefile)	\
	{	\
		PyObject* pyFile = PyObject_CallFunction(pyGetsourcefile, \
			const_cast<char*>("(O)"), PYOBJ);	\
		\
		Py_DECREF(pyGetsourcefile);	\
		\
		if (!pyFile)	\
		{	\
			Py_XDECREF(pyGetLineno);	\
			Py_XDECREF(pyGetCurrentFrame);	\
			PY_RETURN_ERROR;	\
		}	\
		else	\
		{	\
			/* 防止不同系统造成的路径不一致，剔除系统相关路径 */		\
			OUT = PyUnicode_AsUTF8AndSize(pyFile, NULL);	\
			strutil::kbe_replace(OUT, "\\\\", "/");	\
			strutil::kbe_replace(OUT, "\\", "/");	\
			strutil::kbe_replace(OUT, "//", "/");	\
			std::string kbe_root = Resmgr::getSingleton().getPyUserScriptsPath();	\
			strutil::kbe_replace(kbe_root, "\\\\", "/");	\
			strutil::kbe_replace(kbe_root, "\\", "/");	\
			strutil::kbe_replace(kbe_root, "/", "/");	\
			strutil::kbe_replace(OUT, kbe_root, "");	\
			Py_DECREF(pyFile);	\
		}	\
	}	\
	else	\
	{	\
		Py_XDECREF(pyGetLineno);	\
		Py_XDECREF(pyGetCurrentFrame);	\
		PY_RETURN_ERROR;	\
	}	\
	\
	if (pyGetLineno)	\
	{	\
		if(!pyGetCurrentFrame)	\
		{	\
			Py_DECREF(pyGetLineno);	\
		}	\
		\
		PyObject* pyCurrentFrame = PyObject_CallFunction(pyGetCurrentFrame, \
				const_cast<char*>("()"));	\
		\
		PyObject* pyLine = PyObject_CallFunction(pyGetLineno, \
			const_cast<char*>("(O)"), pyCurrentFrame);	\
		\
		Py_DECREF(pyGetLineno);	\
		Py_DECREF(pyGetCurrentFrame);	\
		Py_DECREF(pyCurrentFrame);	\
		\
		if (!pyLine)	\
		{	\
			PY_RETURN_ERROR;	\
		}	\
		else	\
		{	\
			/* 加上行号，避免同文件中多次定义 */		\
			OUT += fmt::format("#{}",  PyLong_AsLong(pyLine));	\
			Py_DECREF(pyLine);	\
		}	\
	}	\
	else	\
	{	\
		Py_XDECREF(pyGetCurrentFrame);	\
		PY_RETURN_ERROR;	\
	}	\
}

static PyObject* __py_def_parse(PyObject *self, PyObject* args)
{
	CallContext cc = g_callContexts.top();
	g_callContexts.pop();
	
	DefContext defContext;
	defContext.optionName = cc.optionName;

	PyObject* kbeModule = PyImport_AddModule("KBEngine");
	KBE_ASSERT(kbeModule);

	PyObject* pyComponentName = PyObject_GetAttrString(kbeModule, "component");
	if (!pyComponentName)
	{
		PyErr_Format(PyExc_AssertionError, "EntityDef.__py_def_call(): get KBEngine.component error!\n");
		PY_RETURN_ERROR;
	}

	defContext.componentType = ComponentName2ComponentType(PyUnicode_AsUTF8AndSize(pyComponentName, NULL));
	Py_DECREF(pyComponentName);

	if (!args || PyTuple_Size(args) < 1)
	{
		PyErr_Format(PyExc_AssertionError, "EntityDef.__py_def_call(EntityDef.%s): error!\n", defContext.optionName.c_str());
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

	defContext.pyObjectPtr = PyObjectPtr(pyFunc);
	PYOBJECT_SOURCEFILE(defContext.pyObjectPtr.get(), defContext.pyObjectSourceFile);

	if (defContext.optionName == "method")
	{
		static char * keywords[] =
		{
			const_cast<char *> ("exposed"),
			const_cast<char *> ("utype"),
			NULL
		};

		PyObject* pyExposed = NULL;
		PyObject* pyUtype = NULL;

		if (!PyArg_ParseTupleAndKeywords(cc.pyArgs.get(), cc.pyKwargs.get(), "|OO",
			keywords, &pyExposed, &pyUtype))
		{
			PY_RETURN_ERROR;
		}

		if (pyExposed && !PyBool_Check(pyExposed))
		{
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'exposed\' error! not a bool type.\n", defContext.optionName.c_str());
			PY_RETURN_ERROR;
		}

		if (pyUtype && !PyLong_Check(pyUtype))
		{
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'utype\' error! not a number type.\n", defContext.optionName.c_str());
			PY_RETURN_ERROR;
		}

		defContext.exposed = pyExposed == Py_True;

		if (pyUtype)
			defContext.utype = (int)PyLong_AsLong(pyUtype);
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
				const_cast<char *> ("utype"),
				NULL
			};

			PyObject* pyFlags = NULL;
			PyObject* pyPersistent = NULL;
			PyObject* pyIndex = NULL;
			PyObject* pyDatabaseLength = NULL;
			PyObject* pyUtype = NULL;

			if (!PyArg_ParseTupleAndKeywords(cc.pyArgs.get(), cc.pyKwargs.get(), "|OOOOO",
				keywords, &pyFlags, &pyPersistent, &pyIndex, &pyDatabaseLength, &pyUtype))
			{
				PY_RETURN_ERROR;
			}

			if (!pyFlags || !isRefEntityDefModule(pyFlags))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'flags\' must be referenced from the [EntityDef.ALL_CLIENTS, EntityDef.*] module!\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			if (!isRefEntityDefModule(pyIndex))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'index\' must be referenced from the [EntityDef.UNIQUE, EntityDef.INDEX] module!\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			if (pyDatabaseLength && !PyLong_Check(pyDatabaseLength))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'databaseLength\' error! not a number type.\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			if (pyPersistent && !PyBool_Check(pyPersistent))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'persistent\' error! not a bool type.\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			if(pyUtype && !PyLong_Check(pyUtype))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'utype\' error! not a number type.\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			defContext.propertyFlags = PyUnicode_AsUTF8AndSize(pyFlags, NULL);

			if (pyPersistent)
				defContext.persistent = pyPersistent == Py_True;

			if (pyIndex)
				defContext.propertyIndex = PyUnicode_AsUTF8AndSize(pyIndex, NULL);

			if (pyDatabaseLength)
				defContext.databaseLength = (int)PyLong_AsLong(pyDatabaseLength);

			if (pyUtype)
				defContext.utype = (int)PyLong_AsLong(pyUtype);
		}

		// 对于属性， 我们需要获得返回值作为默认值
		PyObject* pyRet = PyObject_CallFunction(pyFunc,
			const_cast<char*>("(O)"), Py_None);

		if (!pyRet)
			return NULL;

		if (pyRet != Py_None)
		{
			PyObject* pyStrResult = PyObject_Str(pyRet);
			Py_DECREF(pyRet);

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
		else
		{
			Py_DECREF(pyRet);
		}
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
				defContext.implementedBy = pImplementedBy;
			}

			PyObject* pyQualname = PyObject_GetAttrString(defContext.implementedBy.get(), "__qualname__");
			if (!pyQualname)
			{
				PY_RETURN_ERROR;
			}

			defContext.implementedByModuleName = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
			Py_DECREF(pyQualname);

			PYOBJECT_SOURCEFILE(defContext.implementedBy.get(), defContext.implementedByModuleFile);
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
		PyErr_Format(PyExc_AssertionError, "EntityDef.%s: not support!\n", defContext.optionName.c_str());
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
					PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'%s\' must be defined in the entity module!\n", 
						defContext.optionName.c_str(), moduleQualname);
				else
					PyErr_Format(PyExc_AssertionError, "EntityDef.%s: please check the command format is: EntityDef.%s(..)\n", 
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
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: error! such as: @EntityDef.rename()\n\tdef ENTITY_ID() -> EntityDef.INT32: pass\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			defContext.moduleName = outs[0];
		}

		PyObject* pyInspectModule =
			PyImport_ImportModule(const_cast<char*>("inspect"));

		PyObject* pyGetfullargspec = NULL;
		if (pyInspectModule)
		{
			pyGetfullargspec =
				PyObject_GetAttrString(pyInspectModule, const_cast<char *>("getfullargspec"));

			Py_DECREF(pyInspectModule);
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
						PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'%s\' did not find \'self\' parameter!\n", defContext.optionName.c_str(), moduleQualname);
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

					// 如果是EntityDef.Array则此处可能是一个tuple，参考__py_array
					if (PyTuple_Check(value) && PyTuple_Size(value) == 2)
					{
						PyObject *entitydefModule = PyImport_AddModule(pyDefModuleName.c_str());
						PyObject* pyARRAY = PyObject_GetAttrString(entitydefModule, "ARRAY");
						PyObject* item0 = PyTuple_GET_ITEM(value, 0);

						if (pyARRAY == item0)
						{
							value = PyTuple_GET_ITEM(value, 1);
							defContext.optionName = "anonymous_fixed_array";

							if (std::string(skey) != "return")
							{
								PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'EntityDef.ARRAY\' Can only be used to define property types!\n", defContext.optionName.c_str());
								PY_RETURN_ERROR;
							}
						}
						else
						{
							Py_DECREF(pyARRAY);
							PY_RETURN_ERROR;
						}

						Py_DECREF(pyARRAY);
					}

					if (PyUnicode_Check(value))
					{
						svalue = PyUnicode_AsUTF8AndSize(value, NULL);
					}
					else
					{
						PyObject* pyQualname = PyObject_GetAttrString(value, "__qualname__");
						if (!pyQualname)
						{
							PY_RETURN_ERROR;
						}

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
		else
		{
			PY_RETURN_ERROR;
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
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'%s\' does not inherit the KBEngine.Entity class!\n", defContext.optionName.c_str(), moduleQualname);
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
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'%s\' all parameters must have annotations!\n", defContext.optionName.c_str(), moduleQualname);
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
	else if (defContext.optionName == "anonymous_fixed_array")
	{
		// 创建一个新数组
		DefContext arrayType;
		arrayType = defContext;
		arrayType.moduleName += ".anonymous_fixed_array";

		// 防止2个源文件行数一致，底层认为重复添加
		arrayType.pyObjectSourceFile += "(array)";

		// 增加一个item类别
		DefContext itemType;
		itemType = arrayType;
		itemType.optionName = "fixed_item";

		// 防止2个源文件行数一致，底层认为重复添加
		itemType.pyObjectSourceFile += "(array_item)";

		noerror = onDefFixedItem(itemType);

		arrayType.returnType = "";

		if(noerror)
			noerror = onDefFixedArray(arrayType);

		defContext.returnType = arrayType.moduleName;

		if (noerror)
			noerror = onDefFixedItem(defContext);
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

	// 类似这种定义方式 EntityDef.rename(ENTITY_ID=EntityDef.INT32)
	if (kwargs)
	{
		PyObject *key, *value;
		Py_ssize_t pos = 0;

		while (PyDict_Next(kwargs, &pos, &key, &value)) {
			if (!PyType_Check(value))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg2 not legal type! such as: EntityDef.rename(ENTITY_ID=EntityDef.INT32)\n", cc.optionName.c_str());
				return NULL;
			}

			PyObject* pyQualname = PyObject_GetAttrString(value, "__qualname__");
			if (!pyQualname)
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg2 get __qualname__ error! such as: EntityDef.rename(ENTITY_ID=EntityDef.INT32)\n", cc.optionName.c_str());
				return NULL;
			}

			std::string typeName = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
			Py_DECREF(pyQualname);

			if (!PyUnicode_Check(key))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg1 must be a string! such as: EntityDef.rename(ENTITY_ID=EntityDef.INT32)\n", cc.optionName.c_str());
				return NULL;
			}

			DefContext defContext;
			defContext.optionName = cc.optionName;
			defContext.moduleName = PyUnicode_AsUTF8AndSize(key, NULL);
			defContext.returnType = typeName;

			PyObject* kbeModule = PyImport_AddModule("KBEngine");
			KBE_ASSERT(kbeModule);

			PyObject* pyComponentName = PyObject_GetAttrString(kbeModule, "component");
			if (!pyComponentName)
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.rename(): get KBEngine.component error!\n");
				PY_RETURN_ERROR;
			}

			defContext.componentType = ComponentName2ComponentType(PyUnicode_AsUTF8AndSize(pyComponentName, NULL));
			Py_DECREF(pyComponentName);

			if (!onDefRename(defContext))
				return NULL;
		}

		S_Return;
	}

	g_callContexts.push(cc);

	// @EntityDef.rename()
	// def ENTITY_ID() -> int: pass
	return PyCFunction_New(&__call_def_parse, self);
}

static PyObject* __py_def_fixed_array(PyObject* self, PyObject* args, PyObject* kwargs)
{
	CallContext cc;
	cc.pyArgs = PyObjectPtr(Copy::deepcopy(args));
	cc.pyKwargs = kwargs ? PyObjectPtr(Copy::deepcopy(kwargs)) : PyObjectPtr(NULL);
	cc.optionName = "fixed_array";

	Py_XDECREF(cc.pyArgs.get());
	Py_XDECREF(cc.pyKwargs.get());

	// 类似这种定义方式 EntityDef.fixed_array(XXArray=EntityDef.INT32)
	if (kwargs)
	{
		PyObject *key, *value;
		Py_ssize_t pos = 0;

		while (PyDict_Next(kwargs, &pos, &key, &value)) {
			if (!PyType_Check(value))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg2 not legal type! such as: EntityDef.fixed_array(XXArray=EntityDef.INT32)\n", cc.optionName.c_str());
				return NULL;
			}

			PyObject* pyQualname = PyObject_GetAttrString(value, "__qualname__");
			if (!pyQualname)
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg2 get __qualname__ error! such as: EntityDef.fixed_array(XXArray=EntityDef.INT32)\n", cc.optionName.c_str());
				return NULL;
			}

			std::string typeName = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
			Py_DECREF(pyQualname);

			if (!PyUnicode_Check(key))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg1 must be a string! such as: EntityDef.fixed_array(XXArray=EntityDef.INT32)\n", cc.optionName.c_str());
				return NULL;
			}

			DefContext defContext;
			defContext.optionName = cc.optionName;
			defContext.moduleName = PyUnicode_AsUTF8AndSize(key, NULL);
			defContext.returnType = typeName;

			PyObject* kbeModule = PyImport_AddModule("KBEngine");
			KBE_ASSERT(kbeModule);

			PyObject* pyComponentName = PyObject_GetAttrString(kbeModule, "component");
			if (!pyComponentName)
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.fixed_array(): get KBEngine.component error!\n");
				PY_RETURN_ERROR;
			}

			defContext.componentType = ComponentName2ComponentType(PyUnicode_AsUTF8AndSize(pyComponentName, NULL));
			Py_DECREF(pyComponentName);

			if (!onDefRename(defContext))
				return NULL;
		}

		S_Return;
	}

	g_callContexts.push(cc);

	// @EntityDef.fixed_array()
	// class XXXArray：
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
	DefContext::allScriptDefContextMaps.clear();
	DefContext::allScriptDefContextLineMaps.clear();
	return true; 
}

//-------------------------------------------------------------------------------------
static bool loadAllScriptForComponentType(COMPONENT_TYPE loadComponentType)
{
	std::string rootPath = Resmgr::getSingleton().getPyUserComponentScriptsPath(loadComponentType);

	if (rootPath.size() == 0)
	{
		ERROR_MSG(fmt::format("PyEntityDef::loadAllScriptForComponentType(): get scripts path error! loadComponentType={}\n", 
			COMPONENT_NAME_EX(loadComponentType)));

		return false;
	}

	while (rootPath[rootPath.size() - 1] == '/' || rootPath[rootPath.size() - 1] == '\\') rootPath.pop_back();

	wchar_t* wpath = strutil::char2wchar((rootPath).c_str());
	std::vector<std::wstring> results;
	Resmgr::getSingleton().listPathRes(wpath, L"py|pyc", results);

	std::vector<std::wstring>::iterator iter = results.begin();
	for (; iter != results.end(); ++iter)
	{
		std::wstring wstrpath = (*iter);

		if (wstrpath.find(L"__pycache__") != std::wstring::npos)
			continue;

		if (wstrpath.find(L"__init__.") != std::wstring::npos)
			continue;

		std::pair<std::wstring, std::wstring> pathPair = script::PyPlatform::splitPath(wstrpath);
		std::pair<std::wstring, std::wstring> filePair = script::PyPlatform::splitText(pathPair.second);

		if (filePair.first.size() == 0)
			continue;

		char* cpacketPath = strutil::wchar2char(pathPair.first.c_str());
		std::string packetPath = cpacketPath;
		free(cpacketPath);

		strutil::kbe_replace(packetPath, rootPath, "");
		while (packetPath.size() > 0 && (packetPath[0] == '/' || packetPath[0] == '\\')) packetPath.erase(0, 1);
		strutil::kbe_replace(packetPath, "/", ".");
		strutil::kbe_replace(packetPath, "\\", ".");

		char* moduleName = strutil::wchar2char(filePair.first.c_str());

		// 由于脚本内部可能会import造成重复import， 我们过滤已经import过的模块
		if (DefContext::allScriptDefContextMaps.find(moduleName) == DefContext::allScriptDefContextMaps.end())
		{
			PyObject* pyModule = NULL;

			if (packetPath.size() == 0 || packetPath == "components" || packetPath == "interfaces")
			{
				pyModule = PyImport_ImportModule(const_cast<char*>(moduleName));
			}
			else
			{
				pyModule = PyImport_ImportModule(const_cast<char*>((packetPath + "." + moduleName).c_str()));
			}

			if (!pyModule)
			{
				SCRIPT_ERROR_CHECK();
				return false;
			}
			else
			{
				Py_DECREF(pyModule);
			}
		}

		free(moduleName);
	}

	free(wpath);
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* __py_getAppPublish(PyObject* self, PyObject* args)
{
	return PyLong_FromLong(g_appPublish);
}

//-------------------------------------------------------------------------------------
static bool execPython(COMPONENT_TYPE componentType)
{
	std::pair<std::wstring, std::wstring> pyPaths = getComponentPythonPaths(componentType);
	if (pyPaths.first.size() == 0)
	{
		ERROR_MSG(fmt::format("PyEntityDef::execPython(): PythonApp({}) paths error!\n", COMPONENT_NAME_EX(componentType)));
		return false;
	}

	APPEND_PYSYSPATH(pyPaths.second);

	PyObject* modulesOld = PySys_GetObject("modules");

	PyThreadState* pCurInterpreter = PyThreadState_Get();
	PyThreadState* pNewInterpreter = Py_NewInterpreter();

	if (!pNewInterpreter)
	{
		ERROR_MSG(fmt::format("PyEntityDef::execPython(): Py_NewInterpreter()!\n"));
		SCRIPT_ERROR_CHECK();
		return false;
	}

#if KBE_PLATFORM != PLATFORM_WIN32
	strutil::kbe_replace(pyPaths.second, L";", L":");
#endif

	PySys_SetPath(pyPaths.second.c_str());

	PyObject* modulesNew = PySys_GetObject("modules");
	PyDict_Merge(modulesNew, Script::getSingleton().getSysInitModules(), 0);

	{
		PyObject *key, *value;
		Py_ssize_t pos = 0;

		while (PyDict_Next(modulesOld, &pos, &key, &value))
		{
			const char* typeName = PyUnicode_AsUTF8AndSize(key, NULL);

			if (std::string(typeName) == "KBEngine")
				continue;

			PyObject* pyDoc = PyObject_GetAttrString(value, "__doc__");

			if (pyDoc)
			{
				const char* doc = PyUnicode_AsUTF8AndSize(pyDoc, NULL);

				if (doc && std::string(doc).find("KBEngine") != std::string::npos)
					PyDict_SetItemString(modulesNew, typeName, value);

				if (PyErr_Occurred())
					PyErr_Clear();

				Py_XDECREF(pyDoc);
			}
			else
			{
				SCRIPT_ERROR_CHECK();
			}
		}
	}

	PyObject *m = PyImport_AddModule("__main__");

	// 添加一个脚本基础模块
	PyObject* kbeModule = PyImport_AddModule("KBEngine");
	KBE_ASSERT(kbeModule);

	Entity::registerScript(kbeModule);
	Space::registerScript(kbeModule);
	EntityComponent::registerScript(kbeModule);

	if (componentType == BASEAPP_TYPE)
		Proxy::registerScript(kbeModule);

	const char* componentName = COMPONENT_NAME_EX(componentType);
	if (PyModule_AddStringConstant(kbeModule, "component", componentName))
	{
		ERROR_MSG(fmt::format("PyEntityDef::execPython(): Unable to set KBEngine.component to {}\n",
			componentName));

		return false;
	}

	APPEND_SCRIPT_MODULE_METHOD(kbeModule, publish, __py_getAppPublish, METH_VARARGS, 0);

	// 将模块对象加入main
	PyObject_SetAttrString(m, "KBEngine", kbeModule);

	if (pNewInterpreter != PyThreadState_Swap(pCurInterpreter))
	{
		KBE_ASSERT(false);
		return false;
	}

	PyThreadState_Swap(pNewInterpreter);

	bool otherPartSuccess = loadAllScriptForComponentType(componentType);

	Entity::unregisterScript();
	Space::unregisterScript();
	EntityComponent::unregisterScript();

	if (componentType == BASEAPP_TYPE)
		Proxy::unregisterScript();

	if (pNewInterpreter != PyThreadState_Swap(pCurInterpreter))
	{
		KBE_ASSERT(false);
		return false;
	}

	// 此处不能使用Py_EndInterpreter，会导致Math、Def等模块析构
	PyInterpreterState_Clear(pNewInterpreter->interp);
	PyInterpreterState_Delete(pNewInterpreter->interp);
	return otherPartSuccess;
}

//-------------------------------------------------------------------------------------
static bool loadAllScripts()
{
	std::vector< COMPONENT_TYPE > loadOtherComponentTypes;

	if (g_componentType == CELLAPP_TYPE || g_componentType == BASEAPP_TYPE)
	{
		bool otherPartSuccess = loadAllScriptForComponentType(g_componentType);
		if (!otherPartSuccess)
			return false;

		loadOtherComponentTypes.push_back((g_componentType == BASEAPP_TYPE) ? CELLAPP_TYPE : BASEAPP_TYPE);
	}
	else
	{
		loadOtherComponentTypes.push_back(BASEAPP_TYPE);
		loadOtherComponentTypes.push_back(CELLAPP_TYPE);
	}

	for (std::vector< COMPONENT_TYPE >::iterator iter = loadOtherComponentTypes.begin(); iter != loadOtherComponentTypes.end(); ++iter)
	{
		COMPONENT_TYPE componentType = (*iter);
		
		if (!execPython(componentType))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
static bool registerDefTypes()
{
	DefContext::DEF_CONTEXT_MAP::iterator iter = DefContext::allScriptDefContextMaps.begin();
	for (; iter != DefContext::allScriptDefContextMaps.end(); ++iter)
	{
		DefContext& defContext = iter->second;

		if (defContext.type == DefContext::DC_TYPE_FIXED_ARRAY)
		{
			FixedArrayType* fixedArray = new FixedArrayType;

			if (fixedArray->initialize(&defContext, defContext.moduleName))
			{
				if (!DataTypes::addDataType(defContext.moduleName, fixedArray))
					return false;
			}
			else
			{
				ERROR_MSG(fmt::format("PyEntityDef::registerDefTypes: parse ARRAY [{}] error! file: \"{}\"!\n",
					defContext.moduleName.c_str(), defContext.pyObjectSourceFile));

				delete fixedArray;
				return false;
			}
		}
		else if (defContext.type == DefContext::DC_TYPE_FIXED_DICT)
		{
			FixedDictType* fixedDict = new FixedDictType;

			if (fixedDict->initialize(&defContext, defContext.moduleName))
			{
				if (!DataTypes::addDataType(defContext.moduleName, fixedDict))
					return false;
			}
			else
			{
				ERROR_MSG(fmt::format("PyEntityDef::registerDefTypes: parse FIXED_DICT [{}] error! file: \"{}\"!\n",
					defContext.moduleName.c_str(), defContext.pyObjectSourceFile));

				delete fixedDict;
				return false;
			}
		}
		else if (defContext.type == DefContext::DC_TYPE_RENAME)
		{
			DataType* dataType = DataTypes::getDataType(defContext.returnType, false);
			if (dataType == NULL)
			{
				ERROR_MSG(fmt::format("PyEntityDef::registerDefTypes: cannot fount type \'{}\', by alias[{}], file: \"{}\"!\n",
					defContext.returnType, defContext.moduleName.c_str(), defContext.pyObjectSourceFile));

				return false;
			}

			if (!DataTypes::addDataType(defContext.moduleName, dataType))
			{
				ERROR_MSG(fmt::format("PyEntityDef::registerDefTypes: addDataType \"{}\" error! file: \"{}\"!\n",
					defContext.moduleName.c_str(), defContext.pyObjectSourceFile));

				return false;
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
static bool registerDetailLevelInfo(ScriptDefModule* pScriptModule, DefContext& defContext)
{
	return true;
}

//-------------------------------------------------------------------------------------
static bool registerVolatileInfo(ScriptDefModule* pScriptModule, DefContext& defContext)
{
	return true;
}

//-------------------------------------------------------------------------------------
static bool registerDefPropertys(ScriptDefModule* pScriptModule, DefContext& defContext)
{
	DefContext::DEF_CONTEXTS& propertys = defContext.propertys;

	DefContext::DEF_CONTEXTS::iterator iter = propertys.begin();
	for (; iter != propertys.end(); ++iter)
	{
		DefContext& defPropContext = (*iter);

		ENTITY_PROPERTY_UID			futype = 0;
		EntityDataFlags				flags = stringToEntityDataFlags(defPropContext.propertyFlags);
		int32						hasBaseFlags = 0;
		int32						hasCellFlags = 0;
		int32						hasClientFlags = 0;
		DataType*					dataType = NULL;
		bool						isPersistent = (defPropContext.persistent != 0);
		bool						isIdentifier = false;								// 是否是一个索引键
		uint32						databaseLength = defPropContext.databaseLength;			// 这个属性在数据库中的长度
		std::string					indexType = defPropContext.propertyIndex;
		DETAIL_TYPE					detailLevel = DETAIL_LEVEL_FAR;
		std::string					name = defPropContext.attrName;
		
		if (!EntityDef::validDefPropertyName(name))
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefPropertys: '{}' is limited, in module({}), file: \"{}\"!\n",
				name, pScriptModule->getName(), defPropContext.pyObjectSourceFile));

			return false;
		}

		if (defPropContext.detailLevel == "FAR")
			detailLevel = DETAIL_LEVEL_FAR;
		else if (defPropContext.detailLevel == "MEDIUM")
			detailLevel = DETAIL_LEVEL_MEDIUM;
		else if (defPropContext.detailLevel == "NEAR")
			detailLevel = DETAIL_LEVEL_NEAR;
		else
			detailLevel = DETAIL_LEVEL_FAR;

		if (!EntityDef::calcDefPropertyUType(pScriptModule->getName(), name, defPropContext.utype > 0 ? defPropContext.utype : -1, pScriptModule, futype))
			return false;

		hasBaseFlags = ((uint32)flags) & ENTITY_BASE_DATA_FLAGS;
		if (hasBaseFlags > 0)
			pScriptModule->setBase(true);

		hasCellFlags = ((uint32)flags) & ENTITY_CELL_DATA_FLAGS;
		if (hasCellFlags > 0)
			pScriptModule->setCell(true);

		hasClientFlags = ((uint32)flags) & ENTITY_CLIENT_DATA_FLAGS;
		if (hasClientFlags > 0)
			pScriptModule->setClient(true);

		if (hasBaseFlags <= 0 && hasCellFlags <= 0)
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefPropertys: not fount flags[{}], is {}.{}, file: \"{}\"!\n",
				defPropContext.propertyFlags, pScriptModule->getName(), defPropContext.pyObjectSourceFile));

			return false;
		}

		dataType = DataTypes::getDataType(defPropContext.returnType, false);

		if (!dataType)
		{
			DefContext* pDefPropTypeContext = DefContext::findDefContext(defPropContext.returnType);
			if (!pDefPropTypeContext)
			{
				ERROR_MSG(fmt::format("PyEntityDef::registerDefPropertys: not fount type[{}], is {}.{}, file: \"{}\"!\n",
					defPropContext.returnType, pScriptModule->getName(), name.c_str(), defPropContext.pyObjectSourceFile));

				return false;
			}

			// 组件放到组件函数中处理
			if (pDefPropTypeContext->type == DefContext::DC_TYPE_COMPONENT)
				continue;

			if (pDefPropTypeContext->type == DefContext::DC_TYPE_FIXED_ARRAY)
			{
				FixedArrayType* dataType1 = new FixedArrayType();
				if (dataType1->initialize(pDefPropTypeContext, std::string(pScriptModule->getName()) + "_" + name))
					dataType = dataType1;
				else
					return false;
			}
			else
			{
				dataType = DataTypes::getDataType(defPropContext.returnType, false);
			}
		}

		if (dataType == NULL)
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefPropertys: not fount type[{}], is {}.{}, file: \"{}\"!\n",
				defPropContext.returnType, pScriptModule->getName(), name.c_str(), defPropContext.pyObjectSourceFile));

			return false;
		}

		// 产生一个属性描述实例
		PropertyDescription* propertyDescription = PropertyDescription::createDescription(futype, defPropContext.returnType,
			name, flags, isPersistent,
			dataType, isIdentifier, indexType,
			databaseLength, defPropContext.propertyDefaultVal,
			detailLevel);

		bool ret = true;

		// 添加到模块中
		if (hasCellFlags > 0)
			ret = pScriptModule->addPropertyDescription(name.c_str(),
				propertyDescription, CELLAPP_TYPE);

		if (hasBaseFlags > 0)
			ret = pScriptModule->addPropertyDescription(name.c_str(),
				propertyDescription, BASEAPP_TYPE);

		if (hasClientFlags > 0)
			ret = pScriptModule->addPropertyDescription(name.c_str(),
				propertyDescription, CLIENT_TYPE);

		if (!ret)
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefPropertys: error, is {}.{}, file: \"{}\"!\n",
				pScriptModule->getName(), name.c_str(), defPropContext.pyObjectSourceFile));

			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
static bool registerDefComponents(ScriptDefModule* pScriptModule, DefContext& defContext)
{
	DefContext::DEF_CONTEXTS& propertys = defContext.propertys;

	DefContext::DEF_CONTEXTS::iterator iter = propertys.begin();
	for (; iter != propertys.end(); ++iter)
	{
		DefContext& defPropContext = (*iter);

		std::string					componentName = defPropContext.attrName;
		std::string					componentTypeName = defPropContext.returnType;
		bool						isPersistent = (defPropContext.persistent != 0);
		ENTITY_PROPERTY_UID			futype = 0;

		DefContext* pDefPropTypeContext = DefContext::findDefContext(componentTypeName);
		if (!pDefPropTypeContext)
		{
			continue;
		}

		if (pDefPropTypeContext->type != DefContext::DC_TYPE_COMPONENT)
			continue;

		if (!EntityDef::calcDefPropertyUType(pScriptModule->getName(), componentName, defPropContext.utype > 0 ? defPropContext.utype : -1, pScriptModule, futype))
			return false;

		// 产生一个属性描述实例
		uint32						flags = ED_FLAG_BASE | ED_FLAG_CELL_PUBLIC | ENTITY_CLIENT_DATA_FLAGS;
		bool						isIdentifier = false;		// 是否是一个索引键
		uint32						databaseLength = 0;			// 这个属性在数据库中的长度
		std::string					indexType = "";
		DETAIL_TYPE					detailLevel = DETAIL_LEVEL_FAR;
		std::string					detailLevelStr = "";
		std::string					strisPersistent;
		std::string					defaultStr = "";

		if (!EntityDef::validDefPropertyName(componentName))
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefComponents: '{}' is limited, in module({}), file: \"{}\"!\n",
				componentName, pScriptModule->getName(), defPropContext.pyObjectSourceFile));

			return false;
		}

		// 查找是否有这个模块，如果有说明已经加载过相关描述，这里无需再次加载
		ScriptDefModule* pCompScriptDefModule = EntityDef::findScriptModule(componentTypeName.c_str(), false);

		if (!pCompScriptDefModule)
		{
			pCompScriptDefModule = EntityDef::registerNewScriptDefModule(componentTypeName);
			pCompScriptDefModule->isPersistent(false);
			pCompScriptDefModule->isComponentModule(true);
		}
		else
		{
			flags = ED_FLAG_UNKOWN;

			if (pCompScriptDefModule->hasBase())
				flags |= ED_FLAG_BASE;

			if (pCompScriptDefModule->hasCell())
				flags |= ED_FLAG_CELL_PUBLIC;

			if (pCompScriptDefModule->hasClient())
			{
				if (pCompScriptDefModule->hasBase())
					flags |= ED_FLAG_BASE_AND_CLIENT;
				else
					flags |= (ED_FLAG_ALL_CLIENTS | ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OTHER_CLIENTS | ED_FLAG_OWN_CLIENT);
			}

			EntityDef::addComponentProperty(futype, componentTypeName, componentName, flags, isPersistent, isIdentifier,
				indexType, databaseLength, defaultStr, detailLevel, pScriptModule, pCompScriptDefModule);

			pScriptModule->addComponentDescription(componentName.c_str(), pCompScriptDefModule);
			continue;
		}

		// 加载属性描述
		if (!registerDefPropertys(pScriptModule, defContext))
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefComponents: failed to loadDefPropertys(), entity:{}\n",
				pScriptModule->getName()));

			return false;
		}

		// 尝试加载detailLevelInfo数据
		if (!registerDetailLevelInfo(pScriptModule, defContext))
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefComponents: failed to register component:{} detailLevelInfo.\n",
				pScriptModule->getName()));

			return false;
		}

		// 尝试加载volatileInfo数据
		if (!registerVolatileInfo(pScriptModule, defContext))
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefComponents: failed to register component:{} volatileInfo.\n",
				pScriptModule->getName()));

			return false;
		}

		pCompScriptDefModule->autoMatchCompOwn();

		flags = ED_FLAG_UNKOWN;

		if (pCompScriptDefModule->hasBase())
			flags |= ED_FLAG_BASE;

		if (pCompScriptDefModule->hasCell())
			flags |= ED_FLAG_CELL_PUBLIC;

		if (pCompScriptDefModule->hasClient())
		{
			if (pCompScriptDefModule->hasBase())
				flags |= ED_FLAG_BASE_AND_CLIENT;

			if (pCompScriptDefModule->hasCell())
				flags |= (ED_FLAG_ALL_CLIENTS | ED_FLAG_CELL_PUBLIC_AND_OWN | ED_FLAG_OTHER_CLIENTS | ED_FLAG_OWN_CLIENT);
		}

		EntityDef::addComponentProperty(futype, componentTypeName, componentName, flags, isPersistent, isIdentifier,
			indexType, databaseLength, defaultStr, detailLevel, pScriptModule, pCompScriptDefModule);

		pScriptModule->addComponentDescription(componentName.c_str(), pCompScriptDefModule);
	}

	return true;
}

//-------------------------------------------------------------------------------------
static bool registerEntityDef(ScriptDefModule* pScriptModule, DefContext& defContext)
{
	// 加载属性描述
	if (!registerDefPropertys(pScriptModule, defContext))
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerEntityDef: failed to registerDefPropertys(), entity:{}\n",
			pScriptModule->getName()));

		return false;
	}

	// 加载组件描述， 并将他们的方法和属性加入到模块中
	if (!registerDefComponents(pScriptModule, defContext))
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerEntityDef: failed to registerDefComponents(), component:{}\n",
			pScriptModule->getName()));

		return false;
	}

	// 尝试加载detailLevelInfo数据
	if (!registerDetailLevelInfo(pScriptModule, defContext))
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerEntityDef: failed to register entity:{} detailLevelInfo.\n",
			pScriptModule->getName()));

		return false;
	}

	// 尝试加载volatileInfo数据
	if (!registerVolatileInfo(pScriptModule, defContext))
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerEntityDef: failed to register entity:{} volatileInfo.\n",
			pScriptModule->getName()));

		return false;
	}

	pScriptModule->autoMatchCompOwn();
	return true;
}

//-------------------------------------------------------------------------------------
static bool registerEntityDefs()
{
	if (!registerDefTypes())
		return false;

	DefContext::DEF_CONTEXT_MAP::iterator iter = DefContext::allScriptDefContextMaps.begin();
	for (; iter != DefContext::allScriptDefContextMaps.end(); ++iter)
	{
		DefContext& defContext = iter->second;

		if (defContext.type != DefContext::DC_TYPE_ENTITY)
			continue;

		ScriptDefModule* pScriptModule = EntityDef::registerNewScriptDefModule(defContext.moduleName);

		if (!registerEntityDef(pScriptModule, defContext))
			return false;

		pScriptModule->onLoaded();
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool initialize()
{
	if (g_inited)
		return false;

	g_inited = true;

	PyObject *entitydefModule = PyImport_AddModule(pyDefModuleName.c_str());

	ENTITYFLAGMAP::iterator iter = g_entityFlagMapping.begin();
	for (; iter != g_entityFlagMapping.end(); ++iter)
	{
		if (PyModule_AddStringConstant(entitydefModule, iter->first.c_str(), iter->first.c_str()))
		{
			ERROR_MSG(fmt::format("PyEntityDef::initialize(): Unable to set EntityDef.{} to {}\n",
				iter->first, iter->first));

			return false;
		}
	}

	static const char* UNIQUE = "UNIQUE";
	if (PyModule_AddStringConstant(entitydefModule, UNIQUE, UNIQUE))
	{
		ERROR_MSG(fmt::format("PyEntityDef::initialize(): Unable to set EntityDef.{} to {}\n",
			UNIQUE, UNIQUE));

		return false;
	}

	static const char* INDEX = "INDEX";
	if (PyModule_AddStringConstant(entitydefModule, INDEX, INDEX))
	{
		ERROR_MSG(fmt::format("PyEntityDef::initialize(): Unable to set EntityDef.{} to {}\n",
			INDEX, INDEX));

		return false;
	}

	static const char* thisClass = "thisClass";
	if (PyModule_AddStringConstant(entitydefModule, thisClass, thisClass))
	{
		ERROR_MSG(fmt::format("PyEntityDef::initialize(): Unable to set EntityDef.{} to {}\n",
			thisClass, thisClass));

		return false;
	}
	
	APPEND_SCRIPT_MODULE_METHOD(entitydefModule, ARRAY, __py_array, METH_VARARGS, 0);

	static char allBaseTypeNames[64][MAX_BUF];
	std::vector< std::string > baseTypeNames = DataTypes::getBaseTypeNames();
	KBE_ASSERT(baseTypeNames.size() < 64);

	for (size_t idx = 0; idx < baseTypeNames.size(); idx++)
	{
		memset(allBaseTypeNames[idx], 0, MAX_BUF);
		kbe_snprintf(allBaseTypeNames[idx], MAX_BUF, "%s", baseTypeNames[idx].c_str());
		if (PyModule_AddStringConstant(entitydefModule, allBaseTypeNames[idx], allBaseTypeNames[idx]))
		{
			ERROR_MSG(fmt::format("PyEntityDef::initialize(): Unable to set EntityDef.{} to {}\n",
				allBaseTypeNames[idx], allBaseTypeNames[idx]));

			return false;
		}
	}

	if (!loadAllScripts())
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}

	while (!g_callContexts.empty())
		g_callContexts.pop();

	DefContext::allScriptDefContextLineMaps.clear();

	if (!assemblyContexts(true))
	{
		SCRIPT_ERROR_CHECK();
		DefContext::allScriptDefContextMaps.clear();
		return false;
	}

	if (!registerEntityDefs())
	{
		DefContext::allScriptDefContextMaps.clear();
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
	g_inited = false;
}

//-------------------------------------------------------------------------------------
bool initializeWatcher()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool addToStream(MemoryStream* pMemoryStream)
{
	int size = DefContext::allScriptDefContextMaps.size();
	(*pMemoryStream) << size;

	DefContext::DEF_CONTEXT_MAP::iterator iter = DefContext::allScriptDefContextMaps.begin();
	for (; iter != DefContext::allScriptDefContextMaps.end(); ++iter)
	{
		const std::string& name = iter->first;
		DefContext& defContext = iter->second;

		(*pMemoryStream) << name;

		defContext.addToStream(pMemoryStream);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool createFromStream(MemoryStream* pMemoryStream)
{
	int size = 0;
	(*pMemoryStream) >> size;

	for (int i = 0; i < size; ++i)
	{
		std::string name = "";
		(*pMemoryStream) >> name;

		DefContext defContext;
		defContext.createFromStream(pMemoryStream);
		DefContext::allScriptDefContextLineMaps[defContext.pyObjectSourceFile] = defContext;
	}

	return true;
}

//-------------------------------------------------------------------------------------

}
}
}
