#include "cellapp.hpp"
#include "space.hpp"	
#include "entity.hpp"	
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
	pEntity->setSpaceID(this->id_);
	pEntity->spaceEntityIdx(entities_.size());
	entities_.push_back(pEntity);
}

//-------------------------------------------------------------------------------------
void Space::removeEntity(Entity* pEntity)
{
	pEntity->setSpaceID(0);
	
	// 先获取到所在位置
	SPACE_ENTITIES::size_type idx = pEntity->spaceEntityIdx();

	KBE_ASSERT(idx < entities_.size());
	KBE_ASSERT(entities_[ idx ] == pEntity);

	// 如果有2个或以上的entity则将最后一个entity移至删除的这个entity所在位置
	Entity* pBack = entities_.back().get();
	pBack->spaceEntityIdx(idx);
	entities_[idx] = pBack;
	pEntity->spaceEntityIdx(SPACE_ENTITIES::size_type(-1));
	entities_.pop_back();

	// 如果没有entity了则需要销毁space, 因为space最少存在一个entity
	if(entities_.empty())
	{
	}
}

//-------------------------------------------------------------------------------------
}
