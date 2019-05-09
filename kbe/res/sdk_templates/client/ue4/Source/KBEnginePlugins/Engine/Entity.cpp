#include "Entity.h"
#include "KBEngine.h"
#include "KBEvent.h"
#include "DataTypes.h"
#include "Method.h"
#include "ScriptModule.h"
#include "EntityCall.h"
#include "Bundle.h"
#include "EntityDef.h"
#include "Property.h"
#include "KBDebug.h"

namespace KBEngine
{

Entity::Entity():
	id_(0),
	className_(TEXT("")),
	isOnGround_(false),
	inWorld_(false),
	isControlled_(false),
	inited_(false),
	velocity_(0.f),
	position(),
	direction(),
	spaceID(0),
	entityLastLocalPos(),
	entityLastLocalDir()
{
}

Entity::~Entity()
{
	DEBUG_MSG("%s::~%s() %d", *className_, *className_, id());
}

void Entity::clear()
{

}

void Entity::__init__()
{

}

bool Entity::isPlayer()
{
	return id() == KBEngineApp::getSingleton().entity_id();
}

void Entity::enterWorld()
{
	inWorld_ = true;

	onEnterWorld();
	onComponentsEnterworld();

	UKBEventData_onEnterWorld* pEventData = NewObject<UKBEventData_onEnterWorld>();
	pEventData->entityID = id();
	pEventData->spaceID = KBEngineApp::getSingleton().spaceID();
	KBPos2UE4Pos(pEventData->position, position);
	pEventData->direction = direction;
	pEventData->moveSpeed = velocity_;
	pEventData->isOnGround = isOnGround_;
	pEventData->isPlayer = isPlayer();
	pEventData->entityClassName = className();
	pEventData->res = TEXT("");
	KBENGINE_EVENT_FIRE(KBEventTypes::onEnterWorld, pEventData);
}

void Entity::onEnterWorld()
{

}

void Entity::leaveWorld()
{
	inWorld_ = false;

	onLeaveWorld();
	onComponentsLeaveworld();

	UKBEventData_onLeaveWorld* pEventData = NewObject<UKBEventData_onLeaveWorld>();
	pEventData->entityID = id();
	pEventData->spaceID = KBEngineApp::getSingleton().spaceID();
	pEventData->isPlayer = isPlayer();
	KBENGINE_EVENT_FIRE(KBEventTypes::onLeaveWorld, pEventData);
}

void Entity::onLeaveWorld()
{

}

void Entity::enterSpace()
{
	inWorld_ = true;

	onEnterSpace();

	UKBEventData_onEnterSpace* pEventData = NewObject<UKBEventData_onEnterSpace>();
	pEventData->entityID = id();
	pEventData->spaceID = KBEngineApp::getSingleton().spaceID();
	KBPos2UE4Pos(pEventData->position, position);
	pEventData->direction = direction;
	pEventData->moveSpeed = velocity_;
	pEventData->isOnGround = isOnGround_;
	pEventData->isPlayer = isPlayer();
	pEventData->entityClassName = className();
	pEventData->res = TEXT("");
	KBENGINE_EVENT_FIRE(KBEventTypes::onEnterSpace, pEventData);
	
	// 要立即刷新表现层对象的位置
	UKBEventData_set_position* pPosEventData = NewObject<UKBEventData_set_position>();
	KBPos2UE4Pos(pPosEventData->position, position);
	pPosEventData->entityID = id();
	pPosEventData->moveSpeed = velocity_;
	pPosEventData->isOnGround = isOnGround();
	KBENGINE_EVENT_FIRE(KBEventTypes::set_position, pPosEventData);

	UKBEventData_set_direction* pDirEventData = NewObject<UKBEventData_set_direction>();
	KBDir2UE4Dir(pDirEventData->direction, direction);
	pDirEventData->entityID = id();
	KBENGINE_EVENT_FIRE(KBEventTypes::set_direction, pDirEventData);
}

void Entity::onEnterSpace()
{

}

void Entity::leaveSpace()
{
	inWorld_ = false;

	onLeaveSpace();

	UKBEventData_onLeaveSpace* pEventData = NewObject<UKBEventData_onLeaveSpace>();
	pEventData->entityID = id();
	pEventData->spaceID = KBEngineApp::getSingleton().spaceID();
	pEventData->isPlayer = isPlayer();
	KBENGINE_EVENT_FIRE(KBEventTypes::onLeaveSpace, pEventData);
}

void Entity::onLeaveSpace()
{

}

void Entity::onPositionChanged(const FVector& oldValue)
{
	//DEBUG_MSG("%s::onPositionChanged: (%f, %f, %f) => (%f, %f, %f)", *className, 
	//	oldValue.X, oldValue.Y, oldValue.Z, position.X, position.Y, position.Z); 
	
	if(isPlayer())
		KBEngineApp::getSingleton().entityServerPos(position);
	
	if(inWorld())
	{
		UKBEventData_set_position* pEventData = NewObject<UKBEventData_set_position>();
		KBPos2UE4Pos(pEventData->position, position);
		pEventData->entityID = id();
		pEventData->moveSpeed = velocity_;
		pEventData->isOnGround = isOnGround();
		KBENGINE_EVENT_FIRE(KBEventTypes::set_position, pEventData);
	}
}

void Entity::onDirectionChanged(const FVector& oldValue)
{
	//DEBUG_MSG("%s::onDirectionChanged: (%f, %f, %f) => (%f, %f, %f)", *className, 
	//	oldValue.X, oldValue.Y, oldValue.Z, direction.X, direction.Y, direction.Z); 

	if (inWorld())
	{
		UKBEventData_set_direction* pEventData = NewObject<UKBEventData_set_direction>();
		KBDir2UE4Dir(pEventData->direction, direction);
		pEventData->entityID = id();
		KBENGINE_EVENT_FIRE(KBEventTypes::set_direction, pEventData);
	}
}

}