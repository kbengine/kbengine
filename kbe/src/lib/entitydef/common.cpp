// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "common.h"
#include "resmgr/resmgr.h"

namespace KBEngine{

ENTITYFLAGMAP g_entityFlagMapping;

//-------------------------------------------------------------------------------------
std::string entityDataFlagsToString(uint32 flags)
{
	if(flags == ED_FLAG_CELL_PUBLIC)
		return "CELL_PUBLIC";

	if(flags == ED_FLAG_CELL_PRIVATE)
		return "CELL_PRIVATE";

	if(flags == ED_FLAG_ALL_CLIENTS)
		return "ALL_CLIENTS";

	if(flags == ED_FLAG_CELL_PUBLIC_AND_OWN)
		return "CELL_PUBLIC_AND_OWN";

	if(flags == ED_FLAG_OWN_CLIENT)
		return "OWN_CLIENT";

	if(flags == ED_FLAG_BASE_AND_CLIENT)
		return "BASE_AND_CLIENT";

	if(flags == ED_FLAG_BASE)
		return "BASE";

	if(flags == ED_FLAG_OTHER_CLIENTS)
		return "OTHER_CLIENTS";

	return "UNKOWN";
}

//-------------------------------------------------------------------------------------
EntityDataFlags stringToEntityDataFlags(const std::string& strFlags)
{
	if (strFlags == "CELL_PUBLIC")
		return ED_FLAG_CELL_PUBLIC;

	if (strFlags == "CELL_PRIVATE")
		return ED_FLAG_CELL_PRIVATE;

	if (strFlags == "ALL_CLIENTS")
		return ED_FLAG_ALL_CLIENTS;

	if (strFlags == "CELL_PUBLIC_AND_OWN")
		return ED_FLAG_CELL_PUBLIC_AND_OWN;

	if (strFlags == "OWN_CLIENT")
		return ED_FLAG_OWN_CLIENT;

	if (strFlags == "BASE_AND_CLIENT")
		return ED_FLAG_BASE_AND_CLIENT;

	if (strFlags == "BASE")
		return ED_FLAG_BASE;

	if (strFlags == "OTHER_CLIENTS")
		return ED_FLAG_OTHER_CLIENTS;

	return ED_FLAG_UNKOWN;
}

//-------------------------------------------------------------------------------------
std::pair<std::wstring, std::wstring> getComponentPythonPaths(COMPONENT_TYPE componentType)
{
	std::pair<std::wstring, std::wstring> pathPair = std::make_pair(L"", L"");

	std::wstring user_scripts_path = L"";
	wchar_t* tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(Resmgr::getSingleton().getPyUserScriptsPath().c_str()));
	if (tbuf != NULL)
	{
		user_scripts_path += tbuf;
		free(tbuf);
	}
	else
	{
		return pathPair;
	}

	std::wstring pyPaths = user_scripts_path + L";";
	pyPaths += user_scripts_path + L"common;";
	pyPaths += user_scripts_path + L"data;";
	pyPaths += user_scripts_path + L"user_type;";

	switch (componentType)
	{
	case BASEAPP_TYPE:
		pyPaths += user_scripts_path + L"server_common;";
		pyPaths += user_scripts_path + L"base;";
		pyPaths += user_scripts_path + L"base/interfaces;";
		pyPaths += user_scripts_path + L"base/components;";
		break;
	case CELLAPP_TYPE:
		pyPaths += user_scripts_path + L"server_common;";
		pyPaths += user_scripts_path + L"cell;";
		pyPaths += user_scripts_path + L"cell/interfaces;";
		pyPaths += user_scripts_path + L"cell/components;";
		break;
	case DBMGR_TYPE:
		pyPaths += user_scripts_path + L"server_common;";
		pyPaths += user_scripts_path + L"db;";
		break;
	default:
		pyPaths += user_scripts_path + L"client;";
		pyPaths += user_scripts_path + L"client/interfaces;";
		pyPaths += user_scripts_path + L"client/components;";
		break;
	};

	std::string kbe_res_path = Resmgr::getSingleton().getPySysResPath();
	kbe_res_path += "scripts/common";

	tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(kbe_res_path.c_str()));
	pathPair.first = tbuf;
	pathPair.second = pyPaths;
	free(tbuf);
	return pathPair;
}

//-------------------------------------------------------------------------------------

}
