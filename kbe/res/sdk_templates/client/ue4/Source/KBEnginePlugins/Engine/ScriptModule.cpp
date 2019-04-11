
#include "ScriptModule.h"
#include "Method.h"
#include "Property.h"
#include "Entity.h"
#include "EntityDef.h"
#include "KBDebug.h"

namespace KBEngine
{

ScriptModule::ScriptModule(const FString& moduleName, int type):
	name(moduleName),
	usePropertyDescrAlias(false),
	useMethodDescrAlias(false),
	propertys(),
	idpropertys(),
	methods(),
	base_methods(),
	cell_methods(),
	idmethods(),
	idbase_methods(),
	idcell_methods(),
	utype(type)
{
}

ScriptModule::~ScriptModule()
{
}

Entity* ScriptModule::createEntity()
{
	return EntityDef::createEntity(utype);
}

}