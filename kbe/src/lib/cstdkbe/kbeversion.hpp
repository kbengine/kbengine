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

#include "cstdkbe/platform.hpp"
#ifndef __KBEVERSION_HPP__
#define __KBEVERSION_HPP__
namespace KBEngine{
	
#define KBE_VERSION_MAJOR 0
#define KBE_VERSION_MINOR 1
#define KBE_VERSION_PATCH 6

namespace KBEVersion
{
	const std::string & versionString();
}

}
#endif // __KBEVERSION_HPP__
