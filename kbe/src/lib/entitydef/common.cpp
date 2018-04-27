// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "common.h"
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


}
