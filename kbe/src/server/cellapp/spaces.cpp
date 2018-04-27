// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "spaces.h"	
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
void Spaces::finalise()
{
	Spaces::SPACES spaces = spaces_;
	while (spaces.size() > 0)
	{
		SPACES::iterator iter = spaces.begin();
		KBEShared_ptr<Space> pSpace = iter->second;
		spaces.erase(iter++);
		pSpace->destroy(0, false);
	}

	spaces_.clear();
}

//-------------------------------------------------------------------------------------
Space* Spaces::createNewSpace(SPACE_ID spaceID, const std::string& scriptModuleName)
{
	SPACES::iterator iter = spaces_.find(spaceID);
	if(iter != spaces_.end())
	{
		ERROR_MSG(fmt::format("Spaces::createNewSpace: space {} is exist! scriptModuleName={}\n", spaceID, scriptModuleName));
		return NULL;
	}
	
	Space* space = new Space(spaceID, scriptModuleName);
	spaces_[spaceID].reset(space);
	
	DEBUG_MSG(fmt::format("Spaces::createNewSpace: new space({}) {}.\n", scriptModuleName, spaceID));
	return space;
}

//-------------------------------------------------------------------------------------
bool Spaces::destroySpace(SPACE_ID spaceID, ENTITY_ID entityID)
{
	INFO_MSG(fmt::format("Spaces::destroySpace: {}.\n", spaceID));

	Space* pSpace = Spaces::findSpace(spaceID);
	if(!pSpace)
		return true;
	
	if(pSpace->isDestroyed())
		return true;

	if(!pSpace->destroy(entityID))
	{
		//WARNING_MSG("Spaces::destroySpace: destroying!\n");
		return false;
	}

	// 延时一段时间再销毁
	//spaces_.erase(spaceID);
	return true;
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

	for(; iter != spaces_.end(); )
	{
		if(!iter->second->update())
		{
			spaces_.erase(iter++);
		}
		else
		{
			++iter;
		}
	}
}

//-------------------------------------------------------------------------------------
}
