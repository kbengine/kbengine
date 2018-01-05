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

#ifndef KBE_MGR_SPACE_H
#define KBE_MGR_SPACE_H

// common include
#include "cells.h"
#include "helper/debug_helper.h"
#include "common/common.h"


namespace KBEngine{

class Space
{
public:
	Space();
	~Space();

	void updateGeomappingPath(const std::string& geomappingPath);
	void updateScriptModuleName(const std::string& scriptModuleName);
	void setSpaceID(SPACE_ID spaceID) { spaceID_ = spaceID; }

	SPACE_ID id() const { return spaceID_; }
	std::string& getGeomappingPath() { return geomappingPath_; }
	std::string& getScriptModuleName() { return scriptModuleName_; }

	Cells& cells() { return cells_; }

private:
	SPACE_ID spaceID_;
	Cells cells_;

	std::string geomappingPath_;
	std::string scriptModuleName_;
};

}
#endif
