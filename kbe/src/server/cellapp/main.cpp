/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
#include "server/kbemain.hpp"
#include "server/idallocate.hpp"
#include "entity.hpp"
#include "entities.hpp"

using namespace KBEngine;

class App: public ServerApp
{
protected:
	IDClient<ENTITY_ID>*		m_idClient_;
	Entities*					m_entities_;									// 存储所有的entity的容器
public:
	App(Mercury::EventDispatcher& dispatcher, COMPONENT_TYPE componentType):ServerApp(dispatcher, componentType)
	{
		
	}

	~App()
	{
	}

	bool installPyModules()
	{
		Entities::installScript(NULL);
		Entity::installScript(getScript().getModule());

		registerScript(Entity::getScriptType());
		
		 m_entities_ = new Entities();
		registerPyObjectToScript("entities", m_entities_);
		return true;
	}

	//-------------------------------------------------------------------------------------
	bool uninstallPyModules()
	{
		Entities::uninstallScript();
		Entity::uninstallScript();
		return true;
	}

	bool run()
	{
		m_idClient_->onAddRange(1, 500);
		Entity* e = createEntity("Avatar", NULL);
		registerPyObjectToScript("avatar", e);
		PyRun_SimpleString("print ('888888888888888888888', KBEngine.avatar.id)");
		DEBUG_MSG("kbe:python is init successfully!!! %d\n", 88);
		SmartPointer<PyObject> testsmartpointer(::PyBytes_FromString("test"));
		testsmartpointer.clear();
		return true;
	}
	
	bool initializeBegin()
	{
		 m_idClient_ = new IDClient<ENTITY_ID>;
		return true;
	}

	bool initializeEnd()
	{
		return true;
	}

	Entity* createEntity(const char* entityType, PyObject* params, bool isInitializeScript = true, ENTITY_ID eid = 0);
};

Entity* App::createEntity(const char* entityType, PyObject* params, bool isInitializeScript, ENTITY_ID eid)
{
	// 检查ID是否足够, 不足返回NULL
	if(eid <= 0 && m_idClient_->getSize() == 0)
	{
		PyErr_SetString(PyExc_SystemError, "App::createEntity: is Failed. not enough entityIDs.");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	ScriptModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL || !sm->hasCell())
	{
		PyErr_Format(PyExc_TypeError, "App::createEntity: entityType [%s] not found.\n", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* obj = sm->createObject();

	// 判断是否要分配一个新的id
	ENTITY_ID id = eid;
	if(id <= 0)
		id = m_idClient_->alloc();

	// 执行Entity的构造函数
	Entity* entity = new(obj) Entity(id, sm);

	// 创建名字空间
	entity->createNamespace(params);

	// 将entity加入entities
	m_entities_->add(id, entity); 
	
	// 检查ID的足够性，不足则申请
	//checkEntityIDEnough();

	// 初始化脚本
	if(isInitializeScript)
		entity->initializeScript();
	
	INFO_MSG("App::createEntity: new %s (%ld).\n", entityType, id);
	return entity;
}

template<> App* Singleton<App>::m_singleton_ = 0;

int KBENGINE_MAIN(int argc, char* argv[])
{
	int ret= kbeMainT<App>(argc, argv, CELLAPP_TYPE);
	getchar();
	return ret; 
}
