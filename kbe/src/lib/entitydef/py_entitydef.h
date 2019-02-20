// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SCRIPT_ENTITYDEF_H
#define KBE_SCRIPT_ENTITYDEF_H

#include "common.h"
#include "pyscript/pyobject_pointer.h"

namespace KBEngine{ 

class MemoryStream;

namespace script { namespace entitydef {

class DefContext
{
public:
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

	DefContext();

	bool addToStream(MemoryStream* pMemoryStream);
	bool createFromStream(MemoryStream* pMemoryStream);

	bool addChildContext(DefContext& defContext);

	static DefContext* findDefContext(const std::string& name)
	{
		DEF_CONTEXT_MAP::iterator iter = allScriptDefContextMaps.find(name);
		if (iter != script::entitydef::DefContext::allScriptDefContextMaps.end())
		{
			return &iter->second;
		}

		return NULL;
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
	int utype;

	std::string detailLevel;

	std::string propertyFlags;
	std::string propertyIndex;
	std::string propertyDefaultVal;

	PyObjectPtr implementedBy;
	std::string implementedByModuleName;
	std::string implementedByModuleFile;

	PyObjectPtr pyObjectPtr;
	std::string pyObjectSourceFile;

	std::vector< std::string > baseClasses;

	DCType inheritEngineModuleType;
	DCType type;

	typedef std::vector< DefContext > DEF_CONTEXTS;

	DEF_CONTEXTS methods;
	DEF_CONTEXTS client_methods;
	DEF_CONTEXTS propertys;
	std::vector< std::string > components;

	COMPONENT_TYPE componentType;

	typedef std::map<std::string, DefContext> DEF_CONTEXT_MAP;
	static DEF_CONTEXT_MAP allScriptDefContextMaps;
	static DEF_CONTEXT_MAP allScriptDefContextLineMaps;
};

/** 安装entitydef模块 */
bool installModule(const char* moduleName);
bool uninstallModule();

bool initialize();
bool finalise(bool isReload = false);

void reload(bool fullReload);

bool initializeWatcher();

bool addToStream(MemoryStream* pMemoryStream);
bool createFromStream(MemoryStream* pMemoryStream);

}
}
}

#endif // KBE_SCRIPT_ENTITYDEF_H
