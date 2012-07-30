#include "cellapp.hpp"
#include "space.hpp"	
#include "entitydef/entities.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
Space::Space(SPACE_ID spaceID, int32 mapSize):
id_(spaceID),
entities_(),
isLoadGeometry_(false),
mapSize_(mapSize),
loadGeometryPathName_()
{
}

//-------------------------------------------------------------------------------------
Space::~Space()
{
}

//-------------------------------------------------------------------------------------
void Space::loadSpaceGeometry(const char* path)
{
	loadGeometryPathName_ = path;
}

//-------------------------------------------------------------------------------------
void Space::update()
{
	if(!isLoadGeometry_)
		return;
}

//-------------------------------------------------------------------------------------
void Space::addEntity(Entity* pEntity)
{
}

//-------------------------------------------------------------------------------------
}
