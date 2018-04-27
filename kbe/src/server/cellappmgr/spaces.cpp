// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "space.h"
#include "spaces.h"
#include "helper/profile.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Spaces::Spaces():
spaces_()
{
}

//-------------------------------------------------------------------------------------
Spaces::~Spaces()
{
	spaces_.clear();
}

//-------------------------------------------------------------------------------------
void Spaces::updateSpaceData(SPACE_ID spaceID, const std::string& scriptModuleName, const std::string& geomappingPath, bool deleted)
{
	if (deleted)
	{
		spaces_.erase(spaceID);
		return;
	}

	Space& space = spaces_[spaceID];

	space.setSpaceID(spaceID);
	space.updateGeomappingPath(geomappingPath);
	space.updateScriptModuleName(scriptModuleName);
}

//-------------------------------------------------------------------------------------
}
