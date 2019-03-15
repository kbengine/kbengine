/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_PY_PLATFORM_H
#define KBE_PY_PLATFORM_H

#include "common/common.h"
#include "scriptobject.h"

namespace KBEngine{ namespace script{

class PyPlatform
{						
public:	
	static bool initialize(void);
	static void finalise(void);

	static bool rmdir(const std::string& path);
	static bool rmdir(const std::wstring& path);

	static bool pathExists(const std::string& path);
	static bool pathExists(const std::wstring& path);

	static std::pair<std::string, std::string> splitPath(const std::string& path);
	static std::pair<std::wstring, std::wstring> splitPath(const std::wstring& path);

	static std::pair<std::string, std::string> splitText(const std::string& path);
	static std::pair<std::wstring, std::wstring> splitText(const std::wstring& path);

private:
	static bool	isInit;
	
} ;

}
}

#endif // KBE_PY_PLATFORM_H
