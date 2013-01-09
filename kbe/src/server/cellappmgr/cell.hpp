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

#ifndef __MGR_CELL_HPP__
#define __MGR_CELL_HPP__

// common include
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"


namespace KBEngine{

class Cell
{
public:
	Cell(CELL_ID id);
	~Cell();

	CELL_ID id()const{ return id_; }
private:
	CELL_ID id_;
};

}
#endif
