// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "kbeversion.h"
#include "helper/debug_helper.h"
namespace KBEngine{
namespace KBEVersion
{
std::string g_versionString;

/**
	½Å±¾²ã°æ±¾
*/
std::string g_scriptVersion = "0.0.0";

const std::string & versionString()
{
	if(g_versionString.size() == 0)
	{
		char buf[MAX_BUF];
		kbe_snprintf(buf, MAX_BUF, "%d.%d.%d", KBE_VERSION_MAJOR, KBE_VERSION_MINOR, KBE_VERSION_PATCH);
		g_versionString = buf;
	}

	return g_versionString;
}

void setScriptVersion(const std::string& ver)
{
	g_scriptVersion = ver;
}

const std::string & scriptVersionString()
{
	return g_scriptVersion;
}


}
} 


