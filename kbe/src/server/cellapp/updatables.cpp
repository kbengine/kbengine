#include "updatables.hpp"	
#include "helper/profile.hpp"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Updatables::Updatables()
{
}

//-------------------------------------------------------------------------------------
Updatables::~Updatables()
{
}

//-------------------------------------------------------------------------------------
bool Updatables::add(Updatable* updatable)
{
	static uint32 idx = 1;
	objects_[idx] = updatable;

	// ¼ÇÂ¼´æ´¢Î»ÖÃ
	updatable->removeIdx = idx++;

	return true;
}

//-------------------------------------------------------------------------------------
bool Updatables::remove(Updatable* updatable)
{
	objects_.erase(updatable->removeIdx);
	updatable->removeIdx = -1;
	return true;
}

//-------------------------------------------------------------------------------------
void Updatables::update()
{
	AUTO_SCOPED_PROFILE("callUpdates");

	std::map<uint32, Updatable*>::iterator iter = objects_.begin();
	for(; iter != objects_.end(); iter++)
	{
		iter->second->update();
	}
}

//-------------------------------------------------------------------------------------
}
