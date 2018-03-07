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
#ifndef KBE_NAVIGATION_H
#define KBE_NAVIGATION_H

#include "common/common.h"
#include "helper/debug_helper.h"
#include "common/smartpointer.h"
#include "common/singleton.h"
#include "math/math.h"
#include "navigation_handle.h"

namespace KBEngine
{
/*
	µº∫Ω¿‡
*/
class Navigation : public Singleton<Navigation>
{
public:
	Navigation();
	virtual ~Navigation();
	
	void finalise();

	NavigationHandlePtr loadNavigation(std::string resPath, const std::map< int, std::string >& params);

	bool hasNavigation(std::string resPath);

	bool removeNavigation(std::string resPath);

	NavigationHandlePtr findNavigation(std::string resPath);

private:
	KBEUnordered_map<std::string, NavigationHandlePtr> navhandles_;
	KBEngine::thread::ThreadMutex mutex_;
};

}
#endif
