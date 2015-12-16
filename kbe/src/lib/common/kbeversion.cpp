/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

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
		char buf[ 256 ];
		kbe_snprintf( buf, 256, "%d.%d.%d",
				KBE_VERSION_MAJOR, KBE_VERSION_MINOR, KBE_VERSION_PATCH );
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


