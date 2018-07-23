// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "space.h"
#include "helper/profile.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Space::Space() :
spaceID_(0),
cells_(),
geomappingPath_(),
scriptModuleName_()
{
}

//-------------------------------------------------------------------------------------
Space::~Space()
{
}

//-------------------------------------------------------------------------------------
void Space::updateGeomappingPath(const std::string& geomappingPath)
{
	geomappingPath_ = geomappingPath;
}

//-------------------------------------------------------------------------------------
void Space::updateScriptModuleName(const std::string& scriptModuleName)
{
	scriptModuleName_ = scriptModuleName;
}

//-------------------------------------------------------------------------------------
}
