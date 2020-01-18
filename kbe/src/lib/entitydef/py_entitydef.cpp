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
#include "server/serverconfig.h"
#include "server/components.h"

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
static int g_order = 1;

//-------------------------------------------------------------------------------------
/*
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
*/

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
	order = g_order++;

	optionName = "";

	moduleName = "";
	attrName = "";
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
	pyObjectSourceFileComponentType = UNKNOWN_COMPONENT_TYPE;

	inheritEngineModuleType = DC_TYPE_UNKNOWN;
	type = DC_TYPE_UNKNOWN;

	componentType = UNKNOWN_COMPONENT_TYPE;
}

//-------------------------------------------------------------------------------------
bool DefContext::addToStream(MemoryStream* pMemoryStream)
{
	(*pMemoryStream) << order;
	(*pMemoryStream) << optionName;
	(*pMemoryStream) << moduleName;
	(*pMemoryStream) << attrName;
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
	(*pMemoryStream) << pyObjectSourceFileComponentType;
	
	(*pMemoryStream) << (int)baseClasses.size();
	std::vector< std::string >::iterator baseClassesIter = baseClasses.begin();
	for (; baseClassesIter != baseClasses.end(); ++baseClassesIter)
		(*pMemoryStream) << (*baseClassesIter);

	(*pMemoryStream) << (int)inheritEngineModuleType;
	(*pMemoryStream) << (int)type;

	(*pMemoryStream) << (int)base_methods.size();
	std::vector< DefContext >::iterator base_methodsIter = base_methods.begin();
	for (; base_methodsIter != base_methods.end(); ++base_methodsIter)
		(*base_methodsIter).addToStream(pMemoryStream);

	(*pMemoryStream) << (int)cell_methods.size();
	std::vector< DefContext >::iterator cell_methodsIter = cell_methods.begin();
	for (; cell_methodsIter != cell_methods.end(); ++cell_methodsIter)
		(*cell_methodsIter).addToStream(pMemoryStream);

	(*pMemoryStream) << (int)client_methods.size();
	std::vector< DefContext >::iterator client_methodsIter = client_methods.begin();
	for (; client_methodsIter != client_methods.end(); ++client_methodsIter)
		(*client_methodsIter).addToStream(pMemoryStream);

	(*pMemoryStream) << (int)propertys.size();
	std::vector< DefContext >::iterator propertysIter = propertys.begin();
	for (; propertysIter != propertys.end(); ++propertysIter)
		(*propertysIter).addToStream(pMemoryStream);

	(*pMemoryStream) << (int)components.size();
	std::vector< DefContext >::iterator componentsIter = components.begin();
	for (; componentsIter != components.end(); ++componentsIter)
		(*componentsIter).addToStream(pMemoryStream);

	return true;
}

//-------------------------------------------------------------------------------------
bool DefContext::createFromStream(MemoryStream* pMemoryStream)
{
	(*pMemoryStream) >> order;
	(*pMemoryStream) >> optionName;
	(*pMemoryStream) >> moduleName;
	(*pMemoryStream) >> attrName;
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
	(*pMemoryStream) >> pyObjectSourceFileComponentType;
	
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

		base_methods.push_back(dc);
	}

	(*pMemoryStream) >> size;
	for (int i = 0; i < size; ++i)
	{
		DefContext dc;
		dc.createFromStream(pMemoryStream);

		cell_methods.push_back(dc);
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
		DefContext dc;
		dc.createFromStream(pMemoryStream);

		components.push_back(dc);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool DefContext::addChildContext(DefContext& defContext)
{
	std::vector< DefContext >* pContexts = NULL;

	if (defContext.type == DefContext::DC_TYPE_PROPERTY)
	{
		pContexts = &propertys;
	}
	else if (defContext.type == DefContext::DC_TYPE_COMPONENT_PROPERTY)
	{
		pContexts = &components;
	}
	else if (defContext.type == DefContext::DC_TYPE_METHOD)
	{
		if(defContext.componentType == BASEAPP_TYPE)
			pContexts = &base_methods;
		else
			pContexts = &cell_methods;
	}
	else if (defContext.type == DefContext::DC_TYPE_CLIENT_METHOD)
	{
		pContexts = &client_methods;
		defContext.componentType = CLIENT_TYPE;
	}
	else if (defContext.type == DefContext::DC_TYPE_FIXED_ITEM)
	{
		pContexts = &propertys;
	}
	else
	{
		KBE_ASSERT(false);
	}

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
			defContext.type == DefContext::DC_TYPE_COMPONENT_PROPERTY ||
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

				childContexts.insert(childContexts.end(), parentDefContext.base_methods.begin(), parentDefContext.base_methods.end());
				childContexts.insert(childContexts.end(), parentDefContext.cell_methods.begin(), parentDefContext.cell_methods.end());
				childContexts.insert(childContexts.end(), parentDefContext.client_methods.begin(), parentDefContext.client_methods.end());
				childContexts.insert(childContexts.end(), parentDefContext.propertys.begin(), parentDefContext.propertys.end());
				childContexts.insert(childContexts.end(), parentDefContext.components.begin(), parentDefContext.components.end());

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
		if(!EntityDef::validDefPropertyName(defContext.attrName))
		{
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: '%s.%s' is limited!\n\n",
				defContext.optionName.c_str(), name.c_str(), defContext.attrName.c_str());

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
	else if (defContext.type == DefContext::DC_TYPE_COMPONENT_PROPERTY)
	{
		if (!EntityDef::validDefPropertyName(defContext.attrName))
		{
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: '%s.%s' is limited!\n\n", 
				defContext.optionName.c_str(), name.c_str(), defContext.attrName.c_str());

			return false;
		}

		name += "." + defContext.attrName;
	}
	else if(defContext.type == DefContext::DC_TYPE_METHOD)
	{
		if (!EntityDef::validDefPropertyName(defContext.attrName))
		{
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: '%s.%s' is limited!\n\n",
				defContext.optionName.c_str(), name.c_str(), defContext.attrName.c_str());

			return false;
		}

		name += "." + defContext.attrName;
	}
	else if (defContext.type == DefContext::DC_TYPE_CLIENT_METHOD)
	{
		if (!EntityDef::validDefPropertyName(defContext.attrName))
		{
			PyErr_Format(PyExc_AssertionError, "EntityDef.%s: '%s.%s' is limited!\n\n",
				defContext.optionName.c_str(), name.c_str(), defContext.attrName.c_str());

			return false;
		}

		// 由于可能出现客户端方法的声明名称与服务器方法一致的情况，这里需要将客户端方法临时做个别名
		name += ".#client#." + defContext.attrName;
	}
	else if (defContext.type == DefContext::DC_TYPE_FIXED_ITEM)
	{
		// 字典里面的字段放开关键字限制，例如允许定义名称为id的字段
		//if (!EntityDef::validDefPropertyName(defContext.attrName))
		//{
		//	PyErr_Format(PyExc_AssertionError, "EntityDef.%s: '%s.%s' is limited!\n\n",
		//		defContext.optionName.c_str(), name.c_str(), defContext.attrName.c_str());
		//
		//	return false;
		//}

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

					KBE_ASSERT(defContext.base_methods.size() == 0 && 
						defContext.cell_methods.size() == 0 &&  
						defContext.client_methods.size() == 0 && 
						defContext.components.size() == 0 && 
						defContext.propertys.size() == 0);

					if (defContext.hasClient)
						iter->second.hasClient = true;
				}

				// 脚本对象强制设置为当前进程的对象
				if (g_componentType == defContext.componentType)
				{
					iter->second.pyObjectPtr = defContext.pyObjectPtr;
					iter->second.pyObjectSourceFile = defContext.pyObjectSourceFile;
					iter->second.pyObjectSourceFileComponentType = g_componentType;
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
	if (defContext.isModuleScope)
	{
		defContext.type = DefContext::DC_TYPE_COMPONENT;
	}
	else
	{
		defContext.type = DefContext::DC_TYPE_COMPONENT_PROPERTY;
	}

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
	defContext.pyObjectSourceFileComponentType = defContext.componentType;

	if (defContext.optionName == "method" || defContext.optionName == "clientmethod")
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
		{
			defContext.utype = (int)PyLong_AsLong(pyUtype);

			if (defContext.utype > 0)
			{
				ENTITY_METHOD_UID muid = defContext.utype;

				if (defContext.utype != int(muid))
				{
					PyErr_Format(PyExc_AssertionError, "EntityDef.%s: 'Utype' has overflowed({} > 65535), is {}.{}!\n", defContext.optionName.c_str(), defContext.utype);
					PY_RETURN_ERROR;
				}
			}
		}
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
			{
				defContext.utype = (int)PyLong_AsLong(pyUtype);

				if (defContext.utype > 0)
				{
					ENTITY_PROPERTY_UID muid = defContext.utype;

					if (defContext.utype != int(muid))
					{
						PyErr_Format(PyExc_AssertionError, "EntityDef.%s: 'Utype' has overflowed({} > 65535), is {}.{}!\n", defContext.optionName.c_str(), defContext.utype);
						PY_RETURN_ERROR;
					}
				}
			}
		}
		else
		{
			static char * keywords[] =
			{
				const_cast<char *> ("persistent"),
				const_cast<char *> ("databaseLength"),
				NULL
			};

			PyObject* pyPersistent = NULL;
			PyObject* pyDatabaseLength = NULL;

			if (!PyArg_ParseTupleAndKeywords(cc.pyArgs.get(), cc.pyKwargs.get(), "|OO",
				keywords, &pyPersistent, &pyDatabaseLength))
			{
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

			// 如果是数据类型定义，默认应该是存储的
			// fixed_item只有定义fixed_dict时会用到
			defContext.persistent = true;

			if (pyPersistent)
				defContext.persistent = pyPersistent == Py_True;

			if (pyDatabaseLength)
				defContext.databaseLength = (int)PyLong_AsLong(pyDatabaseLength);
		}

		// 对于属性， 我们需要获得返回值作为默认值
		PyObject* pyRet = PyObject_CallFunction(pyFunc,
			const_cast<char*>("(O)"), Py_None);

		if (!pyRet)
		{
			PY_RETURN_ERROR;
		}

		if (pyRet != Py_None)
		{
			PyObject* pyStrResult = PyObject_Str(pyRet);
			Py_DECREF(pyRet);

			defContext.propertyDefaultVal = PyUnicode_AsUTF8AndSize(pyStrResult, NULL);
			Py_DECREF(pyStrResult);
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
		if (PyFunction_Check(pyFunc))
		{
			defContext.isModuleScope = false;

			static char * keywords[] =
			{
				const_cast<char *> ("persistent"),
				NULL
			};

			PyObject* pyPersistent = NULL;

			if (!PyArg_ParseTupleAndKeywords(cc.pyArgs.get(), cc.pyKwargs.get(), "|O",
				keywords, &pyPersistent))
			{
				PY_RETURN_ERROR;
			}

			if (pyPersistent && !PyBool_Check(pyPersistent))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'persistent\' error! not a bool type.\n", defContext.optionName.c_str());
				PY_RETURN_ERROR;
			}

			// 如果是组件属性，默认应该是不存储的
			defContext.persistent = false;

			if (pyPersistent)
				defContext.persistent = pyPersistent == Py_True;
		}
		else
		{
			defContext.isModuleScope = true;

			PyObject* pyHasClient = NULL;

			static char * keywords[] =
			{
				const_cast<char *> ("hasClient"),
				NULL
			};

			if (!PyArg_ParseTupleAndKeywords(cc.pyArgs.get(), cc.pyKwargs.get(), "|O",
				keywords, &pyHasClient))
			{
				PY_RETURN_ERROR;
			}

			defContext.hasClient = pyHasClient == Py_True;
		}
	}
	else if (defContext.optionName == "fixed_dict")
	{
		defContext.isModuleScope = true;

		if (PyObject_HasAttrString(pyFunc, "createObjFromDict") && PyObject_HasAttrString(pyFunc, "getDictFromObj") && PyObject_HasAttrString(pyFunc, "isSameType"))
		{
			defContext.implementedBy = pyFunc;

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
		// @Def.fixed_array() 
		// def ENTITYID_LIST()->ENTITY_ID: pass
		defContext.isModuleScope = false;
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

		if (defContext.optionName != "rename" && defContext.optionName != "fixed_array")
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

				if (defContext.optionName != "rename" && defContext.optionName != "fixed_array")
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
		{
			PY_RETURN_ERROR;
		}

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
			else if (parentClass == "Entity")
			{
				defContext.inheritEngineModuleType = DefContext::DC_TYPE_ENTITY;
				continue;
			}
			else if (parentClass == "Proxy")
			{
				if (defContext.componentType != BASEAPP_TYPE)
				{
					PyErr_Format(PyExc_AssertionError, "EntityDef.%s: \'%s\' Only BASE can inherit KBEngine.Proxy!\n", defContext.optionName.c_str(), moduleQualname);
					PY_RETURN_ERROR;
				}

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
		// 验证这个字符串是否可以还原成对象
		if (defContext.propertyDefaultVal.size() > 0)
		{
			PyObject* module = PyImport_AddModule("__main__");
			if (module == NULL)
			{
				noerror = false;
			}
			else
			{
				if (defContext.returnType != "UNICODE" && defContext.returnType != "STRING")
				{
					PyObject* mdict = PyModule_GetDict(module); // Borrowed reference.
					PyObject* result = PyRun_String(const_cast<char*>(defContext.propertyDefaultVal.c_str()),
						Py_eval_input, mdict, mdict);

					if (result == NULL)
						noerror = false;
					else
						Py_DECREF(result);
				}
			}
		}

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
		cc.pyArgs = PyObjectPtr(Copy::deepcopy(args), PyObjectPtr::STEAL_REF);	\
		cc.pyKwargs = kwargs ? PyObjectPtr(Copy::deepcopy(kwargs), PyObjectPtr::STEAL_REF) : PyObjectPtr(NULL);	\
		cc.optionName = #NAME;	\
		g_callContexts.push(cc);	\
		\
		return PyCFunction_New(&__call_def_parse, self);	\
	}

static PyObject* __py_def_rename(PyObject* self, PyObject* args, PyObject* kwargs)
{
	CallContext cc;
	cc.pyArgs = PyObjectPtr(Copy::deepcopy(args), PyObjectPtr::STEAL_REF);
	cc.pyKwargs = kwargs ? PyObjectPtr(Copy::deepcopy(kwargs), PyObjectPtr::STEAL_REF) : PyObjectPtr(NULL);
	cc.optionName = "rename";

	// 类似这种定义方式 EntityDef.rename(ENTITY_ID=EntityDef.INT32)
	if (kwargs)
	{
		PyObject *key, *value;
		Py_ssize_t pos = 0;

		while (PyDict_Next(kwargs, &pos, &key, &value)) 
		{
			std::string typeName = "";
			if (!PyUnicode_Check(value))
			{
				if (PyType_Check(value))
				{
					PyObject* pyQualname = PyObject_GetAttrString(value, "__qualname__");
					if (!pyQualname)
					{
						PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg2 get __qualname__ error! such as: EntityDef.rename(ENTITY_ID=EntityDef.INT32)\n", cc.optionName.c_str());
						return NULL;
					}

					typeName = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
					Py_DECREF(pyQualname);
				}
				else
				{
					PyObject* pyName = PyObject_GetAttrString(value, "__name__");
					if (!pyName)
					{
						PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg2 get __qualname__ error! such as: EntityDef.rename(ENTITY_ID=EntityDef.INT32)\n", cc.optionName.c_str());
						return NULL;
					}

					typeName = PyUnicode_AsUTF8AndSize(pyName, NULL);
					Py_DECREF(pyName);
				}
			}
			else
			{
				typeName = PyUnicode_AsUTF8AndSize(value, NULL);
			}

			if (!PyUnicode_Check(key))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg1 must be a string! such as: EntityDef.rename(ENTITY_ID=EntityDef.INT32)\n", cc.optionName.c_str());
				return NULL;
			}

			if (!isRefEntityDefModule(value))
			{
				DefContext::DEF_CONTEXT_MAP::iterator iter = DefContext::allScriptDefContextMaps.find(typeName);
				if (iter == DefContext::allScriptDefContextMaps.end())
				{
					PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg2 not legal type! such as: EntityDef.rename(ENTITY_ID=EntityDef.INT32)\n", cc.optionName.c_str());
					return NULL;
				}
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
	cc.pyArgs = PyObjectPtr(Copy::deepcopy(args), PyObjectPtr::STEAL_REF);
	cc.pyKwargs = kwargs ? PyObjectPtr(Copy::deepcopy(kwargs), PyObjectPtr::STEAL_REF) : PyObjectPtr(NULL);
	cc.optionName = "fixed_array";

	// 类似这种定义方式 EntityDef.fixed_array(XXArray=EntityDef.INT32)
	if (kwargs)
	{
		PyObject *key, *value;
		Py_ssize_t pos = 0;

		while (PyDict_Next(kwargs, &pos, &key, &value)) 
		{
			std::string typeName = "";
			if (!PyUnicode_Check(value))
			{
				if (PyType_Check(value))
				{
					PyObject* pyQualname = PyObject_GetAttrString(value, "__qualname__");
					if (!pyQualname)
					{
						PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg2 get __qualname__ error! such as: EntityDef.fixed_array(XXArray=EntityDef.INT32)\n", cc.optionName.c_str());
						return NULL;
					}

					typeName = PyUnicode_AsUTF8AndSize(pyQualname, NULL);
					Py_DECREF(pyQualname);
				}
				else
				{
					PyObject* pyName = PyObject_GetAttrString(value, "__name__");
					if (!pyName)
					{
						PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg2 get __qualname__ error! such as: EntityDef.fixed_array(XXArray=EntityDef.INT32)\n", cc.optionName.c_str());
						return NULL;
					}

					typeName = PyUnicode_AsUTF8AndSize(pyName, NULL);
					Py_DECREF(pyName);
				}
			}
			else
			{
				typeName = PyUnicode_AsUTF8AndSize(value, NULL);
			}

			if (!PyUnicode_Check(key))
			{
				PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg1 must be a string! such as: EntityDef.fixed_array(XXArray=EntityDef.INT32)\n", cc.optionName.c_str());
				return NULL;
			}

			if (!isRefEntityDefModule(value))
			{
				DefContext::DEF_CONTEXT_MAP::iterator iter = DefContext::allScriptDefContextMaps.find(typeName);
				if (iter == DefContext::allScriptDefContextMaps.end())
				{
					PyErr_Format(PyExc_AssertionError, "EntityDef.%s: arg2 not legal type! such as: EntityDef.fixed_array(XXArray=EntityDef.INT32)\n", cc.optionName.c_str());
					return NULL;
				}
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

static PyObject* _tp_entitydef_getattro(PyObject* self, PyObject* name)
{
	if (!isRefEntityDefModule(name))
	{
		const char* typeName = PyUnicode_AsUTF8AndSize(name, NULL);
		DefContext::DEF_CONTEXT_MAP::iterator iter = DefContext::allScriptDefContextMaps.find(typeName);
		if (iter != DefContext::allScriptDefContextMaps.end())
		{
			Py_INCREF(name);
			return name;
		}
	}

	return PyObject_GenericGetAttr(self, name);
}

//-------------------------------------------------------------------------------------
bool installModule(const char* moduleName)
{
	pyDefModuleName = moduleName;

	PyObject *entitydefModule = PyImport_AddModule(pyDefModuleName.c_str());
	PyObject* pyDoc = PyUnicode_FromString("This module is created by KBEngine!");
	PyObject_SetAttrString(entitydefModule, "__doc__", pyDoc);
	Py_DECREF(pyDoc);

	static  PyMethodDef entitydef_getattro_methods[] =
	{
		{ "__getattr__", (PyCFunction)&_tp_entitydef_getattro, METH_O, 0 },
		{ NULL, NULL, 0, NULL }
	};

	PyModule_AddFunctions(entitydefModule, entitydef_getattro_methods);

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
	std::string entryScriptFileName = "";
	if (loadComponentType == BASEAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getBaseApp();
		entryScriptFileName = info.entryScriptFile;
	}
	else if (loadComponentType == CELLAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getCellApp();
		entryScriptFileName = info.entryScriptFile;
	}
	else
	{
		KBE_ASSERT(false);
	}

	std::string rootPath = Resmgr::getSingleton().getPyUserComponentScriptsPath(loadComponentType);

	if (rootPath.size() == 0)
	{
		ERROR_MSG(fmt::format("PyEntityDef::loadAllScriptForComponentType(): get scripts path error! loadComponentType={}\n", 
			COMPONENT_NAME_EX(loadComponentType)));

		return false;
	}

	while (rootPath[rootPath.size() - 1] == '/' || rootPath[rootPath.size() - 1] == '\\') rootPath.pop_back();

	wchar_t* wpath = strutil::char2wchar((rootPath).c_str());

	wchar_t* _wentryScriptFileName = strutil::char2wchar((entryScriptFileName).c_str());
	std::wstring wentryScriptFileName = _wentryScriptFileName;
	free(_wentryScriptFileName);

	std::vector<std::wstring> results;
	Resmgr::getSingleton().listPathRes(wpath, L"py|pyc", results);

	// 优先执行入口脚本
	std::vector<std::wstring>::iterator iter = results.begin();
	for (; iter != results.end(); )
	{
		std::wstring wstrpath = (*iter);
		if (wstrpath.find(wentryScriptFileName + L".py") == std::wstring::npos && wstrpath.find(wentryScriptFileName + L".pyc") == std::wstring::npos)
		{
			++iter;
			continue;
		}

		iter = results.erase(iter);
	}

	results.insert(results.begin(), std::wstring(wpath) + L"/" + wentryScriptFileName + L".py");

	iter = results.begin();
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
				free(wpath);
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

	// 重定向python输出
	ScriptStdOutErr* pyStdouterr = new ScriptStdOutErr();

	// 安装py重定向脚本模块
	if (!pyStdouterr->install()) 
	{
		ERROR_MSG("PyEntityDef::execPython(): pyStdouterr->install() is failed!\n");
		delete pyStdouterr;
		SCRIPT_ERROR_CHECK();
		return false;
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

	if (pyStdouterr)
	{
		if (pyStdouterr->isInstall() && !pyStdouterr->uninstall()) {
			ERROR_MSG("PyEntityDef::execPython(): pyStdouterr->uninstall() is failed!\n");
		}

		delete pyStdouterr;
	}

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
	loadOtherComponentTypes.push_back(BASEAPP_TYPE);
	loadOtherComponentTypes.push_back(CELLAPP_TYPE);

	for (std::vector< COMPONENT_TYPE >::iterator iter = loadOtherComponentTypes.begin(); iter != loadOtherComponentTypes.end(); ++iter)
	{
		COMPONENT_TYPE componentType = (*iter);
		
		if (g_componentType == componentType)
		{
			if (!loadAllScriptForComponentType(g_componentType))
				return false;
		}
		else
		{
			if (!execPython(componentType))
				return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
static bool sortFun(const DefContext* def1, const DefContext* def2)
{
	return def1->order < def2->order;
}

//-------------------------------------------------------------------------------------
static bool updateScript(DefContext& defContext)
{
	PyObject* pyModule = EntityDef::loadScriptModule(defContext.moduleName);

	if (!pyModule)
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}

	PyObject* pyClass =
		PyObject_GetAttrString(pyModule, const_cast<char *>(defContext.moduleName.c_str()));

	if (pyClass == NULL)
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerEntityDefs: Could not find EntityClass[{}]\n",
			defContext.moduleName.c_str()));

		return false;
	}
	else
	{
		std::string typeNames = EntityDef::isSubClass(pyClass);

		if (typeNames.size() > 0)
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerEntityDefs: registerEntityDefs {} is not derived from KBEngine.[{}]\n",
				defContext.moduleName.c_str(), typeNames.c_str()));

			return false;
		}
	}

	if (!PyType_Check(pyClass))
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerEntityDefs: EntityClass[{}] is valid!\n",
			defContext.moduleName.c_str()));

		return false;
	}

	defContext.pyObjectPtr = PyObjectPtr(pyClass, PyObjectPtr::STEAL_REF);
	Py_DECREF(pyModule);

	return true;
}

//-------------------------------------------------------------------------------------
static void autosetHasClient(DefContext& defContext)
{
	if (!defContext.hasClient)
	{
		if (defContext.client_methods.size() > 0)
		{
			defContext.hasClient = true;
		}
		else
		{
			DefContext::DEF_CONTEXTS propertys = defContext.propertys;
			DefContext::DEF_CONTEXTS::iterator clientpropIter = propertys.begin();

			for (; clientpropIter != propertys.end(); ++clientpropIter)
			{
				if (((uint32)stringToEntityDataFlags(((*clientpropIter).propertyFlags)) & ENTITY_CLIENT_DATA_FLAGS) > 0)
				{
					defContext.hasClient = true;
					break;
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------
static bool registerDefTypes()
{
	std::vector< DefContext* > defContexts;

	{
		// 由于注册的类型有先后依赖关系，这里对他们做个排序再后续处理
		DefContext::DEF_CONTEXT_MAP::iterator iter = DefContext::allScriptDefContextMaps.begin();
		for (; iter != DefContext::allScriptDefContextMaps.end(); ++iter)
		{
			DefContext& defContext = iter->second;

			defContexts.push_back(&defContext);
		}

		std::sort(defContexts.begin(), defContexts.end(), sortFun);
	}

	{
		std::vector< DefContext* >::iterator iter = defContexts.begin();
		for (; iter != defContexts.end(); ++iter)
		{
			DefContext& defContext = *(*iter);

			if (defContext.type == DefContext::DC_TYPE_FIXED_ARRAY)
			{
				FixedArrayType* fixedArray = new FixedArrayType();

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
				FixedDictType* fixedDict = new FixedDictType();

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
static bool registerDefMethods(ScriptDefModule* pScriptModule, DefContext& defContext)
{
	DefContext::DEF_CONTEXTS methods;
	methods.insert(methods.end(), defContext.base_methods.begin(), defContext.base_methods.end());
	methods.insert(methods.end(), defContext.cell_methods.begin(), defContext.cell_methods.end());
	methods.insert(methods.end(), defContext.client_methods.begin(), defContext.client_methods.end());

	DefContext::DEF_CONTEXTS::iterator iter = methods.begin();
	for (; iter != methods.end(); ++iter)
	{
		DefContext& defMethodContext = (*iter);

		MethodDescription::EXPOSED_TYPE exposedType = MethodDescription::NO_EXPOSED;

		size_t argIdx = 0;

		// 检查第一个参数是否是Exposed callerID
		if (defMethodContext.argsvecs.size() > 0)
		{
			std::string argName = defMethodContext.argsvecs[0];
			std::string argType = defMethodContext.annotationsMaps[argName];

			// 如果是exposed方法的callerID参数就忽略
			if (argType == "CALLER_ID")
			{
				if (!defMethodContext.exposed)
				{
					ERROR_MSG(fmt::format("PyEntityDef::registerDefMethods: arg1 is Def.CallerID, but the method is not exposed! is {}.{}(arg={}), file: \"{}\"!\n",
						pScriptModule->getName(), defMethodContext.attrName.c_str(), argName, defMethodContext.pyObjectSourceFile));

					return false;
				}

				if (defMethodContext.componentType != CELLAPP_TYPE)
				{
					ERROR_MSG(fmt::format("PyEntityDef::registerDefMethods: arg1 is Def.CallerID, only the cell method supports this parameter.! is {}.{}(arg={}), file: \"{}\"!\n",
						pScriptModule->getName(), defMethodContext.attrName.c_str(), argName, defMethodContext.pyObjectSourceFile));

					return false;
				}

				exposedType = MethodDescription::EXPOSED_AND_CALLER_CHECK;
				argIdx = 1;
			}
		}

		if (defMethodContext.exposed && exposedType == MethodDescription::NO_EXPOSED)
			exposedType = MethodDescription::EXPOSED;

		MethodDescription* methodDescription = EntityDef::createMethodDescription(pScriptModule, defMethodContext.utype > 0 ? defMethodContext.utype : 0,
			defMethodContext.componentType, defMethodContext.attrName, exposedType);

		if (!methodDescription)
			return false;

		for (; argIdx < defMethodContext.argsvecs.size(); ++argIdx)
		{
			std::string argName = defMethodContext.argsvecs[argIdx];
			std::string argType = defMethodContext.annotationsMaps[argName];

			// 如果是exposed方法的callerID参数就忽略
			if (argType == "CALLER_ID")
			{
				ERROR_MSG(fmt::format("PyEntityDef::registerDefMethods: Def.CallerID must be the first parameter! is {}.{}(arg={}), file: \"{}\"!\n",
					pScriptModule->getName(), defMethodContext.attrName.c_str(), argName, defMethodContext.pyObjectSourceFile));

				return false;
			}

			DataType* dataType = DataTypes::getDataType(argType, false);

			if (!dataType)
			{
				DefContext* pDefMethodArgContext = DefContext::findDefContext(argType);
				if (!pDefMethodArgContext)
				{
					ERROR_MSG(fmt::format("PyEntityDef::registerDefMethods: not fount type[{}], is {}.{}(arg={}), file: \"{}\"!\n",
						argType, pScriptModule->getName(), defMethodContext.attrName.c_str(), argName, defMethodContext.pyObjectSourceFile));

					return false;
				}

				if (pDefMethodArgContext->type == DefContext::DC_TYPE_FIXED_ARRAY)
				{
					FixedArrayType* dataType1 = new FixedArrayType();
					std::string parentName = defContext.moduleName + "_" + defMethodContext.attrName;

					if (!dataType1->initialize(pDefMethodArgContext, parentName))
					{
						ERROR_MSG(fmt::format("PyEntityDef::registerDefMethods: parse ARRAY [{}.{}(arg={})] error! file: \"{}\"!\n",
							pScriptModule->getName(), defMethodContext.attrName.c_str(), argName, defMethodContext.pyObjectSourceFile));

						delete dataType1;
						return false;
					}
					
					dataType = dataType1;
				}
			}

			if (dataType == NULL)
			{
				ERROR_MSG(fmt::format("PyEntityDef::registerDefMethods: dataType[{}] not found, in {}.{}(arg={}), file: \"{}\"!\n",
					argType, pScriptModule->getName(), defMethodContext.attrName.c_str(), argName, defMethodContext.pyObjectSourceFile));

				return false;
			}

			methodDescription->pushArgType(dataType);
		}

		if (defMethodContext.componentType == CELLAPP_TYPE)
		{
			if (!pScriptModule->addCellMethodDescription(defMethodContext.attrName.c_str(), methodDescription))
				return false;
		}
		else if(defMethodContext.componentType == BASEAPP_TYPE)
		{
			if (!pScriptModule->addBaseMethodDescription(defMethodContext.attrName.c_str(), methodDescription))
				return false;
		}
		else
		{
			if (!pScriptModule->addClientMethodDescription(defMethodContext.attrName.c_str(), methodDescription))
				return false;
		}
	}

	// 除了这几个进程以外，其他进程不需要根据方法检测脚本的正确性
	if (g_componentType == BASEAPP_TYPE || g_componentType == CELLAPP_TYPE || g_componentType == BOTS_TYPE || g_componentType == CLIENT_TYPE)
	{
		if (!EntityDef::checkDefMethod(pScriptModule, defContext.pyObjectPtr.get(), defContext.moduleName))
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefMethods: EntityClass[{}] checkDefMethod is failed!\n",
				defContext.moduleName.c_str()));

			return false;
		}
	}

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
		bool						isIdentifier = false;									// 是否是一个索引键
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
				defPropContext.propertyFlags, pScriptModule->getName(), name.c_str(), defPropContext.pyObjectSourceFile));

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
	DefContext::DEF_CONTEXTS& components = defContext.components;

	DefContext::DEF_CONTEXTS::iterator iter = components.begin();
	for (; iter != components.end(); ++iter)
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

		autosetHasClient(*pDefPropTypeContext);

		// 除了这几个进程以外，其他进程不需要访问脚本
		if (g_componentType == BASEAPP_TYPE || g_componentType == CELLAPP_TYPE || g_componentType == BOTS_TYPE || g_componentType == CLIENT_TYPE)
		{
			// 如果是bots类型，需要将脚本类设置为程序环境的类
			// 注意：如果是CLIENT_TYPE使用def文件模式或者将定义放在一个common的py中，因为该模式相关定义都在服务器代码上，而客户端环境没有服务器代码
			if ((g_componentType == BOTS_TYPE || g_componentType == CLIENT_TYPE) && pDefPropTypeContext->hasClient)
			{
				if (!updateScript(*pDefPropTypeContext))
					return false;
			}

			if (pDefPropTypeContext->pyObjectSourceFileComponentType == g_componentType)
			{
				PyObject* pyClass = pDefPropTypeContext->pyObjectPtr.get();
				if (pyClass)
				{
					if (!PyType_Check(pyClass))
					{
						ERROR_MSG(fmt::format("PyEntityDef::registerDefComponents: EntityClass[{}] is valid!\n",
							pDefPropTypeContext->moduleName.c_str()));

						return false;
					}

					Py_INCREF((PyTypeObject *)pyClass);
					pCompScriptDefModule->setScriptType((PyTypeObject *)pyClass);
				}
			}
		}

		// 注册属性描述
		if (!registerDefPropertys(pCompScriptDefModule, *pDefPropTypeContext))
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefComponents: failed to registerDefPropertys(), entity:{}\n",
				pScriptModule->getName()));

			return false;
		}

		// 注册方法描述
		if(!registerDefMethods(pCompScriptDefModule, *pDefPropTypeContext))
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefComponents: failed to registerDefMethods(), entity:{}\n",
				pScriptModule->getName()));

			return false;
		}

		// 尝试加载detailLevelInfo数据
		if (!registerDetailLevelInfo(pCompScriptDefModule, *pDefPropTypeContext))
		{
			ERROR_MSG(fmt::format("PyEntityDef::registerDefComponents: failed to register component:{} detailLevelInfo.\n",
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
	// 注册属性描述
	if (!registerDefPropertys(pScriptModule, defContext))
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerEntityDef: failed to registerDefPropertys(), entity:{}\n",
			pScriptModule->getName()));

		return false;
	}

	// 注册方法描述
	if (!registerDefMethods(pScriptModule, defContext))
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerDefComponents: failed to registerDefMethods(), entity:{}\n",
			pScriptModule->getName()));

		return false;
	}

	// 注册组件描述， 并将他们的方法和属性加入到模块中
	if (!registerDefComponents(pScriptModule, defContext))
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerEntityDef: failed to registerDefComponents(), component:{}\n",
			pScriptModule->getName()));

		return false;
	}

	// 尝试注册detailLevelInfo数据
	if (!registerDetailLevelInfo(pScriptModule, defContext))
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerEntityDef: failed to register entity:{} detailLevelInfo.\n",
			pScriptModule->getName()));

		return false;
	}

	// 尝试注册volatileInfo数据
	if (!registerVolatileInfo(pScriptModule, defContext))
	{
		ERROR_MSG(fmt::format("PyEntityDef::registerEntityDef: failed to register entity:{} volatileInfo.\n",
			pScriptModule->getName()));

		return false;
	}

	// 除了这几个进程以外，其他进程不需要访问脚本
	if (g_componentType == BASEAPP_TYPE || g_componentType == CELLAPP_TYPE || g_componentType == BOTS_TYPE || g_componentType == CLIENT_TYPE)
	{
		if (defContext.pyObjectSourceFileComponentType == g_componentType)
		{
			PyObject* pyClass = defContext.pyObjectPtr.get();
			if (pyClass)
			{
				if (!PyType_Check(pyClass))
				{
					ERROR_MSG(fmt::format("PyEntityDef::registerEntityDef: EntityClass[{}] is valid!\n",
						defContext.moduleName.c_str()));

					return false;
				}

				Py_INCREF((PyTypeObject *)pyClass);
				pScriptModule->setScriptType((PyTypeObject *)pyClass);
			}
		}
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

		// 根据是否包含客户端属性或者方法决定是否这个实体拥有客户端部分
		// hasClient=True也可以强制指定拥有客户端部分
		// autosetHasClient(defContext);

		// 如果是bots类型，需要将脚本类设置为程序环境的类
		// 注意：如果是CLIENT_TYPE使用def文件模式或者将定义放在一个common的py中，因为该模式相关定义都在服务器代码上，而客户端环境没有服务器代码
		if ((g_componentType == BOTS_TYPE || g_componentType == CLIENT_TYPE) && defContext.hasClient)
		{
			if (!updateScript(defContext))
				return false;
		}

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

	KBE_ASSERT(pyDefModuleName.size() > 0);
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
	
	static const char* CALLER_ID = "CALLER_ID";
	if (PyModule_AddStringConstant(entitydefModule, CALLER_ID, CALLER_ID))
	{
		ERROR_MSG(fmt::format("PyEntityDef::initialize(): Unable to set EntityDef.{} to {}\n",
			CALLER_ID, CALLER_ID));

		return false;
	}

	//APPEND_SCRIPT_MODULE_METHOD(entitydefModule, ARRAY, __py_array, METH_VARARGS, 0);

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
