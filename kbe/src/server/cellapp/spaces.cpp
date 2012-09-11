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
		ERROR_MSG("Spaces::createNewSpace: space %u is exist!\n", spaceID);
		return NULL;
	}
	
	spaces_[spaceID].reset(space);
	DEBUG_MSG("Spaces::createNewSpace: new space %u.\n", spaceID);
	return space;
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
