/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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
#ifndef __KBE_NAVIGATION_HPP__
#define __KBE_NAVIGATION_HPP__

#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "cstdkbe/singleton.hpp"
#include "math/math.hpp"
#include "navigation_handle.hpp"

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
	
	NavigationHandlePtr loadNavigation(std::string name);

	bool hasNavigation(std::string name);

	bool removeNavigation(std::string name);

	NavigationHandlePtr findNavigation(std::string name);
private:
	KBEUnordered_map<std::string, NavigationHandlePtr> navhandles_;
	KBEngine::thread::ThreadMutex mutex_;
};

}
#endif
