#include "spaces.hpp"	
namespace KBEngine{	
Spaces::SPACES Spaces::spaces_;

//-------------------------------------------------------------------------------------
Spaces::Spaces()
{
}

//-------------------------------------------------------------------------------------
Spaces::~Spaces()
{
}

//-------------------------------------------------------------------------------------
Space* Spaces::createNewSpace(SPACE_ID spaceID)
{
	Space* space = new Space(spaceID);
	SPACES::iterator iter = spaces_.find(spaceID);
	
	if(iter != spaces_.end())
	{
		ERROR_MSG(boost::format("Spaces::createNewSpace: space %1% is exist!\n") % spaceID);
		return NULL;
	}
	
	spaces_[spaceID].reset(space);
	DEBUG_MSG(boost::format("Spaces::createNewSpace: new space %1%.\n") % spaceID);
	return space;
}

//-------------------------------------------------------------------------------------
bool Spaces::destroySpace(SPACE_ID spaceID, ENTITY_ID entityID)
{
	INFO_MSG(boost::format("Spaces::destroySpace: %1%.\n") % spaceID);

	Space* pSpace = Spaces::findSpace(spaceID);
	return pSpace->destroy(entityID);
}

//-------------------------------------------------------------------------------------
Space* Spaces::findSpace(SPACE_ID spaceID)
{
	SPACES::iterator iter = spaces_.find(spaceID);
	if(iter != spaces_.end())
		return iter->second.get();
	
	return NULL;
}

//-------------------------------------------------------------------------------------
void Spaces::update()
{
	SPACES::iterator iter = spaces_.begin();
	for(;iter != spaces_.end(); iter++)
		iter->second->update();
}

//-------------------------------------------------------------------------------------
}
