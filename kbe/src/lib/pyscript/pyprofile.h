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

#ifndef KBE_SCRIPT_PY_PROFILE_H
#define KBE_SCRIPT_PY_PROFILE_H

#include "common/common.h"
#include "common/smartpointer.h"
#include "scriptobject.h"

namespace KBEngine{ 
class MemoryStream;	
typedef SmartPointer<PyObject> PyObjectPtr;

namespace script{

class Script;

class PyProfile
{						
public:	
	/** 
		激活与停止某个profile
	*/
	static bool start(std::string profile);
	static bool stop(std::string profile);
	static bool dump(std::string profile, std::string fileName);
	static void addToStream(std::string profile, MemoryStream* s);
	static bool remove(std::string profile);

	static void print_stats(const std::string& sort = "time", const std::string& profileName = "kbengine");

	/** 
		初始化pickler 
	*/
	static bool initialize(Script* pScript);
	static void finalise(void);
	
private:
	typedef KBEUnordered_map< std::string, PyObjectPtr > PROFILES;
	static PROFILES profiles_;

	static PyObject* profileMethod_;

	static bool	isInit;										// 是否已经被初始化

	static Script* pScript_;
} ;

}
}
#endif // KBE_SCRIPT_PY_PROFILE_H
