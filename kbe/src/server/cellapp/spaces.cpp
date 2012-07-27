#include "spaces.hpp"	
namespace KBEngine{	
std::map<SPACE_ID, Space*> Spaces::spaces_;

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
	std::map<SPACE_ID, Space*>::iterator iter = spaces_.find(spaceID);
	
	if(iter != spaces_.end())
	{
		ERROR_MSG("Spaces::createNewSpace: space %ld is exist!\n", spaceID);
		return NULL;
	}
	
	spaces_[spaceID] = space;
	return space;
}

//-------------------------------------------------------------------------------------
Space* Spaces::findSpace(SPACE_ID spaceID)
{
	std::map<SPACE_ID, Space*>::iterator iter = spaces_.find(spaceID);
	if(iter != spaces_.end())
		return iter->second;
	
	return NULL;
}

//-------------------------------------------------------------------------------------
void Spaces::update()
{
	std::map<SPACE_ID, Space*>::iterator iter = spaces_.begin();
	for(;iter != spaces_.end(); iter++)
		iter->second->update();
}

//-------------------------------------------------------------------------------------
}
