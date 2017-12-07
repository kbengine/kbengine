
#include "ScriptModule.h"
#include "Method.h"
#include "Property.h"
#include "Entity.h"
#include "KBDebug.h"

ScriptModule::ScriptModule(const FString& moduleName):
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
	pEntityCreator(NULL)
{
	pEntityCreator = EntityFactory::findCreator(moduleName);
	if (!pEntityCreator)
		SCREEN_ERROR_MSG("ScriptModule::ScriptModule(): can't load scriptSodule(KBEngine.%s)!", *moduleName);
}

ScriptModule::~ScriptModule()
{
}
