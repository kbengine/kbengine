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

EntityFactory* EntityFactory::pEntityFactory = NULL;


EntityDefMethodHandles* EntityDefMethodHandles::pEntityDefMethodHandles = NULL;

EntityDefPropertyHandles* EntityDefPropertyHandles::pEntityDefPropertyHandles = NULL;


EntityFactory::EntityFactory()
{
	pEntityFactory = this;
}

EntityFactory::~EntityFactory()
{

}

EntityDefMethodHandles::EntityDefMethodHandles()
{
	pEntityDefMethodHandles = this;
}

EntityDefMethodHandles::~EntityDefMethodHandles()
{

}

EntityDefPropertyHandles::EntityDefPropertyHandles()
{
	pEntityDefPropertyHandles = this;
}

EntityDefPropertyHandles::~EntityDefPropertyHandles()
{

}

EntityCreator::EntityCreator(const FString& scriptName)
{
	EntityFactory::getSingleton().addEntityCreator(scriptName, this);
}

EntityCreator::~EntityCreator()
{
}

EntityDefMethodHandle::EntityDefMethodHandle(const FString& scriptName, const FString& defMethodName)
{
	EntityDefMethodHandles::getSingleton().add(scriptName, defMethodName, this);
}

EntityDefMethodHandle::~EntityDefMethodHandle()
{

}

EntityDefPropertyHandle::EntityDefPropertyHandle(const FString& scriptName, const FString& defPropertyName)
{
	EntityDefPropertyHandles::getSingleton().add(scriptName, defPropertyName, this);
}

EntityDefPropertyHandle::~EntityDefPropertyHandle()
{

}

EntityCreator* EntityFactory::addEntityCreator(const FString& scriptName, EntityCreator* pEntityCreator)
{
	creators.Add(scriptName, pEntityCreator);
	DEBUG_MSG("EntityFactory::addEntityCreator(): %s", *scriptName);
	return pEntityCreator;
}

EntityCreator* EntityFactory::findCreator(const FString& scriptName)
{
	EntityCreator** pCreator = EntityFactory::getSingleton().creators.Find(scriptName);
	if (pCreator == NULL)
	{
		return NULL;
	}

	return (*pCreator);
}

void EntityFactory::initialize()
{
	static bool inited = false;

	if (inited)
		return;

	inited = true;

	// 填充所有父类的def信息到子类

	/* 由于所有的信息已经直接注册给了顶级Entity， 所以父类没有数据
	for (auto& item : EntityFactory::getSingleton().creators)
	{
		FString scriptName = item.Key;
		EntityCreator* pEntityCreator = item.Value;
		FString parentClasses = pEntityCreator->parentClasses();
		if (parentClasses.Len() == 0)
			continue;

		TArray<FString> parentClassesArray;
		parentClasses.ParseIntoArray(parentClassesArray, TEXT(","), true);

		for (auto& m : parentClassesArray)
		{
			FString moduleName = m.Trim();
			EntityCreator* pEntityParentCreator = EntityFactory::getSingleton().creators.FindRef(moduleName);
			if (!pEntityParentCreator || pEntityParentCreator == pEntityCreator)
				continue;

			finishDefs(scriptName, moduleName);
		}
	}
	*/
}

void EntityFactory::finishDefs(const FString& scriptName, const FString& parentScriptName)
{
	EntityCreator* pEntityCreator = EntityFactory::getSingleton().creators.FindRef(scriptName);
	EntityCreator* pEntityScriptParentCreator = EntityFactory::getSingleton().creators.FindRef(parentScriptName);

	const TMap<FString, EntityDefMethodHandle*>& EntityDefMethodHandleArray = EntityDefMethodHandles::getSingleton().defMethodHandles.FindRef(parentScriptName);
	for (auto& m1 : EntityDefMethodHandleArray)
	{
		EntityDefMethodHandles::getSingleton().add(scriptName, m1.Key, m1.Value);
	}

	const TMap<FString, EntityDefPropertyHandle*>& EntityDefPropertyHandleArray = EntityDefPropertyHandles::getSingleton().defPropertyHandles.FindRef(parentScriptName);
	for (auto& m2 : EntityDefPropertyHandleArray)
	{
		EntityDefPropertyHandles::getSingleton().add(scriptName, m2.Key, m2.Value);
	}

	FString parentClasses = pEntityScriptParentCreator->parentClasses();
	if (parentClasses.Len() == 0)
		return;

	TArray<FString> parentClassesArray;
	parentClasses.ParseIntoArray(parentClassesArray, TEXT(","), true);

	for (auto& m : parentClassesArray)
	{
		EntityCreator* pEntityParentCreator = EntityFactory::getSingleton().creators.FindRef(m);
		if (!pEntityParentCreator || pEntityParentCreator == pEntityCreator)
			continue;

		finishDefs(scriptName, m);
	}
}

Entity* EntityFactory::create(const FString& scriptName)
{
	EntityCreator** pCreator = EntityFactory::getSingleton().creators.Find(scriptName);
	if (pCreator == NULL)
	{
		return NULL;
	}

	Entity* pEntity = (*pCreator)->create();
	return pEntity;
}

EntityDefMethodHandle* EntityDefMethodHandles::add(const FString& scriptName, const FString& defMethodName, EntityDefMethodHandle* pEntityDefMethodHandle)
{
	if (!defMethodHandles.Contains(scriptName))
		defMethodHandles.Add(scriptName, TMap<FString, EntityDefMethodHandle*>());

	TMap<FString, EntityDefMethodHandle*>* m = defMethodHandles.Find(scriptName);

	if (m->Contains(defMethodName))
	{
		SCREEN_ERROR_MSG("EntityDefMethodHandles::add(): %s::%s exist!", *scriptName, *defMethodName);
		return NULL;
	}

	DEBUG_MSG("EntityDefMethodHandles::add(): %s::%s", *scriptName, *defMethodName);
	m->Add(defMethodName, pEntityDefMethodHandle);
	return pEntityDefMethodHandle;
}

EntityDefMethodHandle* EntityDefMethodHandles::find(const FString& scriptName, const FString& defMethodName)
{
	TMap<FString, EntityDefMethodHandle*>* m = EntityDefMethodHandles::getSingleton().defMethodHandles.Find(scriptName);
	if (!m)
		return NULL;

	EntityDefMethodHandle** pEntityDefMethodHandle = m->Find(defMethodName);
	if (!pEntityDefMethodHandle)
		return NULL;

	return *pEntityDefMethodHandle;
}

EntityDefPropertyHandle* EntityDefPropertyHandles::add(const FString& scriptName, const FString& defPropertyName, EntityDefPropertyHandle* pEntityDefPropertyHandle)
{
	if (!defPropertyHandles.Contains(scriptName))
		defPropertyHandles.Add(scriptName, TMap<FString, EntityDefPropertyHandle*>());

	TMap<FString, EntityDefPropertyHandle*>* m = defPropertyHandles.Find(scriptName);

	if (m->Contains(defPropertyName))
	{
		SCREEN_ERROR_MSG("EntityDefPropertyHandles::add(): %s::%s exist!", *scriptName, *defPropertyName);
		return NULL;
	}

	DEBUG_MSG("EntityDefPropertyHandles::add(): %s::%s", *scriptName, *defPropertyName);
	m->Add(defPropertyName, pEntityDefPropertyHandle);
	return pEntityDefPropertyHandle;
}

EntityDefPropertyHandle* EntityDefPropertyHandles::find(const FString& scriptName, const FString& defPropertyName)
{
	TMap<FString, EntityDefPropertyHandle*>* m = EntityDefPropertyHandles::getSingleton().defPropertyHandles.Find(scriptName);
	if (!m)
		return NULL;

	EntityDefPropertyHandle** pEntityDefPropertyHandle = m->Find(defPropertyName);
	if (!pEntityDefPropertyHandle)
		return NULL;

	return *pEntityDefPropertyHandle;
}


Entity::Entity():
	id_(0),
	className_(TEXT("")),
	isOnGround_(false),
	base_(NULL),
	cell_(NULL),
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

void Entity::base(EntityCall* v)
{
	if (base_)
		delete base_;

	base_ = v;
}

void Entity::cell(EntityCall* v)
{
	if (cell_)
		delete cell_;

	cell_ = v;
}

void Entity::callPropertysSetMethods()
{
	ScriptModule** pModuleFind = EntityDef::moduledefs.Find(className());
	if (!pModuleFind)
	{
		SCREEN_ERROR_MSG("Entity::callPropertysSetMethods(): not found ScriptModule(%s)!", *className());
		return;
	}

	ScriptModule* pModule = *pModuleFind;

	for (auto& item : pModule->propertys)
	{
		Property* propertydata = item.Value;
		EntityDefPropertyHandle* pEntityDefPropertyHandle = EntityDefPropertyHandles::find(className(), propertydata->name);
		if (!pEntityDefPropertyHandle)
		{
			SCREEN_ERROR_MSG("Entity::callPropertysSetMethods(): %s(%d) not found property(%s), update error! Please register with ENTITYDEF_PROPERTY_REGISTER(XXX, %s) in (%s, %s).cpp",
				*className(), id(), *propertydata->name,
				*propertydata->name, *className(), *pModule->pEntityCreator->parentClasses());

			continue;
		}

		EntityDefMethodHandle* pSetMethod = propertydata->pSetMethod;

		KBVar* oldval = pEntityDefPropertyHandle->getPropertyValue(this);

		if (pSetMethod)
		{
			if (propertydata->isBase())
			{
				if (inited())
				{
					//DEBUG_MSG("Entity::callPropertysSetMethods(): %s", *propertydata->name);
					pSetMethod->callMethod(this, *oldval);
				}
			}
			else
			{
				if (inWorld())
					pSetMethod->callMethod(this, *oldval);
			}
		}
		else
		{
			//DEBUG_MSG("Entity::callPropertysSetMethods(): %s not found set_*", *propertydata->name);
		}

		delete oldval;
	}
}

void Entity::enterWorld()
{
	inWorld_ = true;

	onEnterWorld();

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
	KBENGINE_EVENT_FIRE("onEnterWorld", pEventData);
}

void Entity::onEnterWorld()
{

}

void Entity::leaveWorld()
{
	inWorld_ = false;

	onLeaveWorld();

	UKBEventData_onLeaveWorld* pEventData = NewObject<UKBEventData_onLeaveWorld>();
	pEventData->entityID = id();
	pEventData->spaceID = KBEngineApp::getSingleton().spaceID();
	pEventData->isPlayer = isPlayer();
	KBENGINE_EVENT_FIRE("onLeaveWorld", pEventData);
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
	KBENGINE_EVENT_FIRE("onEnterSpace", pEventData);
	
	// 要立即刷新表现层对象的位置
	set_position(position);
	set_direction(direction);
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
	KBENGINE_EVENT_FIRE("onLeaveSpace", pEventData);
}

void Entity::onLeaveSpace()
{

}

void Entity::set_position(const FVector& old)
{
	if (isPlayer())
		KBEngineApp::getSingleton().entityServerPos(position);

	if (inWorld_)
	{
		UKBEventData_set_position* pEventData = NewObject<UKBEventData_set_position>();
		KBPos2UE4Pos(pEventData->position, position);
		pEventData->entityID = id();
		pEventData->moveSpeed = velocity_;
		pEventData->isOnGround = isOnGround();
		KBENGINE_EVENT_FIRE("set_position", pEventData);
	}
}

void Entity::set_direction(const FVector& old)
{
	if (inWorld_)
	{
		UKBEventData_set_direction* pEventData = NewObject<UKBEventData_set_direction>();
		pEventData->direction = direction;
		pEventData->entityID = id();
		KBENGINE_EVENT_FIRE("set_direction", pEventData);
	}
}

void Entity::baseCall(FString methodName, KBVar arg1)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	arguments.Add(&arg16);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	arguments.Add(&arg16);
	arguments.Add(&arg17);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	arguments.Add(&arg16);
	arguments.Add(&arg17);
	arguments.Add(&arg18);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18, KBVar arg19)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	arguments.Add(&arg16);
	arguments.Add(&arg17);
	arguments.Add(&arg18);
	arguments.Add(&arg19);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18, KBVar arg19, KBVar arg20)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	arguments.Add(&arg16);
	arguments.Add(&arg17);
	arguments.Add(&arg18);
	arguments.Add(&arg19);
	arguments.Add(&arg20);
	baseCall(methodName, arguments);
}

void Entity::baseCall(FString methodName, const TArray<KBVar*>& arguments)
{
	if (KBEngineApp::getSingleton().currserver() == TEXT("loginapp"))
	{
		ERROR_MSG("Entity::baseCall(): className=%s, methodName=%s, currserver=(%s != baseapp)!", 
			*className_, *methodName, *KBEngineApp::getSingleton().currserver());

		return;
	}

	ScriptModule** pScriptModuleFind = EntityDef::moduledefs.Find(className_);
	if (!pScriptModuleFind)
	{
		SCREEN_ERROR_MSG("Entity::baseCall(): not found ScriptModule(%s)!",
			*className_);

		return;
	}

	Method** pMethodFind = (*pScriptModuleFind)->base_methods.Find(methodName);
	if (!pMethodFind)
	{
		SCREEN_ERROR_MSG("Entity::baseCall(): className=%s, not found methodName(%s)!",
			*className_, *methodName);

		return;
	}

	uint16 methodID = (*pMethodFind)->methodUtype;

	TArray<KBEDATATYPE_BASE*>& args = (*pMethodFind)->args;
	if (arguments.Num() != args.Num())
	{
		ERROR_MSG("Entity::baseCall(): className=%s, methodName=%s, args(%d != %d)!",
			*className_, *methodName, arguments.Num(), (*pMethodFind)->args.Num());

		return;
	}

	if (!base_)
	{
		ERROR_MSG("Entity::baseCall(): %s no base! methodName=%s", *className_, *methodName);
		return;
	}

	base_->newCall();
	(*base_->pBundle) << methodID;

	for (int32 i = 0; i<args.Num(); ++i)
	{
		if (args[i]->isSameType(*arguments[i]))
		{
			args[i]->addToStream(*base_->pBundle, *arguments[i]);
		}
		else
		{
			SCREEN_ERROR_MSG("Entity::baseCall(): className=%s, methodName=%s, args%d error, not is %s! curr=%s",
				*className_, *methodName, i, *args[i]->c_str(), *arguments[i]->c_str());

			Bundle::reclaimObject(base_->pBundle);
			base_->pBundle = NULL;
			return;
		}
	}

	base_->sendCall(NULL);
}

void Entity::cellCall(FString methodName, KBVar arg1)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	arguments.Add(&arg16);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	arguments.Add(&arg16);
	arguments.Add(&arg17);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	arguments.Add(&arg16);
	arguments.Add(&arg17);
	arguments.Add(&arg18);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18, KBVar arg19)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	arguments.Add(&arg16);
	arguments.Add(&arg17);
	arguments.Add(&arg18);
	arguments.Add(&arg19);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, KBVar arg1, KBVar arg2, KBVar arg3, KBVar arg4, KBVar arg5, KBVar arg6, KBVar arg7, KBVar arg8, KBVar arg9, KBVar arg10,
	KBVar arg11, KBVar arg12, KBVar arg13, KBVar arg14, KBVar arg15, KBVar arg16, KBVar arg17, KBVar arg18, KBVar arg19, KBVar arg20)
{
	TArray<KBVar*> arguments;
	arguments.Add(&arg1);
	arguments.Add(&arg2);
	arguments.Add(&arg3);
	arguments.Add(&arg4);
	arguments.Add(&arg5);
	arguments.Add(&arg6);
	arguments.Add(&arg7);
	arguments.Add(&arg8);
	arguments.Add(&arg9);
	arguments.Add(&arg10);
	arguments.Add(&arg11);
	arguments.Add(&arg12);
	arguments.Add(&arg13);
	arguments.Add(&arg14);
	arguments.Add(&arg15);
	arguments.Add(&arg16);
	arguments.Add(&arg17);
	arguments.Add(&arg18);
	arguments.Add(&arg19);
	arguments.Add(&arg20);
	cellCall(methodName, arguments);
}

void Entity::cellCall(FString methodName, const TArray<KBVar*>& arguments)
{
	if (KBEngineApp::getSingleton().currserver() == TEXT("loginapp"))
	{
		ERROR_MSG("Entity::cellCall(): className=%s, methodName=%s, currserver=(%s != baseapp)!",
			*className_, *methodName, *KBEngineApp::getSingleton().currserver());

		return;
	}

	ScriptModule** pScriptModuleFind = EntityDef::moduledefs.Find(className_);
	if (!pScriptModuleFind)
	{
		SCREEN_ERROR_MSG("Entity::cellCall(): not found ScriptModule(%s)!",
			*className_);

		return;
	}

	Method** pMethodFind = (*pScriptModuleFind)->cell_methods.Find(methodName);
	if (!pMethodFind)
	{
		SCREEN_ERROR_MSG("Entity::cellCall(): className=%s, not found methodName(%s)!",
			*className_, *methodName);

		return;
	}

	uint16 methodID = (*pMethodFind)->methodUtype;

	TArray<KBEDATATYPE_BASE*>& args = (*pMethodFind)->args;
	if (arguments.Num() != args.Num())
	{
		ERROR_MSG("Entity::cellCall(): className=%s, methodName=%s, args(%d != %d)!",
			*className_, *methodName, arguments.Num(), (*pMethodFind)->args.Num());

		return;
	}

	if (!cell_)
	{
		ERROR_MSG("Entity::cellCall(): %s no cell! methodName=%s", *className_, *methodName);
		return;
	}

	cell_->newCall();
	(*cell_->pBundle) << methodID;

	for (int32 i = 0; i<args.Num(); ++i)
	{
		if (args[i]->isSameType(*arguments[i]))
		{
			args[i]->addToStream(*cell_->pBundle, *arguments[i]);
		}
		else
		{
			SCREEN_ERROR_MSG("Entity::cellCall(): className=%s, methodName=%s, args%d error, not is %s! curr=%s",
				*className_, *methodName, i, *args[i]->c_str(), *arguments[i]->c_str());

			Bundle::reclaimObject(base_->pBundle);
			base_->pBundle = NULL;
			return;
		}
	}

	cell_->sendCall(NULL);
}

